#include "AST.hpp"
#include "ASTInfo.hpp"
#include "IRGenerator.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "VisitorPattern.hpp"
#include "llvm/ADT/StringRef.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

static void show_usage(std::string name) {
    std::cerr << "Usage: " << name << " [option(s)] VALUES\n"
              << "Options:\n"
              << "\t-h, --help\t\tShow this help message\n";
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 1;
        }
    }

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

    //std::ofstream out_file{"out.bc", std::ios_base::binary};
    std::ofstream out_file{"out.ll"};
    if (out_file.fail())
        return 1;
    //gen.write_bitcode(out_file);
    gen.write_assembly(out_file);

    return 0;
}
