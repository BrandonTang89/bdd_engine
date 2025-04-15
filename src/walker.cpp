
#include "walker.h"

#include "absl/log/log.h"

Walker::Walker() : counter(2) {
    // Initialize the Walker
    Bdd_Node false_node{Bdd_Node::Bdd_type::FALSE, "false", 0, 0};
    Bdd_Node true_node{Bdd_Node::Bdd_type::TRUE, "true", 1, 1};
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

void Walker::walk(const stmt& statement) {
    // Walk the AST and evaluate the statement
    std::visit(
        [this](const auto& stmt) {
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
            } else {
                assert(false && "Unknown statement type");
            }
        },
        statement);
}

void Walker::walk_decl_stmt(const decl_stmt& statement) {
    // Handle declaration statement
    for (const auto& identifier : statement.identifiers) {
        if (globals.find(identifier.lexeme) == globals.end()) {
            auto new_var = Bvar_ptype{identifier.lexeme};
            globals[identifier.lexeme] = new_var;
            bdd_ordering_map[identifier.lexeme] = bdd_ordering.size();
            bdd_ordering.push_back(identifier.lexeme);
            out << "Declared Symbolic Variable: " << identifier.lexeme << '\n';
        } else {
            if (std::holds_alternative<Bvar_ptype>(globals[identifier.lexeme])) {
                out << "Variable already declared: " << identifier.lexeme << '\n';
            } else {
                out << "Variable name conflict (making a variable holding a bdd symbolic), "
                       "ignoring: "
                    << identifier.lexeme << '\n';
            }
        }
    }
}

void Walker::walk_assign_stmt(const assign_stmt& statement) {
    // Handle assignment statement

    // LOG(ERROR) << std::holds_alternative<Bdd_ptype>(globals[statement.target->name.lexeme]);

    if (globals.find(statement.target->name.lexeme) != globals.end() &&
        !std::holds_alternative<Bdd_ptype>(globals[statement.target->name.lexeme])) {
        out << "Variable name conflict (assigning to symbolic variable), ignoring assignment of: "
            << statement.target->name.lexeme << '\n';
        return;
    }

    auto bdd_id_opt = construct_bdd_safe(*statement.value);
    if (!bdd_id_opt.has_value()) return;
    id_type bdd_id = bdd_id_opt.value();
    globals[statement.target->name.lexeme] = Bdd_ptype{statement.target->name.lexeme, bdd_id};
    out << "Assigned to " << statement.target->name.lexeme << " with BDD ID: " << bdd_id
        << '\n';
}

void Walker::walk_func_call_stmt(const func_call_stmt& statement) {
    switch (statement.func_name.type) {
        case Token::Type::TREE_DISPLAY: {
            LOG(INFO) << "Tree Display Function Called";
            if (statement.arguments.size() != 1) {
                LOG(ERROR) << "Invalid number of arguments for tree display";
            }
            auto bdd_id_opt = construct_bdd_safe(*statement.arguments[0]);
            if (!bdd_id_opt.has_value()) break;
            id_type bdd_id = bdd_id_opt.value();
            out << "BDD ID: " << bdd_id << '\n';
            out << bdd_repr(bdd_id) << '\n';
            break;
        }
        case Token::Type::GRAPH_DISPLAY: {
            if (statement.arguments.size() != 1) {
                LOG(ERROR) << "Invalid number of arguments for graph display";
            }
            auto bdd_id_opt = construct_bdd_safe(*statement.arguments[0]);
            if (!bdd_id_opt.has_value()) break;
            id_type bdd_id = bdd_id_opt.value();
            std::string gviz_rep = bdd_gviz_repr(bdd_id);
            out << gviz_rep << '\n';
            break;
        }
        case Token::Type::IS_SAT: {
            if (statement.arguments.size() != 1) {
                LOG(ERROR) << "Invalid number of arguments for is_sat";
                return;
            }
            LOG(INFO) << "Is SAT Function Called" << '\n';
            auto bdd_id_opt = construct_bdd_safe(*statement.arguments[0]);
            if (!bdd_id_opt.has_value()) break;
            id_type bdd_id = bdd_id_opt.value();
            bool sat = is_sat(bdd_id);

            if (sat) {
                out << "satisfiable" << '\n';
            } else {
                out << "unsatisfiable" << '\n';
            }
            break;
        }
        default:
            LOG(ERROR) << "Unknown function call";
    }
}

void Walker::walk_expr_stmt(const expr_stmt& statement) {
    // Handle expression statement
    auto bdd_id_opt = construct_bdd_safe(*statement.expression);
    if (!bdd_id_opt.has_value()) return;

    out << "BDD ID: " << bdd_id_opt.value() << '\n';
}
