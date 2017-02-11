#include "Parser.hpp"

#include "C++11Compat.hpp"

#include <map>

static std::map<Tok, std::pair<int, Assoc>> binary_precedence{
        {Tok::EQ, {10, Assoc::RIGHT}},
        {Tok::PLUS, {20, Assoc::LEFT}},
        {Tok::MULT, {30, Assoc::LEFT}}};

std::pair<int, Assoc> Parser::get_precedence(Tok tok) {
    auto precedence = binary_precedence[tok];

    if (precedence.first == 0)
        return {-1, Assoc::LEFT};

    return precedence;
}

std::unique_ptr<Statement> Parser::parse() {
    if (lex.get_tok() == Tok::END)
        return nullptr;

    Tok cur_tok = lex.get_next_tok();

    switch (cur_tok) {
        case Tok::INVALID:
        case Tok::END:
        case Tok::PLUS:
        case Tok::MULT:
        case Tok::EQ:
            return nullptr;
        case Tok::SEMICOLON:
            return parse();
        case Tok::T_DOUBLE:
            return parse_var_decl();
        case Tok::ID:
        case Tok::L_DOUBLE:
            return parse_top_expr();
    }
}

std::unique_ptr<VarDecl> Parser::parse_var_decl() {
    Tok cur_tok = lex.get_next_tok();
    if (cur_tok != Tok::ID)
        return nullptr;

    auto id = lex.get_id();

    cur_tok = lex.get_next_tok();
    if (cur_tok != Tok::EQ)
        return nullptr;

    lex.get_next_tok();
    auto expr = parse_expr();
    if (!expr)
        return nullptr;

    return std::make_unique<VarDecl>(id, std::move(expr));
}

std::unique_ptr<Expr> Parser::parse_top_expr() {
    auto expr = parse_expr();

    if (lex.get_tok() != Tok::SEMICOLON)
        return nullptr;

    return expr;
}

std::unique_ptr<Expr> Parser::parse_expr() {
    return parse_expr_rhs(0, parse_primary());
}

std::unique_ptr<Expr> Parser::parse_expr_rhs(int expr_prec,
                                             std::unique_ptr<Expr> lhs) {
    while (true) {
        Tok op = lex.get_tok();
        int op_prec;
        Assoc lr_ass;
        std::tie(op_prec, lr_ass) = get_precedence(op);

        if (op_prec < expr_prec)
            return lhs;

        lex.get_next_tok();

        auto rhs = parse_primary();
        if (!rhs)
            return nullptr;

        Tok new_op = lex.get_tok();
        int new_op_prec = get_precedence(new_op).first;

        if (new_op_prec > op_prec
            || (new_op_prec == op_prec && lr_ass == Assoc::RIGHT)) {
            rhs = parse_expr_rhs(op_prec, std::move(rhs));
        }

        lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<Expr> Parser::parse_primary() {
    std::unique_ptr<Expr> expr;

    Tok cur_tok = lex.get_tok();
    if (cur_tok == Tok::L_DOUBLE) {
        expr = std::make_unique<LiteralExpr<double>>(lex.get_double());
    } else if (cur_tok == Tok::ID) {
        expr = std::make_unique<IdExpr>(lex.get_id());
    } else {
        return nullptr;
    }

    lex.get_next_tok();
    return expr;
}
