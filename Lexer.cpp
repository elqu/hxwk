#include "Lexer.hpp"
#include "Log.hpp"
#include <cctype>
#include <iostream>

#define error_inv(...) Log::error_val<Tok, Tok::INVALID>(cur_loc, __VA_ARGS__)

int Lexer::get_char() {
    int cur_char = std::cin.get();

    if (cur_char == '\n') {
        ++cur_loc.line;
        cur_loc.col = 0;
    } else {
        ++cur_loc.col;
    }

    return cur_char;
}

int Lexer::peek_char() {
    return std::cin.peek();
}

Tok Lexer::get_next_tok() {
    if (cur_tok == Tok::END)
        return cur_tok;

    int cur_char;
    while (std::isspace(cur_char = get_char()))
        ;

    switch (cur_char) {
        case '/':
            if (peek_char() == '/') {
                // The carriage return preceding a newline on e.g. Windows
                // machines should just be consumed by getchar and
                // theoretically not bother the lexing, but this behaviour
                // remains to be tested.
                while ((cur_char = get_char()) != '\n' && cur_char != eof)
                    ;
                return get_next_tok();
            }
            return cur_tok = Tok::SLASH;
        case eof:
            return cur_tok = Tok::END;
        case ',':
            return cur_tok = Tok::COMMA;
        case ';':
            return cur_tok = Tok::SEMICOLON;
        case '=':
            return cur_tok = Tok::EQ;
        case '+':
            return cur_tok = Tok::PLUS;
        case '-':
            if (peek_char() == '>') {
                get_char();
                return cur_tok = Tok::RARROW;
            }
            return cur_tok = Tok::MINUS;
        case '*':
            return cur_tok = Tok::MULT;
        case '<':
            return cur_tok = Tok::CMP_LT;
        case '(':
            return cur_tok = Tok::P_OPEN;
        case ')':
            return cur_tok = Tok::P_CLOSE;
        case '{':
            return cur_tok = Tok::BR_OPEN;
        case '}':
            return cur_tok = Tok::BR_CLOSE;
        case '"':
            id.clear();
            while ((cur_char = get_char()) != '"' && cur_char != eof) {
                if (cur_char == '\n')
                    continue;

                if (cur_char != '\\') {
                    id += cur_char;
                    continue;
                }

                switch (get_char()) {
                    case 'n':
                        id += '\n';
                        break;
                    case '\\':
                        id += '\\';
                        break;
                    case '"':
                        id += '"';
                        break;
                }
            }
            return cur_tok = Tok::L_STR;
    }

    if (std::isalpha(cur_char)) {
        id = cur_char;
        while (std::isalnum(peek_char()))
            id += get_char();

        if (id == "let")
            return cur_tok = Tok::LET;

        if (id == "if")
            return cur_tok = Tok::IF;

        if (id == "else")
            return cur_tok = Tok::ELSE;

        if (id == "fn")
            return cur_tok = Tok::FN;

        return cur_tok = Tok::ID;
    }

    const bool is_point = (cur_char == '.');

    if (std::isdigit(cur_char) || is_point) {
        // TODO: Find out, why the following line doesn't work as intended
        // std::string number{1, static_cast<char>(cur_char)};
        std::string number;
        number = cur_char;

        if (!is_point) {
            while (std::isdigit(cur_char = get_char()))
                number += cur_char;

            if (cur_char != '.')
                return error_inv("Expected double literal");
            number += '.';
        }

        while (std::isdigit(peek_char()))
            number += get_char();

        if (number == ".")
            return error_inv(
                    "Expected numbers following or preceding decimal mark "
                    "`.`");

        l_double = std::stod(number);
        return cur_tok = Tok::L_DOUBLE;
    }
    return error_inv("Invalid character `", static_cast<char>(cur_char), "`");
}
