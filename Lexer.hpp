#ifndef HXWK_LEXER_H
#define HXWK_LEXER_H

#include <string>

enum class Tok {
    INVALID,
    END,
    COMMA,
    SEMICOLON,
    LET,
    IF,
    ELSE,
    EQ,
    PLUS,
    MINUS,
    MULT,
    SLASH,
    CMP_LT,
    L_INT32,
    L_DOUBLE,
    L_STR,
    ID,
    FN,
    P_OPEN,
    P_CLOSE,
    BR_OPEN,
    BR_CLOSE,
    RARROW,
};

struct CodeLocation {
    std::size_t line, col;
};

class Lexer {
  public:
    Lexer() { get_next_tok(); };
    Tok get_tok() const { return cur_tok; };
    Tok get_next_tok();
    std::string get_id() const { return id; };
    int32_t get_int32() const { return l_int32; };
    double get_double() const { return l_double; };
    CodeLocation get_loc() const { return cur_loc; };

  private:
    static constexpr int eof = std::char_traits<char>::eof();
    int get_char();
    int peek_char();

    Tok cur_tok{Tok::INVALID};
    std::string id;
    int32_t l_int32;
    double l_double;
    CodeLocation cur_loc{1, 0};
};

#endif
