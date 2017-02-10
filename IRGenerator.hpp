#ifndef HXWK_IRGENERATOR_H
#define HXWK_IRGENERATOR_H

#include "AST.hpp"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

class IRStatementVis;

class IRGenerator {
  public:
    friend class IRExprVis;
    friend class IRStatementVis;
    IRGenerator(llvm::StringRef name);

    void print() const { module.dump(); };
    void finish(const IRStatementVis &vis);  // TEMPORARY finishing method
  private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Module module;
    std::map<std::string, llvm::Value *> named_values;
};

class IRExprVis : public ExprVis {
  public:
    IRExprVis(IRGenerator &gen) : gen{gen} {};

    VISIT(LiteralExpr<double>);
    VISIT(IdExpr);
    VISIT(BinaryExpr);

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

    llvm::Value *get_val() const { return val; };

  private:
    IRGenerator &gen;
    llvm::Value *val;
};

#endif
