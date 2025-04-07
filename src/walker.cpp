
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
            if (globals[identifier.lexeme].index() == static_cast<size_t>(Ptype_type::BVAR)) {
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