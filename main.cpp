#include "AST.hpp"
#include "ASTInfo.hpp"
#include "IRGenerator.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "VisitorPattern.hpp"
#include "llvm/ADT/StringRef.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

static void show_usage() {
    std::cerr << "Usage: " << argv[0] << " [option(s)] VALUES"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << std::endl;
}

int main(int argc, char** argv) {
    Parser par{Lexer{}};
    IRGenerator gen{"Hexenwerk"};
    IRStatementVis vis_code{gen};

    while (std::unique_ptr<Statement> ast = par.parse()) {
        SynInfoVis vis_info;
        ast->accept(vis_info);
        std::printf("Info:\n%s\n", vis_info.get_str().c_str());

        ast->accept(vis_code);
        if (vis_code.get_val() == nullptr) {
            break;
        }
    }

    gen.print();    

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || ("arg == "--help")) {
            show_usage();
        }
    }

    return 0;
}
