//-------------------------------------------------------------------------- //
//                                   Lexer
//-------------------------------------------------------------------------- //

#include <cctype>
#include <cstdio>

#include <string>

using namespace std::literals::string_literals;

enum class Tok {
    INVALID,
    END,
    SEMICOLON,
    EQ,
    PLUS,
    L_DOUBLE,
    T_DOUBLE,
    ID
};

class Lexer {
    public:
        Tok get_tok() {return cur_tok;};
        Tok get_next_tok();
        std::string get_id() {return id;};
        double get_double() {return l_double;};
    private:
        Tok cur_tok = Tok::INVALID;
        std::string id;
        double l_double;
};

Tok Lexer::get_next_tok() {
    if(cur_tok == Tok::END)
        return cur_tok;

    int cur_char;
    while(std::isspace(cur_char = std::getchar()));

    switch(cur_char) {
        case EOF:
            return cur_tok = Tok::END;
        case ';':
            return cur_tok = Tok::SEMICOLON;
        case '=':
            return cur_tok = Tok::EQ;
        case '+':
            return cur_tok = Tok::PLUS;
    }

    if(std::isalpha(cur_char)) {
        id = cur_char;
        while(std::isalnum(cur_char = std::getchar()))
            id += cur_char;
        std::ungetc(cur_char, stdin);

        if(id == "double"s)
            return cur_tok = Tok::T_DOUBLE;

        return cur_tok = Tok::ID;
    }

    const bool is_point = cur_char == '.';

    if(std::isdigit(cur_char) || is_point) {
        // TODO: Find out, why the following line doesn't work as intended
        // std::string number{1, static_cast<char>(cur_char)};
        std::string number;
        number = cur_char;

        if(!is_point) {
            while(std::isdigit(cur_char = std::getchar()))
                number += cur_char;

            if(cur_char != '.')
                return cur_tok = Tok::INVALID;
            number += '.';
        }

        while(std::isdigit(cur_char = std::getchar()))
            number += cur_char;
        std::ungetc(cur_char, stdin);

        if(number == "."s)
            return cur_tok = Tok::INVALID;

        l_double = std::stod(number);
        return cur_tok = Tok::L_DOUBLE;
    }

    return cur_tok = Tok::INVALID;
}

//---------------------------------------------------------------------------//
//                                Syntax tree
//---------------------------------------------------------------------------//

class Statement {
    public:
        virtual ~Statement() = default;
};

class Expr : public Statement {
};

template<typename T>
class LiteralExpr : public Expr {
    public:
        LiteralExpr(T val) : val(val) {};
    private:
        T val;
};

class IdExpr : public Expr {
    public:
        IdExpr(std::string id) : id(std::move(id)) {};
    private:
        std::string id;
};

class BinaryExpr : public Expr {
    public:
        BinaryExpr(Tok op, std::unique_ptr<Expr> lhs,
                           std::unique_ptr<Expr> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {};
    private:
        Tok op;
        std::unique_ptr<Expr> lhs, rhs;
};

class VarDecl : public Statement {
    public:
        VarDecl(Tok type, std::string id, std::unique_ptr<Expr> rhs)
            : type(type), id(std::move(id)), rhs(std::move(rhs)) {};
    private:
        Tok type;
        std::string id;
        std::unique_ptr<Expr> rhs;
};

//---------------------------------------------------------------------------//
//                                  Parser
//---------------------------------------------------------------------------//

#include <map>

enum class Assoc {
    LEFT,
    RIGHT
};

static std::map<Tok, std::pair<int, Assoc>> binary_precedence{
    {Tok::EQ, {10, Assoc::RIGHT}},
    {Tok::PLUS, {20, Assoc::LEFT}},
};

class Parser {
    public:
        Parser(Lexer lex) : lex(std::move(lex)) {};
        std::pair<int, Assoc> get_precedence(Tok tok);
        std::unique_ptr<Statement> parse_primary();
        std::unique_ptr<VarDecl> parse_var_decl();
        std::unique_ptr<Expr> parse_top_expr();
        std::unique_ptr<Expr> parse_expr();
        std::unique_ptr<Expr> parse_expr_rhs(int precedence,
                                             Assoc lr_ass,
                                             std::unique_ptr<Expr> lhs);
    private:
        Lexer lex;
};

std::pair<int, Assoc> Parser::get_precedence(Tok tok) {
    return binary_precedence[tok];
}

std::unique_ptr<Statement> Parser::parse_primary() {
    if(lex.get_tok() == Tok::END)
        return nullptr;

    Tok cur_tok = lex.get_next_tok();

    switch(cur_tok) {
        case Tok::INVALID:
        case Tok::END:
        case Tok::PLUS:
        case Tok::EQ:
            return nullptr;
        case Tok::SEMICOLON:
            return parse_primary();
        case Tok::T_DOUBLE:
            return parse_var_decl();
        case Tok::ID:
        case Tok::L_DOUBLE:
            return parse_top_expr();
    }
}

std::unique_ptr<VarDecl> Parser::parse_var_decl() {
    Tok op = lex.get_tok();
    Tok cur_tok = lex.get_next_tok();
    if(cur_tok != Tok::ID)
        return nullptr;

    auto id = lex.get_id();

    cur_tok = lex.get_next_tok();
    if(cur_tok != Tok::EQ)
        return nullptr;

    lex.get_next_tok();
    auto expr = parse_expr();
    if(!expr)
        return nullptr;

    return std::make_unique<VarDecl>(op, id, expr);
}

std::unique_ptr<Expr> Parser::parse_top_expr() {
    auto expr = parse_expr();
    lex.get_next_tok();
    return expr;
}

std::unique_ptr<Expr> Parser::parse_expr() {
    Tok cur_tok = lex.get_tok();
    if(cur_tok == Tok::L_DOUBLE);
}


std::unique_ptr<Expr> Parser::parse_expr_rhs(int precedence,
                                             Assoc lr_ass,
                                             std::unique_ptr<Expr> lhs) {
}

//---------------------------------------------------------------------------//
//                               Main function
//---------------------------------------------------------------------------//

int main() {
    Lexer lex;
    Tok cur_tok = Tok::INVALID;

    while(cur_tok != Tok::END)
        printf("%d", cur_tok = lex.get_next_tok());
    printf("\n");

    return 0;
}
