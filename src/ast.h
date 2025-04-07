#pragma once
#include <memory>
#include <variant>

#include "token.h"

// Expressions
struct bin_expr;
struct unary_expr;
struct literal;
struct identifier;

using expr = std::variant<std::monostate, bin_expr, unary_expr, literal, identifier>;
struct bin_expr {
    std::unique_ptr<expr> left;
    std::unique_ptr<expr> right;
    Token op;
};

struct unary_expr {
    std::unique_ptr<expr> operand;
    Token op;
};

struct literal {
    Token value;
};

struct identifier {
    Token name;
};

// Statements
struct expr_stmt;
struct display_stmt;
struct decl_stmt;
struct assign_stmt;
using stmt = std::variant<
    std::monostate,
    expr_stmt,
    display_stmt,
    decl_stmt,
    assign_stmt>;
struct expr_stmt {
    std::unique_ptr<expr> expression;
};

struct display_stmt {
    std::unique_ptr<expr> expression;
};

struct decl_stmt {
    std::vector<Token> identifiers;
};

struct assign_stmt {
    std::unique_ptr<identifier> target;
    std::unique_ptr<expr> value;
};


std::string stmt_repr(const stmt& statement);