#include "AST.hpp"
#include "ASTInfo.hpp"
#include "IRGenerator.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "VisitorPattern.hpp"
#include "llvm/ADT/StringRef.h"
#include <cstdio>
#include <memory>
#include <string>

int main() {
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

    return 0;
}
