
#include "walker.h"

#include <iostream>
#include <variant>

#include "absl/log/log.h"

Walker::Walker() {
    // Initialize the Walker
    Bdd_Node false_node{Bdd_Node::Bdd_type::FALSE, "false", 0, 0};
    Bdd_Node true_node{Bdd_Node::Bdd_type::TRUE, "true", 1, 1};
    node_to_id[false_node] = 0;
    node_to_id[true_node] = 1;
    id_to_iter[0] = node_to_id.find(false_node);
    id_to_iter[1] = node_to_id.find(true_node);
    is_sat_memo[0] = false;
    is_sat_memo[1] = true;
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

void Walker::walk_func_call_stmt(const func_call_stmt& statement) {
    switch (statement.func_name.type) {
        case Token::Type::TREE_DISPLAY: {
            LOG(INFO) << "Tree Display Function Called";
            if (statement.arguments.size() != 1) {
                LOG(ERROR) << "Invalid number of arguments for tree display";
                return;
            }
            try {
                id_type bdd_id = construct_bdd(*statement.arguments[0]);
                std::cout << "BDD ID: " << bdd_id << std::endl;
                std::cout << "BDD Representation: " << bdd_repr(bdd_id) << std::endl;
            } catch (const std::exception& e) {
                LOG(ERROR) << "Error constructing BDD: " << e.what();
            }
            break;
        }
        case Token::Type::GRAPH_DISPLAY: {
            if (statement.arguments.size() != 1) {
                LOG(ERROR) << "Invalid number of arguments for graph display";
                return;
            }
            id_type bdd_id = construct_bdd(*statement.arguments[0]);
            std::string gviz_rep = bdd_gviz_repr(bdd_id);
            std::cout << gviz_rep << std::endl;
            break;
        }
        case Token::Type::IS_SAT: {
            if (statement.arguments.size() != 1) {
                LOG(ERROR) << "Invalid number of arguments for is_sat";
                return;
            }
            LOG(INFO) << "Is SAT Function Called" << std::endl;
            bool sat = is_sat(construct_bdd(*statement.arguments[0]));

            if (sat) {
                std::cout << "satisfiable" << std::endl;
            } else {
                std::cout << "unsatisfiable" << std::endl;
            }
            break;
        }
        default:
            LOG(ERROR) << "Unknown function call";
    }
}

id_type Walker::walk_expr_stmt(const expr_stmt& statement) {
    // Handle expression statement
    try {
        id_type bdd_id = construct_bdd(*statement.expression);
        std::cout << "BDD ID: " << bdd_id << std::endl;
        return bdd_id;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Error constructing BDD: " << e.what();
        return -1;
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
        } else if constexpr (std::is_same_v<T, func_call_stmt>) {
            // Handle display statement
            LOG(INFO) << "Executing Display Statement...";
            walk_func_call_stmt(stmt);
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
