#ifndef HXWK_AST_H
#define HXWK_AST_H

#include "Lexer.hpp"
#include "VisitorPattern.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Expr;
template <typename T>
class LiteralExpr;
class IdExpr;
class BinaryExpr;
class CallExpr;
class VarDecl;
class FnDecl;
class FnDef;

class StatementVis {
  public:
    virtual ~StatementVis() = default;
    ABSTR_VISIT(Expr);
    ABSTR_VISIT(VarDecl);
    ABSTR_VISIT(FnDecl);
    ABSTR_VISIT(FnDef);
};

class ExprVis {
  public:
    virtual ~ExprVis() = default;
    ABSTR_VISIT(LiteralExpr<double>);
    ABSTR_VISIT(IdExpr);
    ABSTR_VISIT(BinaryExpr);
    ABSTR_VISIT(CallExpr);
};

class Statement {
  public:
    virtual ~Statement() = default;
    ABSTR_ACCEPT(StatementVis);
};

class Expr : public Statement {
  public:
    ABSTR_ACCEPT(ExprVis);
    ACCEPT(StatementVis);
};

template <typename T>
class LiteralExpr : public Expr {
  public:
    LiteralExpr(T val) : val(val){};

    T get_val() const { return val; };

    ACCEPT(ExprVis);

  private:
    T val;
};

class IdExpr : public Expr {
  public:
    IdExpr(std::string id) : id(std::move(id)){};

    const std::string &get_id() const { return id; };

    ACCEPT(ExprVis);

  private:
    std::string id;
};

class BinaryExpr : public Expr {
  public:
    BinaryExpr(Tok op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)){};

    Tok get_op() const { return op; };
    const Expr &get_lhs() const { return *lhs; };
    const Expr &get_rhs() const { return *rhs; };

    ACCEPT(ExprVis);

  private:
    Tok op;
    std::unique_ptr<Expr> lhs, rhs;
};

class CallExpr : public Expr {
  public:
    CallExpr(std::string id, std::vector<std::unique_ptr<Expr>> args)
            : id(std::move(id)), args(std::move(args)){};

    const std::string &get_id() const { return id; };
    const std::vector<std::unique_ptr<Expr>> &get_args() const {
        return args;
    };

    ACCEPT(ExprVis);

  private:
    std::string id;
    std::vector<std::unique_ptr<Expr>> args;
};

class VarDecl : public Statement {
  public:
    VarDecl(std::string id, std::unique_ptr<Expr> rhs)
            : id(std::move(id)), rhs(std::move(rhs)){};

    const std::string &get_id() const { return id; };
    const Expr &get_rhs() const { return *rhs; };

    ACCEPT(StatementVis);

  private:
    std::string id;
    std::unique_ptr<Expr> rhs;
};

class FnDecl : public Statement {
  public:
    FnDecl(std::string id, std::vector<std::string> params)
            : id(std::move(id)), params(std::move(params)){};

    const std::string &get_id() const { return id; };
    const std::vector<std::string> &get_params() const { return params; };

    ACCEPT(StatementVis);

  private:
    std::string id;
    std::vector<std::string> params;
};

class FnDef : public Statement {
  public:
    using Body_t = std::vector<std::unique_ptr<Statement>>;

    FnDef(std::unique_ptr<FnDecl> decl, Body_t body)
            : decl(std::move(decl)), body(std::move(body)){};

    const FnDecl &get_decl() const { return *decl; };
    const Body_t &get_body() const { return body; };

    ACCEPT(StatementVis);

  private:
    std::unique_ptr<FnDecl> decl;
    Body_t body;
};

#endif
