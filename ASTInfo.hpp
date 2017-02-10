#ifndef HXWK_ASTINFO_H
#define HXWK_ASTINFO_H

#include "AST.hpp"

#include <string>

class ExprInfoVis : public ExprVis {
  public:
    VISIT(LiteralExpr<double>);
    VISIT(IdExpr);
    VISIT(BinaryExpr);

    std::string get_str() const { return str; };

  private:
    std::string str;
};

class SynInfoVis : public StatementVis {
  public:
    VISIT(Expr);
    VISIT(VarDecl);

    std::string get_str() const { return str; };

  private:
    std::string str;
};

#endif
