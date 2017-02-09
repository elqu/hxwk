#ifndef HXWK_LEXER_H
#define HXWK_LEXER_H

#include <string>

enum class Tok {
    INVALID,
    END,
    SEMICOLON,
    EQ,
    PLUS,
    MULT,
    L_DOUBLE,
    T_DOUBLE,
    ID
};

class Lexer {
    public:
        Tok get_tok() const {return cur_tok;};
        Tok get_next_tok();
        std::string get_id() const {return id;};
        double get_double() const {return l_double;};
    private:
        Tok cur_tok = Tok::INVALID;
        std::string id;
        double l_double;
};

#endif
