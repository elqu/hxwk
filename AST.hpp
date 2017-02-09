#ifndef HXWK_AST_H
#define HXWK_AST_H

#include "Lexer.hpp"
#include "VisitorPattern.hpp"

#include <memory>

class Statement;
    class Expr;
        template<typename T> class LiteralExpr;
        class IdExpr;
        class BinaryExpr;
    class VarDecl;

class StatementVis {
    public:
        virtual ~StatementVis() = default;
        ABSTR_VISIT(Expr);
        ABSTR_VISIT(VarDecl);
};

class ExprVis {
    public:
        virtual ~ExprVis() = default;
        ABSTR_VISIT(LiteralExpr<double>);
        ABSTR_VISIT(IdExpr);
        ABSTR_VISIT(BinaryExpr);
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

template<typename T>
class LiteralExpr : public Expr {
    public:
        LiteralExpr(T val) : val(val) {};

        T get_val() const {return val;};

        ACCEPT(ExprVis);

    private:
        T val;
};

class IdExpr : public Expr {
    public:
        IdExpr(std::string id) : id(std::move(id)) {};

        std::string get_id() const {return id;};

        ACCEPT(ExprVis);

    private:
        std::string id;
};

class BinaryExpr : public Expr {
    public:
        BinaryExpr(Tok op, std::unique_ptr<Expr> lhs,
                           std::unique_ptr<Expr> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {};

        Tok get_op() const {return op;};
        Expr& get_lhs() const {return *lhs;};
        Expr& get_rhs() const {return *rhs;};

        ACCEPT(ExprVis);

    private:
        Tok op;
        std::unique_ptr<Expr> lhs, rhs;
};

class VarDecl : public Statement {
    public:
        VarDecl(Tok type, std::string id, std::unique_ptr<Expr> rhs)
            : type(type), id(std::move(id)), rhs(std::move(rhs)) {};

        Tok get_type() const {return type;};
        std::string get_id() const {return id;};
        Expr& get_rhs() const {return *rhs;};

        ACCEPT(StatementVis);

    private:
        Tok type; // TODO: implement a "type" type for more than just POD types
        std::string id;
        std::unique_ptr<Expr> rhs;
};

#endif
