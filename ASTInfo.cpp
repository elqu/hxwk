#include "ASTInfo.hpp"
#include <memory>
#include <vector>

void ExprInfoVis::visit(LiteralExpr<double> &expr) {
    str = std::to_string(expr.get_val());
}

void ExprInfoVis::visit(IdExpr &expr) {
    str = expr.get_id();
}

void ExprInfoVis::visit(BinaryExpr &expr) {
    ExprInfoVis lhs, rhs;
    expr.get_lhs().accept(lhs);
    expr.get_rhs().accept(rhs);
    str = "(" + lhs.str + " [Operator "
          + std::to_string(static_cast<int>(expr.get_op())) + "] " + rhs.str
          + ")";
}

void ExprInfoVis::visit(CallExpr &expr) {
    str = expr.get_id() + "(";
    for (const auto &arg : expr.get_args()) {
        ExprInfoVis vis;
        arg->accept(vis);
        str += vis.get_str() + ", ";
    }
    str.erase(str.size() - 2, 2);
    str += ")";
}

void SynInfoVis::visit(Expr &expr) {
    ExprInfoVis expr_vis;
    expr.accept(expr_vis);
    str = expr_vis.get_str();
}

void SynInfoVis::visit(VarDecl &expr) {
    ExprInfoVis expr_vis;
    expr.get_rhs().accept(expr_vis);
    str = "let " + expr.get_id() + " = " + expr_vis.get_str();
}

void SynInfoVis::visit(FnDecl &decl) {
    str = "fn " + decl.get_id() + "(";

    if (decl.get_params().size() == 0) {
        str += ");";
        return;
    }

    for (const auto &param : decl.get_params())
        str += param + ", ";
    str.erase(str.size() - 2, 2);
    str += ");";
}

void SynInfoVis::visit(FnDef &def) {
    SynInfoVis vis;
    def.get_decl().accept(vis);
    str = vis.str;
    str.pop_back();
    str += " {";

    for (const auto &statement : def.get_body()) {
        statement->accept(vis);
        str += "\n    " + vis.get_str();
    }

    str += "\n}";
}
