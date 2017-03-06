#ifndef HXWK_IRGENERATOR_H
#define HXWK_IRGENERATOR_H

#include "AST.hpp"
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

class IdScoper {
  public:
    using value_t = llvm::Value *;
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
            : builder{context}, module{std::move(name), context} {}

    void print() const { module.dump(); };

  private:
    template <typename SetupT>
    llvm::Value *gen_scope(const ScopeExpr &scope, SetupT setup);

    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Module module;
    IdScoper named_values;
};

class IRExprVis : public ExprVis {
  public:
    IRExprVis(IRGenerator &gen) : gen{gen} {};

    VISIT(LiteralExpr<double>);
    VISIT(IdExpr);
    VISIT(BinaryExpr);
    VISIT(CallExpr);
    VISIT(ScopeExpr);
    VISIT(IfExpr);

    llvm::Value *get_val() const { return val; };

  private:
    IRGenerator &gen;
    llvm::Value *val;
};

class IRStatementVis : public StatementVis {
  public:
    IRStatementVis(IRGenerator &gen) : gen{gen} {};

    VISIT(Expr);
    VISIT(VarDecl);
    VISIT(FnDecl);
    VISIT(FnDef);

    llvm::Value *get_val() const { return val; };

  private:
    IRGenerator &gen;
    llvm::Value *val;
};

#endif
