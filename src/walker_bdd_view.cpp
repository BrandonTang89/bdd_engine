#include <queue>

#include "walker.h"

bool Walker::is_sat(const id_type a) {
    // Check if the BDD is satisfiable
    if (const auto it = is_sat_memo.find(a); it != is_sat_memo.end()) {
        return it->second;
    }
    const Bdd_Node& node = id_to_iter[a]->first;

    // Base case set up in walker constructor
    // if (node.type == Bdd_Node::Bdd_type::TRUE) {
    //     return is_sat_memo[a] = true;
    // } else if (node.type == Bdd_Node::Bdd_type::FALSE) {
    //     return is_sat_memo[a] = false;
    // }

    const bool result = is_sat(node.high) || is_sat(node.low);
    is_sat_memo[a] = result;
    return result;
}

std::string Walker::bdd_repr(const id_type id) {
    // Prints the BDD as a tree
    // Caution: the tree representation can be exponentially large
    const Bdd_Node& node = id_to_iter[id]->first;
    if (node.type == Bdd_Node::Bdd_type::INTERNAL) {
        return node.var + " ? (" + bdd_repr(node.high) + ") : (" +
               bdd_repr(node.low) + ")";
    } else if (node.type == Bdd_Node::Bdd_type::TRUE) {
        return "TRUE";
    } else if (node.type == Bdd_Node::Bdd_type::FALSE) {
        return "FALSE";
    }
    throw std::runtime_error("Unknown BDD type");
}

std::unordered_set<id_type> Walker::get_bdd_nodes(const id_type id) {
    // Returns a set of all BDD nodes reachable from the given id
    // This is a breadth-first search to find all nodes in the BDD
    std::unordered_set<id_type> visited;
    std::queue<id_type> q;

    q.push(id);
    visited.insert(id);

    while (!q.empty()) {
        id_type current = q.front();
        q.pop();

        if (const Bdd_Node& node = id_to_iter[current]->first;
            node.type == Bdd_Node::Bdd_type::INTERNAL) {
            if (!visited.contains(node.high)) {
                q.push(node.high);
                visited.insert(node.high);
            }
            if (!visited.contains(node.low)) {
                q.push(node.low);
                visited.insert(node.low);
            }
        }
    }

    return visited;
}

std::string Walker::bdd_gviz_repr(const id_type id) {
    // Prints the BDD in Graphviz format
    // Solid edges for high branches, dashed edges for low branches
    const std::unordered_set<id_type> bdd_ids = get_bdd_nodes(id);

    std::string gviz = "digraph G {\n";
    for (const auto& bdd_id : bdd_ids) {
        const Bdd_Node& node = id_to_iter[bdd_id]->first;
        if (node.type == Bdd_Node::Bdd_type::INTERNAL) {
            gviz += "  " + std::to_string(bdd_id) + " [label=\"" + node.var +
                    "\"];\n";
            gviz += "  " + std::to_string(bdd_id) + " -> " +
                    std::to_string(node.high) + " [style=\"solid\"];\n";
            gviz += "  " + std::to_string(bdd_id) + " -> " +
                    std::to_string(node.low) + " [style=\"dashed\"];\n";
        } else if (node.type == Bdd_Node::Bdd_type::TRUE) {
            gviz += "  " + std::to_string(bdd_id) + " [label=\"TRUE\"];\n";
        } else if (node.type == Bdd_Node::Bdd_type::FALSE) {
            gviz += "  " + std::to_string(bdd_id) + " [label=\"FALSE\"];\n";
        }
    }

    gviz += "}\n";
    return gviz;
}