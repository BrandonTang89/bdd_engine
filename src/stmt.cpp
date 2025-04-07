#include <variant>

#include "token.h"

struct bin_expr {
    expr left;
    expr right;
    Token op;
};
struct unary_expr {
    expr operand;
    Token op;
};

struct literal {
    Token value;
};

struct identifier {
    Token name;
};

using expr = std::variant<bin_expr, unary_expr, literal, identifier>;

struct expr_stmt {
    expr expression;
};

struct display_stmt {
    expr expression;
};

struct decl_stmt {
    std::vector<Token> identifiers;
};

struct assign_stmt {
    expr target;
    expr value;
};

using stmt = std::variant<
    expr_stmt,
    display_stmt,
    decl_stmt,
    assign_stmt,
    >;