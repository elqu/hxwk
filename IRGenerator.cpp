#include "IRGenerator.hpp"
#include "Lexer.hpp"
#include "Log.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Verifier.h"
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
        if (val != value_t{})
            return val;
    }
    return nullptr;
}

IdScoper::value_t &IdScoper::current_scope(const std::string &id) {
    return named_values.back()[id];
}

template <typename SetupT>
llvm::Value *IRGenerator::gen_scope(const ScopeExpr &scope, SetupT setup) {
    named_values.enter();

    setup();

    IRStatementVis body_vis{*this};
    for (const auto &statement : scope.get_body()) {
        statement->accept(body_vis);
        if (!body_vis.get_val())
            break;
    }

    named_values.exit();

    return body_vis.get_val();
}

void IRExprVis::visit(const LiteralExpr<double> &expr) {
    val = nullptr;
    val = llvm::ConstantFP::get(gen.context, llvm::APFloat{expr.get_val()});
}

void IRExprVis::visit(const IdExpr &expr) {
    val = nullptr;
    val = gen.named_values[expr.get_id()];
}

void IRExprVis::visit(const BinaryExpr &expr) {
    val = nullptr;

    IRExprVis lhs{gen}, rhs{gen};
    expr.get_lhs().accept(lhs);
    expr.get_rhs().accept(rhs);
    if (!lhs.val || !rhs.val)
        return;

    // TODO: assignment operator
    switch (expr.get_op()) {
        case Tok::PLUS:
            val = gen.builder.CreateFAdd(lhs.val, rhs.val);
            break;
        case Tok::MULT:
            val = gen.builder.CreateFMul(lhs.val, rhs.val);
            break;
        default:
            Log::error("Unknown binary operator");
            break;
    }
}

void IRExprVis::visit(const CallExpr &expr) {
    val = nullptr;

    auto *callee = gen.module.getFunction(expr.get_id());
    if (!callee)
        return Log::error("Undeclared function ", expr.get_id());

    auto expected_args = callee->arg_size();
    auto given_args = expr.get_args().size();
    if (expected_args != given_args)
        return Log::error("Wrong number of arguments (expected ",
                          expected_args, ", got ", given_args, ")");

    std::vector<llvm::Value *> args;
    for (const auto &arg_node : expr.get_args()) {
        IRExprVis arg_vis{gen};
        arg_node->accept(arg_vis);
        if (!arg_vis.val)
            return;
        args.push_back(arg_vis.val);
    }

    val = gen.builder.CreateCall(callee, std::move(args));
}

void IRExprVis::visit(const ScopeExpr& expr) {
    val = nullptr;
    val = gen.gen_scope(expr, [] {});
}

void IRStatementVis::visit(const Expr &expr) {
    val = nullptr;

    IRExprVis expr_vis{gen};
    expr.accept(expr_vis);
    val = expr_vis.get_val();
}

void IRStatementVis::visit(const VarDecl &decl) {
    val = nullptr;

    const auto &id = decl.get_id();

    IRExprVis expr_vis{gen};
    decl.get_rhs().accept(expr_vis);
    val = expr_vis.get_val();
    if (!val)
        return;

    val->setName(id);

    gen.named_values.current_scope(id) = val;
}

void IRStatementVis::visit(const FnDecl &decl) {
    val = nullptr;

    auto *double_type = llvm::FunctionType::getDoubleTy(gen.context);
    std::vector<llvm::Type *> types{decl.get_params().size(), double_type};
    auto *fn_type = llvm::FunctionType::get(double_type, types, false);

    auto *fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage,
                                      decl.get_id(), &gen.module);

    std::size_t i = 0;
    for (auto &arg : fn->args()) {
        arg.setName(decl.get_params()[i++]);
    }

    val = fn;
}

void IRStatementVis::visit(const FnDef &def) {
    val = nullptr;

    const auto &id = def.get_decl().get_id();
    auto *fn = gen.module.getFunction(id);

    if (fn && !fn->empty())
        return Log::error("Cannot redefine function `", id, "`");

    if (!fn) {
        IRStatementVis decl_vis{gen};
        def.get_decl().accept(decl_vis);
        fn = static_cast<llvm::Function *>(decl_vis.val);
        if (!fn)
            return;
    }

    auto *bb = llvm::BasicBlock::Create(gen.context, "entry", fn);
    gen.builder.SetInsertPoint(bb);

    gen.named_values.enter();
    for (auto &arg : fn->args())
        gen.named_values.current_scope(arg.getName()) = &arg;

    IRStatementVis body_vis{gen};
    const auto &body = def.get_body();
    for (auto i = body.cbegin(); i != body.cend(); ++i) {
        (*i)->accept(body_vis);
        if (!body_vis.val) {
            fn->eraseFromParent();
            return;
        }
    }
    gen.named_values.exit();

    gen.builder.CreateRet(body_vis.val);

    llvm::raw_os_ostream err{std::cerr};
    if (llvm::verifyFunction(*fn, &err))
        return;

    val = fn;
}
