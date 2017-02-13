#include "Lexer.hpp"
#include <cctype>
#include <cstdio>

Tok Lexer::get_next_tok() {
    if (cur_tok == Tok::END)
        return cur_tok;

    int cur_char;
    while (std::isspace(cur_char = std::getchar()))
        ;

    switch (cur_char) {
        case '/':
            if ((cur_char = std::getchar()) == '/') {
                // The carriage return preceding a newline on e.g. Windows
                // machines should just be consumed by getchar and
                // theoretically not bother the lexing, but this behaviour
                // remains to be tested.
                while ((cur_char = std::getchar()) != '\n' && cur_char != EOF)
                    ;
                return get_next_tok();
            } else {
                std::ungetc(cur_char, stdin);
                cur_char = '/';
            }
            return cur_tok = Tok::INVALID;
        case EOF:
            return cur_tok = Tok::END;
        case ',':
            return cur_tok = Tok::COMMA;
        case ';':
            return cur_tok = Tok::SEMICOLON;
        case '=':
            return cur_tok = Tok::EQ;
        case '+':
            return cur_tok = Tok::PLUS;
        case '*':
            return cur_tok = Tok::MULT;
        case '(':
            return cur_tok = Tok::P_OPEN;
        case ')':
            return cur_tok = Tok::P_CLOSE;
        case '{':
            return cur_tok = Tok::BR_OPEN;
        case '}':
            return cur_tok = Tok::BR_CLOSE;
    }

    if (std::isalpha(cur_char)) {
        id = cur_char;
        while (std::isalnum(cur_char = std::getchar()))
            id += cur_char;
        std::ungetc(cur_char, stdin);

        if (id == "fn")
            return cur_tok = Tok::FN;

        if (id == "double")
            return cur_tok = Tok::T_DOUBLE;

        return cur_tok = Tok::ID;
    }

    const bool is_point = (cur_char == '.');

    if (std::isdigit(cur_char) || is_point) {
        // TODO: Find out, why the following line doesn't work as intended
        // std::string number{1, static_cast<char>(cur_char)};
        std::string number;
        number = cur_char;

        if (!is_point) {
            while (std::isdigit(cur_char = std::getchar()))
                number += cur_char;

            if (cur_char != '.')
                return cur_tok = Tok::INVALID;
            number += '.';
        }

        while (std::isdigit(cur_char = std::getchar()))
            number += cur_char;
        std::ungetc(cur_char, stdin);

        if (number == ".")
            return cur_tok = Tok::INVALID;

        l_double = std::stod(number);
        return cur_tok = Tok::L_DOUBLE;
    }

    return cur_tok = Tok::INVALID;
}
