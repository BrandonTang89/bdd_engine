#include <cassert>
#include <ranges>
#include <variant>

#include "engine_exceptions.h"
#include "walker.h"

std::shared_ptr<expr> Walker::construct_expr(const id_type id) {
    if (id == 0) {
        return false_expr;
    };
    if (id == 1) {
        return true_expr;
    }

    if (!id_to_iter.contains(id)) {
        throw ExecutionException("ID not found: " + std::to_string(id),
                                 "Walker::construct_expr");
    }
    const auto& node = id_to_iter[id]->first;
    assert(node.type == Bdd_Node::Bdd_type::INTERNAL);

    // (x -> high) & (!x -> low) => (!x | high) & (x | low)
    auto x =
        std::make_shared<expr>(identifier{token{token::Type::IDENTIFIER, node.var}});
    auto not_x =
        std::make_shared<expr>(unary_expr{x, token{token::Type::BANG, "!"}});

    auto x_implies_high = std::make_shared<expr>(
        bin_expr{std::move(not_x), construct_expr(node.high),
                 token{token::Type::LOR, "|"}});
    auto not_x_implies_low = std::make_shared<expr>(bin_expr{
        std::move(x), construct_expr(node.low), token{token::Type::LOR, "|"}});

    // Combine the two implications with AND
    return std::make_shared<expr>(bin_expr{std::move(x_implies_high),
                                           std::move(not_x_implies_low),
                                           token{token::Type::LAND, "&"}});
}

std::shared_ptr<expr> Walker::substitute_expr(const expr& x,
                                              const substitution_map& sub_map) {
    std::shared_ptr<expr> ret_expr{};
    std::visit(
        [&ret_expr, &sub_map, this]<typename T0>(const T0& exp) {
            using T = std::remove_cvref_t<T0>;
            if constexpr (std::is_same_v<T, bin_expr>) {
                auto left_expr = substitute_expr(*exp.left, sub_map);
                auto right_expr = substitute_expr(*exp.right, sub_map);
                return ret_expr = std::make_shared<expr>(
                           bin_expr{std::move(left_expr), std::move(right_expr),
                                    exp.op});

            } else if constexpr (std::is_same_v<T, unary_expr>) {
                auto operand_expr = substitute_expr(*exp.operand, sub_map);
                return ret_expr = std::make_shared<expr>(
                           unary_expr{std::move(operand_expr), exp.op});

            } else if constexpr (std::is_same_v<T, literal>) {
                if (exp.value.type == token::Type::ID) {
                    throw std::runtime_error(
                        "ID literals are not supported in substitution: " +
                        exp.value.lexeme);
                }
                if (exp.value.type == token::Type::TRUE) {
                    return ret_expr = true_expr;
                }
                if (exp.value.type == token::Type::FALSE) {
                    return ret_expr = false_expr;
                }
                throw std::runtime_error("Unsupported literal type");
            } else if constexpr (std::is_same_v<T, identifier>) {
                // Handle identifier
                if (globals.contains(exp.name.lexeme)) {
                    if (std::holds_alternative<Bvar_ptype>(
                            globals[exp.name.lexeme])) {
                        if (sub_map.contains(exp.name.lexeme)) {
                            // Apply substitution
                            return ret_expr = std::shared_ptr<expr>(
                                       sub_map.at(exp.name.lexeme));
                        }

                        const auto& bvar =
                            std::get<Bvar_ptype>(globals[exp.name.lexeme]);
                        const Bdd_Node bdd_node{Bdd_Node::Bdd_type::INTERNAL,
                                                bvar.name, 1, 0};
                        return ret_expr = std::make_shared<expr>(identifier{
                                   token{token::Type::IDENTIFIER, bvar.name}});
                    }
                }
                throw std::runtime_error(
                    "Only BDD variables are supported in substitution");

            } else {
                throw std::runtime_error("Unsupported expression type");
                return ret_expr = false_expr;
            }
        },
        x);

    return ret_expr;
}