#ifndef HXWK_LOG_H
#define HXWK_LOG_H

#include "Lexer.hpp"
#include <iostream>
#include <utility>

class Log {
  public:
    Log() = delete;

    template <typename... Args>
    static void error(Args &&... args) {
        std::cerr << "Error: ";
#ifdef __cpp_fold_expressions
        (std::cerr << ... << std::forward<Args>(args)) << '\n';
#else
        fold_helper((std::cerr << std::forward<Args>(args))...);
        std::cerr << '\n';
#endif
    }

    template <typename... Args>
    static void error(CodeLocation loc, Args &&... args) {
        std::cerr << loc.line << ':' << loc.col << ": ";
        error(std::forward<Args>(args)...);
    }

    template <typename T, T v = T{}, typename... Args>
    static T error_val(Args &&... args) {
        error(std::forward<Args>(args)...);
        return v;
    }

    template <typename T, T v = T{}, typename... Args>
    static T error_val(CodeLocation loc, Args &&... args) {
        std::cerr << loc.line << ':' << loc.col << ": ";
        return error_val<T, v>(std::forward<Args>(args)...);
    }

  private:
#ifndef __cpp_fold_expressions
    template <typename... Args>
    static void fold_helper(Args &&...) {}
#endif
};

#endif
