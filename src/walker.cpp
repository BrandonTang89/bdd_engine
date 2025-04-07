
#include "walker.h"

#include <iostream>
#include <variant>

#include "absl/log/log.h"

void Walker::walk_decl_stmt(const decl_stmt& statement) {
    // Handle declaration statement
    for (const auto& identifier : statement.identifiers) {
        if (globals.find(identifier.lexeme) == globals.end()) {
            globals[identifier.lexeme] = Bvar_ptype{identifier.lexeme};
            std::cout << "Declared Symbolic Variable: " << identifier.lexeme << std::endl;
        } else {
            if (std::holds_alternative<Bvar_ptype>(globals[identifier.lexeme])) {
                std::cout << "Variable already declared: " << identifier.lexeme << std::endl;
            } else {
                std::cout << "Variable name conflict, ignoring: " << identifier.lexeme << std::endl;
            }
        }
    }
}

void Walker::walk(const stmt& statement) {
    // Walk the AST and evaluate the statement
    std::visit([this](const auto& stmt) {
        using T = std::decay_t<decltype(stmt)>;
        if constexpr (std::is_same_v<T, expr_stmt>) {
            // Handle expression statement
            LOG(INFO) << "Executing Expression Statement...";

        } else if constexpr (std::is_same_v<T, display_stmt>) {
            // Handle display statement
            LOG(INFO) << "Executing Display Statement...";
        } else if constexpr (std::is_same_v<T, decl_stmt>) {
            // Handle declaration statement
            LOG(INFO) << "Executing Declaration Statement...";
            walk_decl_stmt(stmt);
        } else if constexpr (std::is_same_v<T, assign_stmt>) {
            LOG(INFO) << "Executing Assignment Statement...";
        }
    },
               statement);
}

id_type Walker::construct_bdd(const expr& x) {
    id_type ret_id{};
    std::visit(
        [&ret_id, this](const auto& expr) {
            using T = std::decay_t<decltype(expr)>;
            if constexpr (std::is_same_v<T, bin_expr>) {
                id_type left_bdd = construct_bdd(*expr.left);
                id_type right_bdd = construct_bdd(*expr.right);
                id_type combined_bdd;
                if (expr.op.type == Token::Type::LAND) {
                    combined_bdd = and_bdd(left_bdd, right_bdd);
                } else if (expr.op.type == Token::Type::LOR) {
                    // Handle logical OR
                    combined_bdd = or_bdd(left_bdd, right_bdd);
                } else {
                    throw std::runtime_error("Unsupported binary operator");
                }
                return ret_id = combined_bdd;
            } else if constexpr (std::is_same_v<T, unary_expr>) {
                // Handle unary expression
                id_type operand_bdd = construct_bdd(*expr.operand);
                if (expr.op.type == Token::Type::BANG) {
                    return ret_id = negate_bdd(operand_bdd);
                } else {
                    throw std::runtime_error("Unsupported unary operator");
                }

            } else if constexpr (std::is_same_v<T, literal>) {
                // Handle literal
                if (expr.value.type == Token::Type::TRUE) {
                    return ret_id = static_cast<id_type>(1);
                } else if (expr.value.type == Token::Type::FALSE) {
                    return ret_id = static_cast<id_type>(0);
                } else {
                    throw std::runtime_error("Unsupported literal type");
                }
            } else if constexpr (std::is_same_v<T, identifier>) {
                // Handle identifier
                if (globals.find(expr.name.lexeme) != globals.end()) {
                    if (std::holds_alternative<Bvar_ptype>(globals[expr.name.lexeme])) {
                        // Handle BDD variable
                        const auto& bvar = std::get<Bvar_ptype>(globals[expr.name.lexeme]);
                        Bdd_Node bdd_node{Bdd_Node::Bdd_type::INTERNAL, bvar.name, 1, 0};  // if x then high else low

                        auto it = to_human_map.find(bdd_node);
                        if (it != to_human_map.end())
                            return ret_id = it->second;
                        else {
                            assert("IMPLEMENT CREATE BDD");
                        }
                        return ret_id = static_cast<id_type>(1);

                    } else {
                        throw std::runtime_error("Identifier is not a BDD variable");
                    }
                } else {
                    throw std::runtime_error("Identifier not found in globals");
                }
            } else {
                return static_cast<id_type>(1);
            }
        },
        x);

    return ret_id;
}

id_type Walker::and_bdd(id_type a, id_type b) {
    // Implement AND operation on BDDs
    return static_cast<id_type>(1);
}

id_type Walker::or_bdd(id_type a, id_type b) {
    // Implement OR operation on BDDs
    return static_cast<id_type>(1);
}

id_type Walker::negate_bdd(id_type a) {
    // Implement NOT operation on BDDs
    return static_cast<id_type>(1);
}