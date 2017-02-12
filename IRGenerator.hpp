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

namespace llvm {
class Value;
}

class IRGenerator {
  public:
    friend class IRExprVis;
    friend class IRStatementVis;
    IRGenerator(llvm::StringRef name)
            : builder{context}, module{std::move(name), context} {}

    void print() const { module.dump(); };

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
    VISIT(CallExpr);

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
