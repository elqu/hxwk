#ifndef HXWK_AST_H
#define HXWK_AST_H

#include "Lexer.hpp"
#include "Type.hpp"
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
class ScopeExpr;
class IfExpr;
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
    ABSTR_VISIT(LiteralExpr<std::string>);
    ABSTR_VISIT(IdExpr);
    ABSTR_VISIT(BinaryExpr);
    ABSTR_VISIT(CallExpr);
    ABSTR_VISIT(ScopeExpr);
    ABSTR_VISIT(IfExpr);
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

class ScopeExpr : public Expr {
  public:
    using Body_t = std::vector<std::unique_ptr<Statement>>;

    ScopeExpr(Body_t body) : body(std::move(body)){};

    const Body_t &get_body() const { return body; };

    ACCEPT(ExprVis);

  private:
    Body_t body;
};

class IfExpr : public Expr {
  public:
    IfExpr(std::unique_ptr<Expr> cond, std::unique_ptr<ScopeExpr> then,
           std::unique_ptr<ScopeExpr> or_else)
            : cond(std::move(cond)),
              then(std::move(then)),
              or_else(std::move(or_else)){};

    const Expr &get_cond() const { return *cond; };
    const ScopeExpr &get_then() const { return *then; };
    const ScopeExpr &get_else() const { return *or_else; };

    ACCEPT(ExprVis);

  private:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<ScopeExpr> then, or_else;
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
    FnDecl(std::string id, std::vector<std::string> params,
           std::shared_ptr<Type> ret_type)
            : id(std::move(id)),
              params(std::move(params)),
              ret_type(std::move(ret_type)){};

    const std::string &get_id() const { return id; };
    const std::vector<std::string> &get_params() const { return params; };
    const std::shared_ptr<Type> &get_ret_type() const { return ret_type; };

    ACCEPT(StatementVis);

  private:
    std::string id;
    std::vector<std::string> params;
    std::shared_ptr<Type> ret_type;
};

class FnDef : public Statement {
  public:
    FnDef(std::unique_ptr<FnDecl> decl, std::unique_ptr<ScopeExpr> body)
            : decl(std::move(decl)), body(std::move(body)){};

    const FnDecl &get_decl() const { return *decl; };
    const ScopeExpr &get_body_scope() const { return *body; };
    const ScopeExpr::Body_t &get_body() const { return body->get_body(); };

    ACCEPT(StatementVis);

  private:
    std::unique_ptr<FnDecl> decl;
    std::unique_ptr<ScopeExpr> body;
};

#endif
