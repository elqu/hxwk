#ifndef HXWK_PARSER_H
#define HXWK_PARSER_H

#include "Lexer.hpp"
#include <memory>
#include <utility>

class Expr;
class Statement;
class VarDecl;
class ScopeExpr;

enum class Assoc { LEFT, RIGHT };

class Parser {
  public:
    Parser(Lexer lex) : lex(std::move(lex)){};
    std::pair<int, Assoc> get_precedence(Tok tok);
    std::unique_ptr<Statement> parse();
    std::unique_ptr<Statement> parse_fn();
    std::unique_ptr<Statement> parse_scope_body();
    std::unique_ptr<VarDecl> parse_var_decl();
    std::unique_ptr<Expr> parse_top_expr();
    std::unique_ptr<Expr> parse_expr();
    std::unique_ptr<Expr> parse_expr_rhs(int precedence,
                                         std::unique_ptr<Expr> lhs);
    std::unique_ptr<Expr> parse_primary();
    std::unique_ptr<ScopeExpr> parse_scope();

  private:
    Lexer lex;
};

#endif
