#include "IRGenerator.hpp"

IRGenerator::IRGenerator(llvm::StringRef name)
    : builder{context}, module{std::move(name), context} {
    auto *main_type
        = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), false);

    auto *main_fn = llvm::Function::Create(main_type,
                                           llvm::Function::ExternalLinkage,
                                           "main", &module);

    auto *bb = llvm::BasicBlock::Create(context, "entry", main_fn);
    builder.SetInsertPoint(bb);
}

void IRGenerator::finish(const IRStatementVis& vis) {
    builder.CreateRet(vis.get_val());
}

void IRExprVis::visit(LiteralExpr<double>& expr) {
    val = llvm::ConstantFP::get(gen.context, llvm::APFloat{expr.get_val()});
}

void IRExprVis::visit(IdExpr& expr) {
    val = gen.named_values[expr.get_id()];
}

void IRExprVis::visit(BinaryExpr& expr) {
    IRExprVis lhs{gen}, rhs{gen};
    expr.get_lhs().accept(lhs);
    expr.get_rhs().accept(rhs);
    if(!lhs.val || !rhs.val) {
        val = nullptr;
        return;
    }

    // TODO: assignment operator
    switch(expr.get_op()) {
        case Tok::PLUS:
            val = gen.builder.CreateFAdd(lhs.val, rhs.val);
            break;
        case Tok::MULT:
            val = gen.builder.CreateFMul(lhs.val, rhs.val);
            break;
        default:
            val = nullptr;
            break;
    }
}

void IRStatementVis::visit(Expr& expr) {
    IRExprVis expr_vis{gen};
    expr.accept(expr_vis);
    val = expr_vis.get_val();
}

void IRStatementVis::visit(VarDecl& decl) {
    val = nullptr;
}
