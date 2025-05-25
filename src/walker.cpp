
#include "walker.h"

#include <fstream>

#include "absl/log/log.h"
#include "ast.h"
#include "colours.h"
#include "config.h"
#include "engine_exceptions.h"
#include "parser.h"

Walker::Walker() : counter(2) {
    // Initialise the Walker
    const Bdd_Node false_node{Bdd_Node::Bdd_type::FALSE, "false", 0, 0};
    const Bdd_Node true_node{Bdd_Node::Bdd_type::TRUE, "true", 1, 1};
    node_to_id[false_node] = 0;
    node_to_id[true_node] = 1;
    id_to_iter[0] = node_to_id.find(false_node);
    id_to_iter[1] = node_to_id.find(true_node);
    is_sat_memo[0] = false;
    is_sat_memo[1] = true;
}

std::string Walker::get_output() {
    std::string output = out.str();
    out.str("");  // Clear the output stream
    return output;
}

void Walker::walk_statements(const std::span<stmt>& statements) {
    for (const auto& statement : statements) {
        try {
            if constexpr (print_ast) LOG(WARNING) << stmt_repr(statement);
            walk_raw(statement);
        } catch (const ExecutionException& e) {
            if constexpr (use_colours) set_colour(out, Colour::RED);
            out << e.what() << '\n';
            if constexpr (use_colours) set_colour(out);
            return;
        } catch (const std::exception& e) {
            out << "Unhandled Execution Error: " << e.what() << '\n';
            return;
        }
    }
}

void Walker::walk_single(const stmt& statement) {
    if constexpr (print_ast) LOG(WARNING) << stmt_repr(statement);
    try {
        walk_raw(statement);
    } catch (const ExecutionException& e) {
        if constexpr (use_colours) set_colour(out, Colour::RED);
        out << e.what() << '\n';
        if constexpr (use_colours) set_colour(out);
    } catch (const std::exception& e) {
        out << "Unhandled Execution Error: " << e.what() << '\n';
    }
}

void Walker::walk_raw(const stmt& statement) {
    // Walk the AST and evaluate the statement
    std::visit(
        [this]<typename T0>(const T0& stmt) {
            using T = std::decay_t<T0>;
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
            } else {
                throw ExecutionException("Unknown statement type",
                                         "Walker::walk_raw");
            }
        },
        statement);
}

void Walker::walk_decl_stmt(const decl_stmt& statement) {
    // Handle declaration statement
    for (const auto& identifier : statement.identifiers) {
        if (!globals.contains(identifier.lexeme)) {
            auto new_var = Bvar_ptype{identifier.lexeme};
            globals[identifier.lexeme] = new_var;
            bdd_ordering_map[identifier.lexeme] = bdd_ordering.size();
            bdd_ordering.push_back(identifier.lexeme);
            out << "Declared Symbolic Variable: " << identifier.lexeme << '\n';
        } else {
            if (std::holds_alternative<Bvar_ptype>(
                    globals[identifier.lexeme])) {
                out << "Variable already declared: " << identifier.lexeme
                    << '\n';
            } else {
                out << "Variable name conflict (making a variable holding a "
                       "bdd symbolic), "
                       "ignoring: "
                    << identifier.lexeme << '\n';
            }
        }
    }
}

void Walker::walk_assign_stmt(const assign_stmt& statement) {
    // Handle assignment statement
    if (globals.contains(statement.target->name.lexeme) &&
        !std::holds_alternative<Bdd_ptype>(
            globals[statement.target->name.lexeme])) {
        out << "Variable name conflict (assigning to symbolic variable), "
               "ignoring assignment of: "
            << statement.target->name.lexeme << '\n';
        return;
    }

    const id_type bdd_id = construct_bdd(*statement.value);
    globals[statement.target->name.lexeme] =
        Bdd_ptype{statement.target->name.lexeme, bdd_id};
    out << "Assigned to " << statement.target->name.lexeme
        << " with BDD ID: " << bdd_id << '\n';
}

void Walker::walk_func_call_stmt(const func_call_stmt& statement) {
    switch (statement.func_name.type) {
        case token::Type::TREE_DISPLAY: {
            LOG(INFO) << "Tree Display Function Called";
            if (statement.arguments.size() != 1) {
                throw ExecutionException(
                    "Invalid number of arguments for tree display", __func__);
            }
            id_type bdd_id = construct_bdd(*statement.arguments[0]);
            out << "BDD ID: " << bdd_id << '\n';
            out << bdd_repr(bdd_id) << '\n';
            break;
        }
        case token::Type::GRAPH_DISPLAY: {
            if (statement.arguments.size() != 1) {
                throw ExecutionException(
                    "Invalid number of arguments for graph display", __func__);
            }

            id_type bdd_id = construct_bdd(*statement.arguments[0]);
            std::string gviz_rep = bdd_gviz_repr(bdd_id);
            out << gviz_rep << '\n';
            break;
        }
        case token::Type::IS_SAT: {
            if (statement.arguments.size() != 1) {
                throw ExecutionException(
                    "Invalid number of arguments for is_sat", __func__);
            }

            if (id_type bdd_id = construct_bdd(*statement.arguments[0]);
                is_sat(bdd_id)) {
                out << "satisfiable" << '\n';
            } else {
                out << "unsatisfiable" << '\n';
            }
            break;
        }
        case token::Type::SOURCE: {
            if (statement.arguments.size() != 1) {
                throw ExecutionException(
                    "Invalid number of arguments for source", __func__);
            }
            LOG(INFO) << "Source Function Called" << '\n';

            if (!std::holds_alternative<identifier>(*statement.arguments[0])) {
                throw ExecutionException("Invalid argument type for source",
                                         __func__);
            }

            auto filename =
                std::get<identifier>(*statement.arguments[0]).name.lexeme;

            // Read all of the file into the buffer
            std::string buffer;
            std::ifstream f(filename);
            if (!f.is_open()) {
                out << "Failed to open file: " << filename;
                return;
            }
            f.seekg(0, std::ios::end);
            buffer.resize(f.tellg());
            f.seekg(0);
            f.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            f.close();

            if (f.fail()) {
                out << "Failed to read file: " << filename;
                return;
            }

            if (buffer.empty()) {
                out << "File is empty: " << filename;
                return;
            }

            lex_result_t tokens = scan_to_tokens(buffer);
            if (!tokens.has_value()) {
                out << tokens.error().what() << '\n';
                return;
            }
            parse_result_t estmts = parse(*tokens);
            if (!estmts.has_value()) {
                for (const auto& error : estmts.error()) {
                    out << error.what() << '\n';
                }
                return;
            }

            walk_statements(*estmts);
            break;
        }
        default:
            throw ExecutionException("Unknown function call", __func__);
    }
}

void Walker::walk_expr_stmt(const expr_stmt& statement) {
    // Handle expression statement
    const id_type bdd_id = construct_bdd(*statement.expression);
    out << "BDD ID: " << bdd_id << '\n';
}
