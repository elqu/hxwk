//---------------------------------------------------------------------------//
//                                   Lexer                                   //
//---------------------------------------------------------------------------//

#include <cctype>
#include <cstdio>

#include <string>

using namespace std::literals::string_literals;

enum class Tok {
    INVALID,
    END,
    SEMICOLON,
    EQ,
    PLUS,
    MULT,
    L_DOUBLE,
    T_DOUBLE,
    ID
};

class Lexer {
    public:
        Tok get_tok() {return cur_tok;};
        Tok get_next_tok();
        std::string get_id() {return id;};
        double get_double() {return l_double;};
    private:
        Tok cur_tok = Tok::INVALID;
        std::string id;
        double l_double;
};

Tok Lexer::get_next_tok() {
    if(cur_tok == Tok::END)
        return cur_tok;

    int cur_char;
    while(std::isspace(cur_char = std::getchar()));

    switch(cur_char) {
        case EOF:
            return cur_tok = Tok::END;
        case ';':
            return cur_tok = Tok::SEMICOLON;
        case '=':
            return cur_tok = Tok::EQ;
        case '+':
            return cur_tok = Tok::PLUS;
        case '*':
            return cur_tok = Tok::MULT;
    }

    if(std::isalpha(cur_char)) {
        id = cur_char;
        while(std::isalnum(cur_char = std::getchar()))
            id += cur_char;
        std::ungetc(cur_char, stdin);

        if(id == "double"s)
            return cur_tok = Tok::T_DOUBLE;

        return cur_tok = Tok::ID;
    }

    const bool is_point = cur_char == '.';

    if(std::isdigit(cur_char) || is_point) {
        // TODO: Find out, why the following line doesn't work as intended
        // std::string number{1, static_cast<char>(cur_char)};
        std::string number;
        number = cur_char;

        if(!is_point) {
            while(std::isdigit(cur_char = std::getchar()))
                number += cur_char;

            if(cur_char != '.')
                return cur_tok = Tok::INVALID;
            number += '.';
        }

        while(std::isdigit(cur_char = std::getchar()))
            number += cur_char;
        std::ungetc(cur_char, stdin);

        if(number == "."s)
            return cur_tok = Tok::INVALID;

        l_double = std::stod(number);
        return cur_tok = Tok::L_DOUBLE;
    }

    return cur_tok = Tok::INVALID;
}

//---------------------------------------------------------------------------//
//                         Visitor pattern templates                         //
//---------------------------------------------------------------------------//

#define ABSTR_ACCEPT(visitor_type) \
    virtual void accept(visitor_type& visitor) = 0

#define ACCEPT(visitor_type) \
    void accept(visitor_type& visitor) override {visitor.visit(*this);}

#define ABSTR_VISIT(visitable_type) \
    virtual void visit(visitable_type& visitable) = 0

#define VISIT(visitable_type) \
    void visit(visitable_type& visitable) override;

//---------------------------------------------------------------------------//
//                                Syntax tree                                //
//---------------------------------------------------------------------------//

#include <memory>

class Statement;
    class Expr;
        template<typename T> class LiteralExpr;
        class IdExpr;
        class BinaryExpr;
    class VarDecl;

class StatementVis {
    public:
        virtual ~StatementVis() = default;
        ABSTR_VISIT(Expr);
        ABSTR_VISIT(VarDecl);
};

class ExprVis {
    public:
        virtual ~ExprVis() = default;
        ABSTR_VISIT(LiteralExpr<double>);
        ABSTR_VISIT(IdExpr);
        ABSTR_VISIT(BinaryExpr);
};

class Statement {
    public:
        virtual ~Statement() = default;
        ABSTR_ACCEPT(StatementVis);
};

class Expr : public Statement {
    public:
        ABSTR_ACCEPT(ExprVis);
        ACCEPT(StatementVis);
};

template<typename T>
class LiteralExpr : public Expr {
    public:
        LiteralExpr(T val) : val(val) {};

        T get_val() const {return val;};

        ACCEPT(ExprVis);

    private:
        T val;
};

class IdExpr : public Expr {
    public:
        IdExpr(std::string id) : id(std::move(id)) {};

        std::string get_id() const {return id;};

        ACCEPT(ExprVis);

    private:
        std::string id;
};

class BinaryExpr : public Expr {
    public:
        BinaryExpr(Tok op, std::unique_ptr<Expr> lhs,
                           std::unique_ptr<Expr> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {};

        Tok get_op() const {return op;};
        Expr& get_lhs() const {return *lhs;};
        Expr& get_rhs() const {return *rhs;};

        ACCEPT(ExprVis);

    private:
        Tok op;
        std::unique_ptr<Expr> lhs, rhs;
};

class VarDecl : public Statement {
    public:
        VarDecl(Tok type, std::string id, std::unique_ptr<Expr> rhs)
            : type(type), id(std::move(id)), rhs(std::move(rhs)) {};

        Tok get_type() {return type;};
        std::string get_id() {return id;};
        Expr& get_rhs() {return *rhs;};

        ACCEPT(StatementVis);

    private:
        Tok type; // TODO: implement a "type" type for more than just POD types
        std::string id;
        std::unique_ptr<Expr> rhs;
};

//---------------------------------------------------------------------------//
//                            Syntax tree output                             //
//---------------------------------------------------------------------------//

class ExprInfoVis : public ExprVis {
    public:
        VISIT(LiteralExpr<double>);
        VISIT(IdExpr);
        VISIT(BinaryExpr);

        std::string get_str() {return str;};
    private:
        std::string str;
};

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
    str = "("s + lhs.str + " [Operator "s
        + std::to_string(static_cast<int>(expr.get_op()))
        + "] "s + rhs.str + ")"s;
}

class SynInfoVis : public StatementVis {
    public:
        VISIT(Expr);
        VISIT(VarDecl);

        std::string get_str() {return str;};
    private:
        std::string str;
};

void SynInfoVis::visit(Expr& expr) {
    ExprInfoVis expr_vis;
    expr.accept(expr_vis);
    str = expr_vis.get_str();
}

void SynInfoVis::visit(VarDecl& expr) {
    ExprInfoVis expr_vis;
    expr.get_rhs().accept(expr_vis);
    str = "double "s + expr.get_id() + " = "s + expr_vis.get_str();
}

//---------------------------------------------------------------------------//
//                                  Parser                                   //
//---------------------------------------------------------------------------//

#include <map>

enum class Assoc {
    LEFT,
    RIGHT
};

static std::map<Tok, std::pair<int, Assoc>> binary_precedence{
    {Tok::EQ, {10, Assoc::RIGHT}},
    {Tok::PLUS, {20, Assoc::LEFT}},
    {Tok::MULT, {30, Assoc::LEFT}}
};

class Parser {
    public:
        Parser(Lexer lex) : lex(std::move(lex)) {};
        std::pair<int, Assoc> get_precedence(Tok tok);
        std::unique_ptr<Statement> parse();
        std::unique_ptr<VarDecl> parse_var_decl();
        std::unique_ptr<Expr> parse_top_expr();
        std::unique_ptr<Expr> parse_expr();
        std::unique_ptr<Expr> parse_expr_rhs(int precedence,
                                             std::unique_ptr<Expr> lhs);
        std::unique_ptr<Expr> parse_primary();
    private:
        Lexer lex;
};

std::pair<int, Assoc> Parser::get_precedence(Tok tok) {
    auto precedence = binary_precedence[tok];

    if(precedence.first == 0)
        return {-1, Assoc::LEFT};

    return precedence;
}

std::unique_ptr<Statement> Parser::parse() {
    if(lex.get_tok() == Tok::END)
        return nullptr;

    Tok cur_tok = lex.get_next_tok();

    switch(cur_tok) {
        case Tok::INVALID:
        case Tok::END:
        case Tok::PLUS:
        case Tok::MULT:
        case Tok::EQ:
            return nullptr;
        case Tok::SEMICOLON:
            return parse();
        case Tok::T_DOUBLE:
            return parse_var_decl();
        case Tok::ID:
        case Tok::L_DOUBLE:
            return parse_top_expr();
    }
}

std::unique_ptr<VarDecl> Parser::parse_var_decl() {
    Tok op = lex.get_tok();
    Tok cur_tok = lex.get_next_tok();
    if(cur_tok != Tok::ID)
        return nullptr;

    auto id = lex.get_id();

    cur_tok = lex.get_next_tok();
    if(cur_tok != Tok::EQ)
        return nullptr;

    lex.get_next_tok();
    auto expr = parse_expr();
    if(!expr)
        return nullptr;

    return std::make_unique<VarDecl>(op, id, std::move(expr));
}

std::unique_ptr<Expr> Parser::parse_top_expr() {
    auto expr = parse_expr();

    if(lex.get_tok() != Tok::SEMICOLON)
        return nullptr;

    lex.get_next_tok();
    return expr;
}

std::unique_ptr<Expr> Parser::parse_expr() {
    return parse_expr_rhs(0, parse_primary());
}

std::unique_ptr<Expr> Parser::parse_expr_rhs(int expr_prec,
                                             std::unique_ptr<Expr> lhs) {
    while(true) {
        Tok op = lex.get_tok();
        int op_prec;
        Assoc lr_ass;
        std::tie(op_prec, lr_ass) = get_precedence(op);

        if(op_prec < expr_prec)
            return lhs;

        lex.get_next_tok();

        auto rhs = parse_primary();
        if(!rhs)
            return nullptr;

        Tok new_op = lex.get_tok();
        int new_op_prec = get_precedence(new_op).first;

        if(new_op_prec > op_prec
                || (new_op_prec == op_prec && lr_ass == Assoc::RIGHT)) {
            rhs = parse_expr_rhs(op_prec, std::move(rhs));
        }

        lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<Expr> Parser::parse_primary() {
    std::unique_ptr<Expr> expr;

    Tok cur_tok = lex.get_tok();
    if(cur_tok == Tok::L_DOUBLE) {
        expr = std::make_unique<LiteralExpr<double>>(lex.get_double());
    } else if(cur_tok == Tok::ID) {
        expr = std::make_unique<IdExpr>(lex.get_id());
    } else {
        return nullptr;
    }

    lex.get_next_tok();
    return expr;
}

//---------------------------------------------------------------------------//
//                               Main function                               //
//---------------------------------------------------------------------------//

int main() {
    Parser par{Lexer{}};
    while(std::unique_ptr<Statement> ast = par.parse()) {
        SynInfoVis vis;
        ast->accept(vis);
        printf("%s;\n", vis.get_str().c_str());
    }

    return 0;
}
