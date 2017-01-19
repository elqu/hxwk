//---------------------------------------------------------------------------//
//                                   Lexer                                   //
//---------------------------------------------------------------------------//

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
//                                Syntax tree                                //
//---------------------------------------------------------------------------//

class Statement {
    public:
        virtual ~Statement() = default;
        virtual std::string str() = 0;
};

class Expr : public Statement {
};

template<typename T>
class LiteralExpr : public Expr {
    public:
        LiteralExpr(T val) : val(val) {};
        std::string str();
    private:
        T val;
};

template<>
std::string LiteralExpr<double>::str() {
    return std::to_string(val);
}

class IdExpr : public Expr {
    public:
        IdExpr(std::string id) : id(std::move(id)) {};
        std::string str();
    private:
        std::string id;
};

std::string IdExpr::str() {
    return id;
}

class BinaryExpr : public Expr {
    public:
        BinaryExpr(Tok op, std::unique_ptr<Expr> lhs,
                           std::unique_ptr<Expr> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {};
        std::string str();
    private:
        Tok op;
        std::unique_ptr<Expr> lhs, rhs;
};

std::string BinaryExpr::str() {
    return "("s + lhs->str() + " [Operator "s
        + std::to_string(static_cast<int>(op)) + "] "s + rhs->str() + ")"s;
}

class VarDecl : public Statement {
    public:
        VarDecl(Tok type, std::string id, std::unique_ptr<Expr> rhs)
            : type(type), id(std::move(id)), rhs(std::move(rhs)) {};
        std::string str();
    private:
        Tok type; // TODO: implement a "type" type for more than just POD types
        std::string id;
        std::unique_ptr<Expr> rhs;
};

std::string VarDecl::str() {
    return "double "s + id + " = "s + rhs->str();
}

//---------------------------------------------------------------------------//
//                                  Parser                                   //
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
        std::unique_ptr<Statement> parse();
        std::unique_ptr<VarDecl> parse_var_decl();
        std::unique_ptr<Expr> parse_top_expr();
        std::unique_ptr<Expr> parse_expr();
        std::unique_ptr<Expr> parse_expr_rhs(int precedence,
                                             std::unique_ptr<Expr> lhs);
        std::unique_ptr<Expr> parse_primary();
    private:
        Lexer lex;
};

std::pair<int, Assoc> Parser::get_precedence(Tok tok) {
    auto precedence = binary_precedence[tok];

    if(precedence.first == 0)
        return {-1, Assoc::LEFT};

    return precedence;
}

std::unique_ptr<Statement> Parser::parse() {
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
            return parse();
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

    return std::make_unique<VarDecl>(op, id, std::move(expr));
}

std::unique_ptr<Expr> Parser::parse_top_expr() {
    auto expr = parse_expr();

    if(lex.get_tok() != Tok::SEMICOLON)
        return nullptr;

    lex.get_next_tok();
    return expr;
}

std::unique_ptr<Expr> Parser::parse_expr() {
    return parse_expr_rhs(0, parse_primary());
}

std::unique_ptr<Expr> Parser::parse_expr_rhs(int expr_prec,
                                             std::unique_ptr<Expr> lhs) {
    while(true) {
        Tok op = lex.get_tok();
        int op_prec;
        Assoc lr_ass;
        std::tie(op_prec, lr_ass) = get_precedence(op);

        if(op_prec < expr_prec)
            return lhs;

        lex.get_next_tok();

        auto rhs = parse_primary();
        if(!rhs)
            return nullptr;

        Tok new_op = lex.get_tok();
        int new_op_prec = get_precedence(new_op).first;

        if(new_op_prec > op_prec
                || (new_op_prec == op_prec && lr_ass == Assoc::RIGHT)) {
            rhs = parse_expr_rhs(op_prec, std::move(rhs));
        }

        lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<Expr> Parser::parse_primary() {
    std::unique_ptr<Expr> expr;

    Tok cur_tok = lex.get_tok();
    if(cur_tok == Tok::L_DOUBLE) {
        expr = std::make_unique<LiteralExpr<double>>(lex.get_double());
    } else if(cur_tok == Tok::ID) {
        expr = std::make_unique<IdExpr>(lex.get_id());
    } else {
        return nullptr;
    }

    lex.get_next_tok();
    return expr;
}

//---------------------------------------------------------------------------//
//                               Main function                               //
//---------------------------------------------------------------------------//

int main() {
    //Lexer lex;
    //Tok cur_tok = Tok::INVALID;

    //while(cur_tok != Tok::END)
        //printf("%d", cur_tok = lex.get_next_tok());
    //printf("\n");

    Parser par{Lexer{}};
    std::unique_ptr<Statement> ast = par.parse();
    printf("%s;\n", ast->str().c_str());

    return 0;
}
