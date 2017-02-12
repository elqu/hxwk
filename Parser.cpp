#include "Parser.hpp"
#include "AST.hpp"
#include "C++11Compat.hpp"
#include <map>
#include <tuple>

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
    switch (lex.get_tok()) {
        case Tok::SEMICOLON:
            lex.get_next_tok();
            return parse();
        case Tok::FN:
            return parse_fn();
        default:
            return nullptr;
    }
}

std::unique_ptr<Statement> Parser::parse_fn() {
    Tok cur_tok = lex.get_next_tok();
    if (cur_tok != Tok::ID)
        return nullptr;
    auto id = lex.get_id();

    if ((cur_tok = lex.get_next_tok()) != Tok::P_OPEN)
        return nullptr;

    std::vector<std::string> params;

    if ((cur_tok = lex.get_next_tok()) == Tok::ID) {
        do {
            params.push_back(lex.get_id());
        } while ((cur_tok = lex.get_next_tok()) == Tok::COMMA
                 && (cur_tok = lex.get_next_tok()) == Tok::ID);
    }

    if (cur_tok != Tok::P_CLOSE)
        return nullptr;

    if ((cur_tok = lex.get_next_tok()) == Tok::SEMICOLON)
        return std::make_unique<FnDecl>(std::move(id), std::move(params));

    if (cur_tok != Tok::BR_OPEN)
        return nullptr;

    std::vector<std::unique_ptr<Statement>> fn_body;
    lex.get_next_tok();
    while (lex.get_tok() != Tok::BR_CLOSE) {
        auto statement = parse_fn_body();
        if (!statement)
            return nullptr;
        fn_body.push_back(std::move(statement));
    }

    lex.get_next_tok();
    auto decl = std::make_unique<FnDecl>(std::move(id), std::move(params));
    return std::make_unique<FnDef>(std::move(decl), std::move(fn_body));
}

std::unique_ptr<Statement> Parser::parse_fn_body() {
    Tok cur_tok = lex.get_tok();

    switch (cur_tok) {
        case Tok::SEMICOLON:
            lex.get_next_tok();
            return parse_fn_body();
        case Tok::T_DOUBLE:
            return parse_var_decl();
        case Tok::ID:
        case Tok::L_DOUBLE:
            return parse_top_expr();
        default:
            return nullptr;
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

    lex.get_next_tok();
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
    Tok cur_tok = lex.get_tok();
    if (cur_tok == Tok::L_DOUBLE) {
        auto expr = std::make_unique<LiteralExpr<double>>(lex.get_double());
        lex.get_next_tok();
        return std::move(expr);
    } else if (cur_tok != Tok::ID) {
        return nullptr;
    }

    lex.get_next_tok();

    std::string id = lex.get_id();
    if (lex.get_tok() != Tok::P_OPEN)
        return std::make_unique<IdExpr>(std::move(id));

    std::vector<std::unique_ptr<Expr>> args;
    if ((cur_tok = lex.get_next_tok()) != Tok::P_CLOSE) {
        do {
            args.push_back(parse_expr());
            if (!args.back())
                return nullptr;
        } while (lex.get_tok() == Tok::COMMA
                 && lex.get_next_tok() != Tok::P_CLOSE);
    }

    lex.get_next_tok();
    return std::make_unique<CallExpr>(std::move(id), std::move(args));
}
