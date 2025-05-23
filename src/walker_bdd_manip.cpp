#include <cassert>
#include <ranges>
#include <variant>

#include "engine_exceptions.h"
#include "walker.h"

void Walker::clear_memos() {  // for later: Implement garbage collection
    binop_memo.clear();
    not_memo.clear();
    is_sat_memo.clear();
}

id_type Walker::construct_bdd(const expr& x) {
    id_type ret_id{};
    std::visit(
        [&ret_id, this]<typename T0>(const T0& expression) {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, bin_expr>) {
                const id_type left_bdd = construct_bdd(*expression.left);
                const id_type right_bdd = construct_bdd(*expression.right);
                id_type combined_bdd = 0;

                if (expression.op.type == Token::Type::LAND) {
                    combined_bdd = rec_apply_and(left_bdd, right_bdd);
                } else if (expression.op.type == Token::Type::LOR) {
                    combined_bdd = rec_apply_or(left_bdd, right_bdd);
                } else {
                    throw std::runtime_error("Unsupported binary operator" +
                                             expression.op.lexeme);
                }
                return ret_id = combined_bdd;
            } else if constexpr (std::is_same_v<T, quantifier_expr>) {
                // Handle quantifier expression
                id_type body_bdd = construct_bdd(*expression.body);

                // Simple Cases
                if (body_bdd == 0 || body_bdd == 1) {
                    return ret_id = body_bdd;
                }
                const Bdd_Node& body_bdd_node = id_to_iter[body_bdd]->first;
                assert(body_bdd_node.type == Bdd_Node::Bdd_type::INTERNAL);

                // Ensure that the bound variables at least as high in
                // bdd_ordering as the top node of the body, Sort the bound
                // variables in the order of their appearance in bdd_ordering
                auto vw =
                    expression.bound_vars |
                    std::views::transform(
                        [](const auto& id) { return id.lexeme; }) |
                    std::views::filter([this, &body_bdd_node](const auto& id) {
                        return bdd_ordering_map[id] >=
                               bdd_ordering_map[body_bdd_node.var];
                    });
                std::vector<std::string> bound_var_names(vw.begin(), vw.end());
                std::ranges::sort(
                    bound_var_names, [this](const auto& a, const auto& b) {
                        return bdd_ordering_map[a] < bdd_ordering_map[b];
                    });

                // Clear the quantifier memo but binary operation memos can be
                // kept
                quantifier_memo.clear();

                // Apply the quantifier
                if (expression.quantifier.type == Token::Type::EXISTS) {
                    return ret_id = rec_apply_quant(
                               body_bdd, bound_var_names,
                               [this](const id_type a, const id_type b) {
                                   return rec_apply_or(a, b);
                               });
                } else if (expression.quantifier.type == Token::Type::FORALL) {
                    return ret_id = rec_apply_quant(
                               body_bdd, bound_var_names,
                               [this](const id_type a, const id_type b) {
                                   return rec_apply_and(a, b);
                               });

                } else {
                    throw std::runtime_error("Unsupported quantifier type");
                }
            } else if constexpr (std::is_same_v<T, unary_expr>) {
                // Handle unary expression
                const id_type operand_bdd = construct_bdd(*expression.operand);
                if (expression.op.type == Token::Type::BANG) {
                    return ret_id = rec_apply_not(operand_bdd);
                } else {
                    throw std::runtime_error("Unsupported unary operator");
                }

            } else if constexpr (std::is_same_v<T, literal>) {
                // Handle literal
                if (expression.value.type == Token::Type::ID) {
                    if (id_to_iter.contains(*expression.value.token_value)) {
                        return ret_id = *expression.value.token_value;
                    } else {
                        throw ExecutionException(
                            "ID not found: " + expression.value.lexeme,
                            "Walker::construct_bdd");
                    }
                } else if (expression.value.type == Token::Type::TRUE) {
                    return ret_id = static_cast<id_type>(1);
                } else if (expression.value.type == Token::Type::FALSE) {
                    return ret_id = static_cast<id_type>(0);
                } else {
                    throw std::runtime_error("Unsupported literal type");
                }
            } else if constexpr (std::is_same_v<T, identifier>) {
                // Handle identifier
                if (globals.contains(expression.name.lexeme)) {
                    if (std::holds_alternative<Bvar_ptype>(
                            globals[expression.name.lexeme])) {
                        // Handle BDD variable
                        const auto& bvar = std::get<Bvar_ptype>(
                            globals[expression.name.lexeme]);
                        const Bdd_Node bdd_node{Bdd_Node::Bdd_type::INTERNAL,
                                                bvar.name, 1,
                                                0};  // if x then high else low
                        return ret_id = get_id(bdd_node);
                    } else {
                        return ret_id = std::get<Bdd_ptype>(
                                            globals[expression.name.lexeme])
                                            .id;
                    }
                } else {
                    throw ExecutionException(
                        "Variable not found: " + expression.name.lexeme,
                        "Walker::construct_bdd");
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

    if (const auto it = node_to_id.find(node); it != node_to_id.end()) {
        return it->second;
    } else {
        node_to_id[node] = counter++;
        id_to_iter[counter - 1] = node_to_id.find(node);
        return counter - 1;
    }
}

template <typename Comb_Fn_Type>
id_type Walker::rec_apply_quant(id_type a, std::span<std::string> bound_vars,
                                Comb_Fn_Type comb_fn) {
    // precondition: the first element of bound_vars >= a.var in bdd_ordering
    if (bound_vars.empty()) return a;
    const Bdd_Node& node = id_to_iter[a]->first;
    const std::tuple memo_key = {a, bound_vars.size()};
    if (quantifier_memo.contains(memo_key)) {
        return quantifier_memo[memo_key];
    }

    // Base Cases
    if (node.type == Bdd_Node::Bdd_type::FALSE) return 0;
    if (node.type == Bdd_Node::Bdd_type::TRUE) return 1;
    size_t var_index = 0;
    while (var_index < bound_vars.size() &&
           bdd_ordering_map[bound_vars[var_index]] <
               bdd_ordering_map[node.var]) {
        ++var_index;
    }

    // Recursive Cases
    if (node.var == bound_vars[0]) {  // we quantify out this variable
        id_type high =
            rec_apply_quant(node.high, bound_vars.subspan(1), comb_fn);
        id_type low = rec_apply_quant(node.low, bound_vars.subspan(1), comb_fn);
        if (high == low) return high;
        return quantifier_memo[memo_key] = comb_fn(high, low);
    } else {
        const id_type high = rec_apply_quant(node.high, bound_vars, comb_fn);
        const id_type low = rec_apply_quant(node.low, bound_vars, comb_fn);
        if (high == low) return high;
        return quantifier_memo[memo_key] = get_id(
                   Bdd_Node{Bdd_Node::Bdd_type::INTERNAL, node.var, high, low});
    }
}

id_type Walker::rec_apply_and(id_type a, id_type b) {
    const Bdd_Node& node_a = id_to_iter[a]->first;
    const Bdd_Node& node_b = id_to_iter[b]->first;

    // Base Cases
    if (node_a == node_b) return a;
    if (node_a.type == Bdd_Node::Bdd_type::FALSE ||
        node_b.type == Bdd_Node::Bdd_type::FALSE) {
        return 0;
    }
    if (node_a.type == Bdd_Node::Bdd_type::TRUE) return b;
    if (node_b.type == Bdd_Node::Bdd_type::TRUE) return a;

    // Use the memo with the AND operation type
    if (const auto mit = binop_memo.find(std::make_tuple(a, b, BinOpType::AND));
        mit != binop_memo.end()) {
        return mit->second;
    }

    // Recursive Cases
    const std::string& a_var = node_a.var;
    const std::string& b_var = node_b.var;

    id_type nhigh = 0;
    id_type nlow = 0;

    const bool pivot_on_a = bdd_ordering_map[a_var] <= bdd_ordering_map[b_var];
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
        return binop_memo[{a, b, BinOpType::AND}] = nhigh;
    }
    const Bdd_Node new_node{Bdd_Node::Bdd_type::INTERNAL,
                            (pivot_on_a ? a_var : b_var), nhigh, nlow};
    return binop_memo[{a, b, BinOpType::AND}] = get_id(new_node);
}

id_type Walker::rec_apply_or(id_type a, id_type b) {
    const Bdd_Node& node_a = id_to_iter[a]->first;
    const Bdd_Node& node_b = id_to_iter[b]->first;

    // Base Cases
    if (node_a == node_b) return a;
    if (node_a.type == Bdd_Node::Bdd_type::TRUE ||
        node_b.type == Bdd_Node::Bdd_type::TRUE) {
        return 1;
    }
    if (node_a.type == Bdd_Node::Bdd_type::FALSE) return b;
    if (node_b.type == Bdd_Node::Bdd_type::FALSE) return a;

    // Use the memo with the OR operation type
    if (const auto mit = binop_memo.find(std::make_tuple(a, b, BinOpType::OR));
        mit != binop_memo.end()) {
        return mit->second;
    }

    // Recursive Cases
    const std::string& a_var = node_a.var;
    const std::string& b_var = node_b.var;

    id_type nhigh = 0;
    id_type nlow = 0;

    const bool pivot_on_a = bdd_ordering_map[a_var] <= bdd_ordering_map[b_var];
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
        return binop_memo[{a, b, BinOpType::OR}] = nhigh;
    }
    const Bdd_Node new_node{Bdd_Node::Bdd_type::INTERNAL,
                            (pivot_on_a ? a_var : b_var), nhigh, nlow};
    return binop_memo[{a, b, BinOpType::OR}] = get_id(new_node);
}

id_type Walker::rec_apply_not(const id_type a) {
    const Bdd_Node& node = id_to_iter[a]->first;

    // Base Cases
    if (node.type == Bdd_Node::Bdd_type::FALSE) return 1;
    if (node.type == Bdd_Node::Bdd_type::TRUE) return 0;
    if (const auto mit = not_memo.find(a); mit != not_memo.end()) {
        return mit->second;
    }

    // Recursive Cases
    const id_type left = rec_apply_not(node.high);
    const id_type right = rec_apply_not(node.low);

    const Bdd_Node new_node{Bdd_Node::Bdd_type::INTERNAL, node.var, left,
                            right};
    return not_memo[a] = get_id(new_node);
}
