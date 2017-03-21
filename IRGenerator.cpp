#include "IRGenerator.hpp"
#include "C++11Compat.hpp"
#include "Lexer.hpp"
#include "Log.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_os_ostream.h"
#include <iostream>
#include <utility>
#include <vector>

namespace llvm {
class Type;
}

void IdScoper::enter() {
    named_values.emplace_back();
}

void IdScoper::exit() {
    named_values.pop_back();
}

IdScoper::value_t IdScoper::operator[](const std::string &id) {
    for (auto i = named_values.rbegin(); i != named_values.rend(); ++i) {
        value_t &val = i->operator[](id);
        if (val.val != nullptr)
            return val;
    }
    return value_t{};
}

IdScoper::value_t &IdScoper::current_scope(const std::string &id) {
    return named_values.back()[id];
}

void IRGenerator::write_assembly(std::ostream &stream) const {
    llvm::raw_os_ostream llvm_stream{stream};
    module.print(llvm_stream, nullptr);
}

void IRGenerator::write_bitcode(std::ostream &stream) const {
    llvm::raw_os_ostream llvm_stream{stream};
    llvm::WriteBitcodeToFile(&module, llvm_stream);
}

template <typename SetupT>
IRHandle IRGenerator::gen_scope(const ScopeExpr &scope, SetupT setup) {
    named_values.enter();

    setup();

    IRStatementVis body_vis{*this};
    for (const auto &statement : scope.get_body()) {
        statement->accept(body_vis);
        if (!body_vis.get_val())
            break;
    }

    named_values.exit();

    if (scope.get_body().empty())
        return {llvm::ConstantPointerNull::get(
                        llvm::Type::getInt8PtrTy(context)),  // Stub value
                std::make_shared<VoidType>()};

    return body_vis.get_handle();
}

llvm::Type *IRGenerator::get_llvm_type(const Type &type) {
    if (llvm::isa<BoolType>(type)) {
        return llvm::Type::getInt1Ty(context);
    } else if (llvm::isa<Int32Type>(type)) {
        return llvm::Type::getInt32Ty(context);
    } else if (llvm::isa<DoubleType>(type)) {
        return llvm::Type::getDoubleTy(context);
    } else if (llvm::isa<VoidType>(type)) {
        return llvm::Type::getVoidTy(context);
    } else {
        return nullptr;
    }
}

llvm::Value *IRGenerator::arit_cast(llvm::Value *val, const Type &from,
                                    const Type &to) {
    using TypeKind = Type::TypeKind;

    if (from.getKind() == to.getKind())
        return val;

    // This really is a mess. There should be a better way of doing things
    switch (from.getKind()) {
        case TypeKind::Bool:
            switch (to.getKind()) {
                case TypeKind::Int32:
                    return builder.CreateIntCast(val, get_llvm_type(to),
                                                 false);
                case TypeKind::Double:
                    return builder.CreateUIToFP(val, get_llvm_type(to));
                default:
                    break;
            }
            break;
        case TypeKind::Int32:
            switch (to.getKind()) {
                case TypeKind::Bool:
                    return builder.CreateIntCast(val, get_llvm_type(to), true);
                case TypeKind::Double:
                    std::printf("immernoch nice nice nice\n");
                    return builder.CreateSIToFP(val, get_llvm_type(to));
                default:
                    break;
            }
            break;
        case TypeKind::Double:
            switch (to.getKind()) {
                case TypeKind::Bool:
                    return builder.CreateFPToUI(val, get_llvm_type(to));
                case TypeKind::Int32:
                    return builder.CreateFPToSI(val, get_llvm_type(to));
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return nullptr;
}

void IRExprVis::visit(const LiteralExpr<int32_t> &expr) {
    handle.reset();
    handle = {llvm::ConstantInt::get(
                      gen.context,
                      llvm::APInt{32, static_cast<uint64_t>(expr.get_val()),
                                  true}),
              std::make_shared<Int32Type>()};
}

void IRExprVis::visit(const LiteralExpr<double> &expr) {
    handle.reset();
    handle = {
            llvm::ConstantFP::get(gen.context, llvm::APFloat{expr.get_val()}),
            std::make_shared<DoubleType>()};
}

void IRExprVis::visit(const LiteralExpr<std::string> &expr) {
    handle.reset();
    handle = {gen.builder.CreateGlobalStringPtr(expr.get_val()),
              std::make_shared<StrLitType>()};
}

void IRExprVis::visit(const IdExpr &expr) {
    handle.reset();
    handle = gen.named_values[expr.get_id()];
}

bool is_arit(const Type &type) {
    return llvm::isa<BoolType>(type) || llvm::isa<Int32Type>(type)
           || llvm::isa<DoubleType>(type);
}

void IRExprVis::visit(const BinaryExpr &expr) {
    handle.reset();

    IRExprVis lhs{gen}, rhs{gen};
    expr.get_lhs().accept(lhs);
    expr.get_rhs().accept(rhs);
    if (!lhs.get_val() || !rhs.get_val())
        return;

    if (!(is_arit(*lhs.get_type()) && is_arit(*rhs.get_type())))
        return Log::error(
                "Both parameters of a binary expression must be of "
                "type `double`");

    handle.type = lhs.get_type()->getKind() > rhs.get_type()->getKind()
                          ? lhs.get_type()
                          : rhs.get_type();

    auto *lhs_val
            = gen.arit_cast(lhs.get_val(), *lhs.get_type(), *handle.type);
    auto *rhs_val
            = gen.arit_cast(rhs.get_val(), *rhs.get_type(), *handle.type);

    using BinaryOps = llvm::Instruction::BinaryOps;
    bool is_fp = llvm::isa<DoubleType>(*handle.type);
    bool is_signed = llvm::isa<Int32Type>(*handle.type);
    bool is_cmp;

    llvm::Instruction::BinaryOps op;
    llvm::CmpInst::Predicate op_cmp;

    switch (expr.get_op()) {
        case Tok::PLUS:
            op = is_fp ? BinaryOps::FAdd : BinaryOps::Add;
            is_cmp = false;
            break;
        case Tok::MINUS:
            op = is_fp ? BinaryOps::FSub : BinaryOps::Sub;
            is_cmp = false;
            break;
        case Tok::MULT:
            op = is_fp ? BinaryOps::FMul : BinaryOps::Mul;
            is_cmp = false;
            break;
        case Tok::SLASH:
            op = is_fp ? BinaryOps::FDiv
                       : (is_signed ? BinaryOps::SDiv : BinaryOps::UDiv);
            is_cmp = false;
            break;
        case Tok::CMP_LT:
            op_cmp = is_fp ? llvm::CmpInst::Predicate::FCMP_ULT
                           : (is_signed ? llvm::CmpInst::Predicate::ICMP_SLT
                                        : llvm::CmpInst::Predicate::ICMP_ULT);
            is_cmp = true;
            break;
        default:
            Log::error("Unknown binary operator");
            return;
    }

    if (is_cmp) {
        handle.val = is_fp ? gen.builder.CreateFCmp(op_cmp, lhs_val, rhs_val)
                           : gen.builder.CreateICmp(op_cmp, lhs_val, rhs_val);
        handle.type = std::make_shared<BoolType>();
    } else {
        handle.val = gen.builder.CreateBinOp(op, lhs_val, rhs_val);
    }
}

void IRExprVis::visit(const CallExpr &expr) {
    handle.reset();

    auto callee_handle = gen.named_values[expr.get_id()];
    if (!callee_handle.val) {
        return Log::error("Undeclared function ", expr.get_id());
    }
    auto *callee = static_cast<llvm::Function *>(callee_handle.val);

    const auto *type = llvm::dyn_cast<FunctionType>(callee_handle.type.get());
    if (!type)
        return Log::error("`", expr.get_id(), "` is not a function");

    auto expected_args = type->get_n_args();
    auto given_args = expr.get_args().size();
    if (expected_args != given_args
        && !(callee->isVarArg() && expected_args <= given_args))
        return Log::error("Wrong number of arguments (expected ",
                          expected_args, " but got ", given_args, ")");

    std::vector<llvm::Value *> args;
    for (const auto &arg_node : expr.get_args()) {
        IRExprVis arg_vis{gen};
        arg_node->accept(arg_vis);
        if (!arg_vis.get_val())
            return;
        args.push_back(arg_vis.get_val());
    }

    handle = {gen.builder.CreateCall(callee, std::move(args)),
              type->get_ret_type()};
}

void IRExprVis::visit(const ScopeExpr &expr) {
    handle.reset();
    handle = gen.gen_scope(expr, [] {});
}

void IRExprVis::visit(const IfExpr &expr) {
    handle.reset();

    IRExprVis cond_vis{gen};
    expr.get_cond().accept(cond_vis);
    if (!cond_vis.get_val())
        return;

    if (!llvm::isa<BoolType>(*cond_vis.get_type()))
        return Log::error("Condition must be of type `bool`");

    auto *fn = gen.builder.GetInsertBlock()->getParent();
    auto *then = llvm::BasicBlock::Create(gen.context, "", fn);
    auto *or_else = llvm::BasicBlock::Create(gen.context, "");
    auto *merge = llvm::BasicBlock::Create(gen.context, "");

    gen.builder.CreateCondBr(cond_vis.get_val(), then, or_else);

    gen.builder.SetInsertPoint(then);
    auto then_val = gen.gen_scope(expr.get_then(), [] {});
    if (!then_val.val)
        return;
    gen.builder.CreateBr(merge);
    then = gen.builder.GetInsertBlock();

    fn->getBasicBlockList().push_back(or_else);
    gen.builder.SetInsertPoint(or_else);
    auto else_val = gen.gen_scope(expr.get_else(), [] {});
    if (!else_val.val)
        return;
    gen.builder.CreateBr(merge);
    or_else = gen.builder.GetInsertBlock();

    if (*then_val.type != *else_val.type)
        return Log::error("Types of then and else scope do not match");

    fn->getBasicBlockList().push_back(merge);
    gen.builder.SetInsertPoint(merge);

    auto *type = gen.get_llvm_type(*then_val.type);
    if (!type)
        return Log::error("Invalid type");

    auto *phi = gen.builder.CreatePHI(type, 2);
    phi->addIncoming(then_val.val, then);
    phi->addIncoming(else_val.val, or_else);

    handle = {phi, then_val.type};
}

void IRStatementVis::visit(const Expr &expr) {
    handle.reset();

    IRExprVis expr_vis{gen};
    expr.accept(expr_vis);
    handle = expr_vis.get_handle();
}

void IRStatementVis::visit(const VarDecl &decl) {
    handle.reset();

    const auto &id = decl.get_id();

    IRExprVis expr_vis{gen};
    decl.get_rhs().accept(expr_vis);
    handle = expr_vis.get_handle();
    if (!handle.val)
        return;

    handle.val->setName(id);

    gen.named_values.current_scope(id) = handle;
}

void IRStatementVis::visit(const FnDecl &decl) {
    handle.reset();

    auto *ret_type = gen.get_llvm_type(*decl.get_ret_type());
    if (!ret_type)
        return Log::error("Invalid type");

    auto *double_type = llvm::FunctionType::getDoubleTy(gen.context);
    std::vector<llvm::Type *> types{decl.get_params().size(), double_type};
    auto *fn_type = llvm::FunctionType::get(ret_type, types, false);

    auto *fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage,
                                      decl.get_id(), &gen.module);

    std::size_t i = 0;
    for (auto &arg : fn->args()) {
        arg.setName(decl.get_params()[i++]);
    }

    handle = {fn, std::make_shared<FunctionType>(
                          fn->arg_size(), std::move(decl.get_ret_type()))};
    gen.named_values.current_scope(decl.get_id()) = handle;
}

void IRStatementVis::visit(const FnDef &def) {
    handle.reset();

    const auto &id = def.get_decl().get_id();
    auto fn_handle = gen.named_values[id];

    if (fn_handle.val && llvm::isa<FunctionType>(*fn_handle.type))
        return Log::error("Cannot redefine function (`", id, "`)");

    if (!fn_handle.val) {
        IRStatementVis decl_vis{gen};
        def.get_decl().accept(decl_vis);
        fn_handle = decl_vis.get_handle();
        if (!fn_handle.val)
            return;
    }

    auto *fn = static_cast<llvm::Function *>(fn_handle.val);

    auto *bb = llvm::BasicBlock::Create(gen.context, "entry", fn);
    gen.builder.SetInsertPoint(bb);

    auto body_val = gen.gen_scope(def.get_body_scope(), [fn, this] {
        auto type = std::make_shared<DoubleType>();
        for (auto &arg : fn->args())
            gen.named_values.current_scope(arg.getName()) = {&arg, type};
    });

    if (!body_val.val) {
        fn->eraseFromParent();
        return;
    }

    const auto &ret_type
            = *(llvm::cast<FunctionType>(*fn_handle.type).get_ret_type());
    const bool ret_void = llvm::isa<VoidType>(ret_type);
    if (*body_val.type != ret_type && !ret_void) {
        fn->eraseFromParent();
        return Log::error("Returned value does not match function type");
    }

    if (ret_void) {
        gen.builder.CreateRetVoid();
    } else {
        gen.builder.CreateRet(body_val.val);
    }

    llvm::raw_os_ostream err{std::cerr};
    if (llvm::verifyFunction(*fn, &err))
        return;

    handle = std::move(fn_handle);
}
