
#include "walker.h"

#include <iostream>
#include <variant>

#include "absl/log/log.h"

Walker::Walker() {
    // Initialize the Walker
    Bdd_Node false_node{Bdd_Node::Bdd_type::FALSE, "false", 0, 0};
    Bdd_Node true_node{Bdd_Node::Bdd_type::TRUE, "true", 1, 1};
    iter_to_false = bdd_set.insert(false_node).first;
    iter_to_true = bdd_set.insert(true_node).first;
    from_human_map[0] = iter_to_false;
    from_human_map[1] = iter_to_true;
    to_human_map[false_node] = 0;
    to_human_map[true_node] = 1;
    counter = 2;
}

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

void Walker::walk_assign_stmt(const assign_stmt& statement) {
    // Handle assignment statement
    if (globals.find(statement.target->name.lexeme) != globals.end() && std::holds_alternative<Bvar_ptype>(globals[statement.target->name.lexeme])) {
        std::cout << "Variable name conflict, ignoring assignment of: " << statement.target->name.lexeme << std::endl;

    } else {
        id_type bdd_id = construct_bdd(*statement.value);
        globals[statement.target->name.lexeme] = Bdd_ptype{statement.target->name.lexeme, bdd_id};
        std::cout << "Assigned to " << statement.target->name.lexeme << " with BDD ID: " << bdd_id << std::endl;
    }
}

void Walker::walk_expr_stmt(const expr_stmt& statement) {
    // Handle expression statement
    try {
        id_type bdd_id = construct_bdd(*statement.expression);
        std::cout << "BDD ID: " << bdd_id << std::endl;
        std::cout << "BDD Representation: " << bdd_repr(bdd_id) << std::endl;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Error constructing BDD: " << e.what();
    }
}

void Walker::walk(const stmt& statement) {
    // Walk the AST and evaluate the statement
    std::visit([this](const auto& stmt) {
        using T = std::decay_t<decltype(stmt)>;
        if constexpr (std::is_same_v<T, expr_stmt>) {
            // Handle expression statement
            LOG(INFO) << "Executing Expression Statement...";
            walk_expr_stmt(stmt);
        } else if constexpr (std::is_same_v<T, display_stmt>) {
            // Handle display statement
            LOG(INFO) << "Executing Display Statement...";
        } else if constexpr (std::is_same_v<T, decl_stmt>) {
            // Handle declaration statement
            LOG(INFO) << "Executing Declaration Statement...";
            walk_decl_stmt(stmt);
        } else if constexpr (std::is_same_v<T, assign_stmt>) {
            LOG(INFO) << "Executing Assignment Statement...";
            walk_assign_stmt(stmt);
        }
    },
               statement);
}

