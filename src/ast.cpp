#include "ast.h"

std::string expr_repr(const expr& expression) {
    return std::visit(
        [](const auto& e) -> std::string {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, bin_expr>) {
                return "BinExpr(" + expr_repr(*e.left) + ", " + e.op.lexeme + ", " +
                       expr_repr(*e.right) + ")";

            } else if constexpr (std::is_same_v<T, quantifier_expr>) {
                std::string result = "QuantifierExpr(" + e.quantifier.lexeme + " (";
                for (const auto& id : e.bound_vars) {
                    result += id.lexeme + ", ";
                }
                result.pop_back();  // Remove last space
                result.pop_back();  // Remove last comma
                result += "), ";
                result += expr_repr(*e.body) + ")";
                return result;
            } else if constexpr (std::is_same_v<T, unary_expr>) {
                return "UnaExpr(" + e.op.lexeme + ", " + expr_repr(*e.operand) + ")";
            } else if constexpr (std::is_same_v<T, literal>) {
                return "Literal(" + e.value.lexeme + ")";
            } else if constexpr (std::is_same_v<T, identifier>) {
                return "Identifier(" + e.name.lexeme + ")";
            } else {
                return "Unknown Expression Type";
            }
        },
        expression);
}

std::string stmt_repr(const stmt& statement) {
    return std::visit(
        [](const auto& s) -> std::string {
            using T = std::decay_t<decltype(s)>;
            if constexpr (std::is_same_v<T, expr_stmt>) {
                return "Expr_Stmt(" + expr_repr(*(s.expression)) + ")";
            } else if constexpr (std::is_same_v<T, func_call_stmt>) {
                std::string result = "Func_Call_Stmt(" + s.func_name.lexeme + "(";
                for (const auto& arg : s.arguments) {
                    result += expr_repr(*(arg)) + ", ";
                }
                result.pop_back();  // Remove last space
                result.pop_back();  // Remove last comma
                result += "))";
                return result;
            } else if constexpr (std::is_same_v<T, decl_stmt>) {
                std::string result = "Decl_Stmt(";
                for (const auto& id : s.identifiers) {
                    result += id.lexeme + ", ";
                }
                result.pop_back();  // Remove last space
                result.pop_back();  // Remove last comma
                result += ")";
                return result;
            } else if constexpr (std::is_same_v<T, assign_stmt>) {
                std::string result = "Assign_Stmt(";
                result += "Target: " + expr_repr(*(s.target)) + ", ";
                result += "Value: " + expr_repr(*(s.value)) + ")";
                return result;
            } else {
                return "Unknown Statement Type";
            }
        },
        statement);
}