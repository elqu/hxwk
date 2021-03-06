#ifndef HXWK_IRGENERATOR_H
#define HXWK_IRGENERATOR_H

#include "AST.hpp"
#include "Type.hpp"
#include "VisitorPattern.hpp"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <map>
#include <string>
#include <utility>

namespace llvm {
class Value;
}

struct IRHandle {
    llvm::Value *val;
    std::shared_ptr<Type> type;

    void reset() {
        val = nullptr;
        type.reset();
    };
};

class IdScoper {
  public:
    using value_t = IRHandle;
    void enter();
    void exit();
    value_t operator[](const std::string &id);
    value_t &current_scope(const std::string &id);

  private:
    std::vector<std::map<std::string, value_t>> named_values;
};

class IRGenerator {
  public:
    friend class IRExprVis;
    friend class IRStatementVis;
    IRGenerator(llvm::StringRef name)
            : builder{context}, module{std::move(name), context} {
        named_values.enter();
        named_values.current_scope("printf")
                = {llvm::Function::Create(
                           llvm::FunctionType::get(
                                   llvm::Type::getInt32Ty(context),
                                   llvm::Type::getInt8PtrTy(context), true),
                           llvm::Function::ExternalLinkage, "printf", &module),
                   std::make_shared<FunctionType>(
                           std::vector<std::shared_ptr<Type>>{
                                   std::make_shared<StrLitType>()},
                           std::make_shared<Int32Type>())};
    };

    void print() const { module.dump(); };
    void write_assembly(std::ostream &stream) const;
    void write_bitcode(std::ostream &stream) const;

  private:
    template <typename SetupT>
    IRHandle gen_scope(const ScopeExpr &scope, SetupT setup);
    llvm::Type *get_llvm_type(const Type &type);
    llvm::Value *arit_cast(llvm::Value *val, const Type &from, const Type &to);

    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Module module;
    IdScoper named_values;
};

class IRExprVis : public ExprVis {
  public:
    IRExprVis(IRGenerator &gen) : gen{gen} {};

    VISIT(LiteralExpr<int32_t>);
    VISIT(LiteralExpr<double>);
    VISIT(LiteralExpr<std::string>);
    VISIT(IdExpr);
    VISIT(BinaryExpr);
    VISIT(CallExpr);
    VISIT(ScopeExpr);
    VISIT(IfExpr);

    const IRHandle &get_handle() const { return handle; };
    llvm::Value *get_val() const { return handle.val; };
    const std::shared_ptr<Type> &get_type() const { return handle.type; };

  private:
    IRGenerator &gen;
    IRHandle handle;
};

class IRStatementVis : public StatementVis {
  public:
    IRStatementVis(IRGenerator &gen) : gen{gen} {};

    VISIT(Expr);
    VISIT(VarDecl);
    VISIT(FnDecl);
    VISIT(FnDef);

    const IRHandle &get_handle() const { return handle; };
    llvm::Value *get_val() const { return handle.val; };
    const std::shared_ptr<Type> &get_type() const { return handle.type; };

  private:
    IRGenerator &gen;
    IRHandle handle;
};

#endif
