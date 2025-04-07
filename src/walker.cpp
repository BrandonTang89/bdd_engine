
#include "walker.h"

#include <iostream>
#include <variant>

#include "absl/log/log.h"

void Walker::walk_decl_stmt(const decl_stmt& statement) {
    // Handle declaration statement
    for (const auto& identifier : statement.identifiers) {
        if (globals.find(identifier.lexeme) == globals.end()) {
            auto new_var = Bvar_ptype{identifier.lexeme};
            globals[identifier.lexeme] = new_var;
            bdd_ordering_map[identifier.lexeme] = bdd_ordering.size();
            bdd_ordering.push_back(identifier.lexeme);
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

            try {
                id_type bdd_id = construct_bdd(*stmt.expression);
                std::cout << "BDD ID: " << bdd_id << std::endl;
                std::cout << "BDD Representation: " << bdd_repr(bdd_id) << std::endl;
            } catch (const std::exception& e) {
                LOG(ERROR) << "Error constructing BDD: " << e.what();
            }

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
                        return ret_id = get_id(bdd_node);
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

id_type Walker::get_id(const Bdd_Node& node) {
    if (node.type == Bdd_Node::Bdd_type::FALSE) return 0;
    if (node.type == Bdd_Node::Bdd_type::TRUE) return 1;
    auto it = to_human_map.find(node);

    if (it != to_human_map.end()) {
        return it->second;
    } else {
        auto nit = bdd_set.insert(node).first;
        to_human_map[node] = (counter++);
        from_human_map[counter - 1] = nit;
        return counter - 1;
    }
}

id_type Walker::rec_apply_and(id_type a, id_type b) {
    const Bdd_Node& node_a = *from_human_map[a];
    const Bdd_Node& node_b = *from_human_map[b];

    // Base Cases
    if (node_a == node_b) return a;
    if (node_a.type == Bdd_Node::Bdd_type::FALSE || node_b.type == Bdd_Node::Bdd_type::FALSE) {
        return static_cast<id_type>(0);
    }
    if (node_a.type == Bdd_Node::Bdd_type::TRUE) return b;
    if (node_b.type == Bdd_Node::Bdd_type::TRUE) return a;
    auto mit = and_memo.find(std::make_tuple(a, b));
    if (mit != and_memo.end()) {
        return mit->second;
    }

    // Recursive Cases
    const std::string& a_var = node_a.var;
    const std::string& b_var = node_b.var;

    id_type nhigh;
    id_type nlow;

    bool pivot_on_a = bdd_ordering_map[a_var] <= bdd_ordering_map[b_var];
    if (a_var == b_var) {
        nhigh = rec_apply_and(node_a.high, node_b.high);
        nlow = rec_apply_and(node_a.low, node_b.low);
    } else if (pivot_on_a) {
        nhigh = rec_apply_and(node_a.high, b);
        nlow = rec_apply_and(node_a.low, b);
    } else {
        nhigh = rec_apply_and(a, node_b.high);
        nlow = rec_apply_and(a, node_b.low);
    }

    if (nhigh == nlow) {
        return and_memo[{a, b}] = nhigh;
    }
    Bdd_Node new_node{Bdd_Node::Bdd_type::INTERNAL, (pivot_on_a ? a_var : b_var), nhigh, nlow};
    return and_memo[{a, b}] = get_id(new_node);
}

id_type Walker::and_bdd(id_type a, id_type b) {
    // Implement AND operation on BDDs
    and_memo.clear();
    return rec_apply_and(a, b);
}

id_type Walker::rec_apply_or(id_type a, id_type b) {
    const Bdd_Node& node_a = *from_human_map[a];
    const Bdd_Node& node_b = *from_human_map[b];

    // Base Cases
    if (node_a == node_b) return a;
    if (node_a.type == Bdd_Node::Bdd_type::TRUE || node_b.type == Bdd_Node::Bdd_type::TRUE) {
        return static_cast<id_type>(1);
    }
    if (node_a.type == Bdd_Node::Bdd_type::FALSE) return b;
    if (node_b.type == Bdd_Node::Bdd_type::FALSE) return a;
    auto mit = or_memo.find(std::make_tuple(a, b));
    if (mit != or_memo.end()) {
        return mit->second;
    }

    // Recursive Cases
    const std::string& a_var = node_a.var;
    const std::string& b_var = node_b.var;

    id_type nhigh;
    id_type nlow;

    bool pivot_on_a = bdd_ordering_map[a_var] <= bdd_ordering_map[b_var];
    if (a_var == b_var) {
        nhigh = rec_apply_or(node_a.high, node_b.high);
        nlow = rec_apply_or(node_a.low, node_b.low);
    } else if (pivot_on_a) {
        nhigh = rec_apply_or(node_a.high, b);
        nlow = rec_apply_or(node_a.low, b);
    } else {
        nhigh = rec_apply_or(a, node_b.high);
        nlow = rec_apply_or(a, node_b.low);
    }
    if (nhigh == nlow) {
        return or_memo[{a, b}] = nhigh;
    }
    Bdd_Node new_node{Bdd_Node::Bdd_type::INTERNAL, (pivot_on_a ? a_var : b_var), nhigh, nlow};
    return or_memo[{a, b}] = get_id(new_node);
}

id_type Walker::or_bdd(id_type a, id_type b) {
    // Implement OR operation on BDDs
    or_memo.clear();
    return rec_apply_or(a, b);
}

id_type Walker::rec_apply_not(id_type a) {
    const Bdd_Node& node = *from_human_map[a];

    // Base Cases
    if (node.type == Bdd_Node::Bdd_type::FALSE) return 1;
    if (node.type == Bdd_Node::Bdd_type::TRUE) return 0;
    auto mit = not_memo.find(a);
    if (mit != not_memo.end()) {
        return mit->second;
    }

    // Recursive Cases
    id_type left = rec_apply_not(node.high);
    id_type right = rec_apply_not(node.low);

    assert(left != right);  // sanity

    Bdd_Node new_node{Bdd_Node::Bdd_type::INTERNAL, node.var, left, right};
    return not_memo[a] = get_id(new_node);
}

id_type Walker::negate_bdd(id_type a) {
    // Implement NOT operation on BDDs
    not_memo.clear();
    return rec_apply_not(a);
}

std::string Walker::bdd_repr(id_type id) {
    const Bdd_Node& node = *from_human_map[id];
    if (node.type == Bdd_Node::Bdd_type::INTERNAL) {
        return (node.var + " ? (" + bdd_repr(node.high) + ") : (" + bdd_repr(node.low)) + ")";
    } else if (node.type == Bdd_Node::Bdd_type::TRUE) {
        return "TRUE";
    } else if (node.type == Bdd_Node::Bdd_type::FALSE) {
        return "FALSE";
    }
    throw std::runtime_error("Unknown BDD type");
}