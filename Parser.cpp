#include "Parser.hpp"
#include "AST.hpp"
#include "C++11Compat.hpp"
#include "Log.hpp"
#include "Type.hpp"
#include <map>
#include <string>
#include <tuple>
#include <vector>

#define error_null(...)                                                       \
    Log::error_val<std::nullptr_t>(lex.get_loc(), __VA_ARGS__)

static std::map<Tok, std::pair<int, Assoc>> binary_precedence{
        {Tok::EQ, {10, Assoc::RIGHT}},  {Tok::CMP_LT, {17, Assoc::LEFT}},
        {Tok::PLUS, {20, Assoc::LEFT}}, {Tok::MINUS, {20, Assoc::LEFT}},
        {Tok::MULT, {30, Assoc::LEFT}}, {Tok::SLASH, {30, Assoc::LEFT}}};

std::unique_ptr<Statement> Parser::parse() {
    switch (lex.get_tok()) {
        case Tok::SEMICOLON:
            lex.get_next_tok();
            return parse();
        case Tok::FN:
            return parse_fn();
        case Tok::END:
            return nullptr;
        default:
            return error_null("Expected function declaration");
    }
}

// Differs from the other functions as it does not expect its first token to be
// valid.
std::unique_ptr<Type> Parser::parse_type() {
    if (lex.get_tok() != Tok::ID)
        return error_null("Expected type identifier");

    auto type_id = lex.get_id();

    if (type_id == "double") {
        return std::make_unique<DoubleType>();
    } else if (type_id == "i32") {
        return std::make_unique<Int32Type>();
    } else if (type_id == "bool") {
        return std::make_unique<BoolType>();
    } else if (type_id == "void") {
        return std::make_unique<VoidType>();
    } else {
        return error_null("Unknown type identifier ", type_id);
    }
}

std::unique_ptr<Statement> Parser::parse_fn() {
    Tok cur_tok = lex.get_next_tok();
    if (cur_tok != Tok::ID)
        return error_null("Expected identifier");
    auto id = lex.get_id();

    if ((cur_tok = lex.get_next_tok()) != Tok::P_OPEN)
        return error_null("Expected opening parenthesis `(`");

    std::vector<FnDecl::Param_t> params;

    if ((cur_tok = lex.get_next_tok()) == Tok::ID) {
        do {
            auto id = lex.get_id();

            if (lex.get_next_tok() != Tok::COLON)
                return error_null("Expected colon `:`");
            lex.get_next_tok();

            auto type = parse_type();
            if (!type)
                return nullptr;

            params.push_back({std::move(id), std::move(type)});
        } while ((cur_tok = lex.get_next_tok()) == Tok::COMMA
                 && (cur_tok = lex.get_next_tok()) == Tok::ID);
    }

    if (cur_tok != Tok::P_CLOSE)
        return error_null("Expected closing parenthesis `)`");

    if ((cur_tok = lex.get_next_tok()) != Tok::RARROW)
        return error_null("Expected right arrow `->`");

    lex.get_next_tok();
    auto ret_type = parse_type();

    if ((cur_tok = lex.get_next_tok()) == Tok::SEMICOLON)
        return std::make_unique<FnDecl>(std::move(id), std::move(params),
                                        std::move(ret_type));

    if (cur_tok != Tok::BR_OPEN)
        return error_null("Expected opening brace `{`");

    auto fn_body_scope = parse_scope();
    if (!fn_body_scope)
        return nullptr;

    auto decl = std::make_unique<FnDecl>(std::move(id), std::move(params),
                                         std::move(ret_type));
    return std::make_unique<FnDef>(std::move(decl), std::move(fn_body_scope));
}

std::unique_ptr<Statement> Parser::parse_scope_body() {
    switch (Tok cur_tok = lex.get_tok()) {
        case Tok::SEMICOLON:
            lex.get_next_tok();
            return parse_scope_body();
        case Tok::LET:
            return parse_var_decl();
        case Tok::IF:
        case Tok::BR_OPEN:
        case Tok::P_OPEN:
        case Tok::ID:
        case Tok::L_INT32:
        case Tok::L_DOUBLE:
        case Tok::L_STR:
            return parse_expr();
        default:
            return error_null("Expected variable declaration or expression");
    }
}

std::unique_ptr<VarDecl> Parser::parse_var_decl() {
    Tok cur_tok = lex.get_next_tok();
    if (cur_tok != Tok::ID)
        return error_null("Expected identifier");

    auto id = lex.get_id();

    cur_tok = lex.get_next_tok();
    if (cur_tok != Tok::EQ)
        return error_null("Expected equality sign `=`");

    lex.get_next_tok();
    auto expr = parse_expr();
    if (!expr)
        return nullptr;

    return std::make_unique<VarDecl>(id, std::move(expr));
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
        std::tie(op_prec, lr_ass) = binary_precedence[op];

        if (op_prec == 0 || op_prec < expr_prec)
            return lhs;

        lex.get_next_tok();

        auto rhs = parse_primary();
        if (!rhs)
            return nullptr;

        Tok new_op = lex.get_tok();
        int new_op_prec = binary_precedence[new_op].first;

        if (new_op_prec > op_prec
            || (new_op_prec != 0 && new_op_prec == op_prec
                && lr_ass == Assoc::RIGHT)) {
            rhs = parse_expr_rhs(op_prec, std::move(rhs));
        }

        lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<Expr> Parser::parse_primary() {
    Tok cur_tok = lex.get_tok();
    if (cur_tok == Tok::L_DOUBLE) {
        auto val = lex.get_double();
        lex.get_next_tok();
        return std::make_unique<LiteralExpr<double>>(val);
    } else if (cur_tok == Tok::L_INT32) {
        auto val = lex.get_int32();
        lex.get_next_tok();
        return std::make_unique<LiteralExpr<int32_t>>(val);
    } else if (cur_tok == Tok::L_STR) {
        auto str_lit = lex.get_id();
        lex.get_next_tok();
        return std::make_unique<LiteralExpr<std::string>>(std::move(str_lit));
    } else if (cur_tok == Tok::P_OPEN) {
        lex.get_next_tok();
        auto expr = parse_expr();
        if (!expr)
            return nullptr;
        if (lex.get_tok() != Tok::P_CLOSE)
            return error_null("Expected closing parenthesis `)`");
        lex.get_next_tok();
        return expr;
    } else if (cur_tok == Tok::BR_OPEN) {
        return parse_scope();
    } else if (cur_tok == Tok::IF) {
        lex.get_next_tok();
        auto cond = parse_expr();
        if (!cond)
            return nullptr;

        if (lex.get_tok() != Tok::BR_OPEN)
            return error_null("Expected opening brace `{`");
        auto then = parse_scope();
        if (!then)
            return nullptr;

        if (lex.get_tok() != Tok::ELSE)
            return error_null("Expected keyword `else`");
        if (lex.get_next_tok() != Tok::BR_OPEN)
            return error_null("Expected opening brace `{`");
        auto or_else = parse_scope();
        if (!or_else)
            return nullptr;

        return std::make_unique<IfExpr>(std::move(cond), std::move(then),
                                        std::move(or_else));
    } else if (cur_tok != Tok::ID) {
        return error_null("Expected primary expression");
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

std::unique_ptr<ScopeExpr> Parser::parse_scope() {
    lex.get_next_tok();

    std::vector<std::unique_ptr<Statement>> body;
    body.push_back(nullptr);

    while (lex.get_tok() != Tok::BR_CLOSE) {
        body.back() = parse_scope_body();
        if (!body.back())
            return nullptr;

        if (lex.get_tok() == Tok::SEMICOLON) {
            lex.get_next_tok();
            body.push_back(nullptr);
        } else if (lex.get_tok() != Tok::BR_CLOSE) {
            return error_null("Expected semicolon `;` or closing brace `}`");
        }
    }

    lex.get_next_tok();
    return std::make_unique<ScopeExpr>(std::move(body));
}
