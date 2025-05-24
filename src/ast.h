#pragma once
#include <memory>
#include <variant>

#include "lexer.h"

// Expressions
struct bin_expr;
struct unary_expr;
struct literal;
struct identifier;
struct quantifier_expr;

using expr =
    std::variant<bin_expr, quantifier_expr, unary_expr, literal, identifier>;
struct bin_expr {
    std::shared_ptr<expr> left;
    std::shared_ptr<expr> right;
    token op;
};

struct quantifier_expr {
    token quantifier;
    std::vector<token> bound_vars;
    std::shared_ptr<expr> body;
};

struct unary_expr {
    std::shared_ptr<expr> operand;
    token op;
};

struct literal {
    token value;
};

struct identifier {
    token name;
};

// Statements
struct expr_stmt;
struct func_call_stmt;
struct decl_stmt;
struct assign_stmt;
using stmt = std::variant<expr_stmt, func_call_stmt, decl_stmt, assign_stmt>;

struct expr_stmt {
    std::shared_ptr<expr> expression;
};

struct func_call_stmt {
    token func_name;
    std::vector<std::shared_ptr<expr>> arguments;
};

struct decl_stmt {
    std::vector<token> identifiers;
};

struct assign_stmt {
    std::shared_ptr<identifier> target;
    std::shared_ptr<expr> value;
};

std::string stmt_repr(const stmt& statement);
std::string expr_repr(const expr& expression);
[[maybe_unused]] std::shared_ptr<expr> clone_expr(
    const std::shared_ptr<expr>& expression);

static_assert(std::is_move_constructible_v<stmt>, "stmt must be movable");
static_assert(std::is_move_constructible_v<expr>, "expr must be movable");
