#include "ASTInfo.hpp"

void ExprInfoVis::visit(LiteralExpr<double>& expr) {
    str = std::to_string(expr.get_val());
}

void ExprInfoVis::visit(IdExpr& expr) {
    str = expr.get_id();
}

void ExprInfoVis::visit(BinaryExpr& expr) {
    ExprInfoVis lhs, rhs;
    expr.get_lhs().accept(lhs);
    expr.get_rhs().accept(rhs);
    str = "(" + lhs.str + " [Operator "
        + std::to_string(static_cast<int>(expr.get_op()))
        + "] " + rhs.str + ")";
}

void SynInfoVis::visit(Expr& expr) {
    ExprInfoVis expr_vis;
    expr.accept(expr_vis);
    str = expr_vis.get_str();
}

void SynInfoVis::visit(VarDecl& expr) {
    ExprInfoVis expr_vis;
    expr.get_rhs().accept(expr_vis);
    str = "double " + expr.get_id() + " = " + expr_vis.get_str();
}
