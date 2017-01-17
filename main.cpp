// ------------------------------------------------------------------------- //
//                                   Lexer
// ------------------------------------------------------------------------- //

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
        [[deprecated]] Tok getTok() {return cur_tok;};
        Tok getNextTok();
        std::string getId() {return id;};
        double getDouble() {return l_double;};
    private:
        Tok cur_tok = Tok::INVALID;
        std::string id;
        double l_double;
};

Tok Lexer::getNextTok() {
    if(cur_tok == Tok::END)
        return cur_tok;

    int cur_char;
    // TODO: Is the following too unreadable?
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
        //std::string number{1, static_cast<char>(cur_char)};
        std::string number;
        number = cur_char;
        //printf("sag waddup %s\n", number.c_str());

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

// --------------------------------------------------------------------------//
//                                Syntax tree
// --------------------------------------------------------------------------//

class Expr {
    public:
        virtual ~Expr() = default;
};

template<typename T>
class LiteralExpr : public Expr {
    public:
        LiteralExpr(T val) : val(val) {};
    private:
        T val;
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

class VariableDecl {
    public:
        VariableDecl(Tok type, std::string id, std::unique_ptr<Expr> rhs)
            : type(type), id(std::move(id)), rhs(std::move(rhs)) {};
    private:
        Tok type;
        std::string id;
        std::unique_ptr<Expr> rhs;
};

// --------------------------------------------------------------------------//
//                               Main function
// --------------------------------------------------------------------------//

int main() {
    Lexer lex;
    Tok cur_tok = Tok::INVALID;

    while(cur_tok != Tok::END)
        printf("%d", cur_tok = lex.getNextTok());
    printf("\n");

    return 0;
}
