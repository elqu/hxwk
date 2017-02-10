#include "ASTInfo.hpp"
#include "IRGenerator.hpp"
#include "Parser.hpp"

int main() {
    Parser par{Lexer{}};
    IRGenerator gen{"top"};
    IRStatementVis vis_code{gen};

    while (std::unique_ptr<Statement> ast = par.parse()) {
        SynInfoVis vis_info;
        ast->accept(vis_info);
        ast->accept(vis_code);

        printf("Info: %s\n", vis_info.get_str().c_str());
    }

    gen.finish(vis_code);
    gen.print();

    return 0;
}
