#include <queue>

#include "walker.h"

std::string Walker::bdd_repr(id_type id) {
    // Prints the BDD as a tree
    // Caution: the tree representation can be exponentially large
    const Bdd_Node& node = id_to_iter[id]->first;
    if (node.type == Bdd_Node::Bdd_type::INTERNAL) {
        return (node.var + " ? (" + bdd_repr(node.high) + ") : (" + bdd_repr(node.low)) + ")";
    } else if (node.type == Bdd_Node::Bdd_type::TRUE) {
        return "TRUE";
    } else if (node.type == Bdd_Node::Bdd_type::FALSE) {
        return "FALSE";
    }
    throw std::runtime_error("Unknown BDD type");
}

std::unordered_set<id_type> Walker::get_bdd_nodes(id_type id) {
    // Returns a set of all BDD nodes reachable from the given id
    // This is a breadth-first search to find all nodes in the BDD
    std::unordered_set<id_type> visited;
    std::queue<id_type> q;

    q.push(id);
    visited.insert(id);

    while (!q.empty()) {
        id_type current = q.front();
        q.pop();

        const Bdd_Node& node = id_to_iter[current]->first;
        if (node.type == Bdd_Node::Bdd_type::INTERNAL) {
            if (visited.find(node.high) == visited.end()) {
                q.push(node.high);
                visited.insert(node.high);
            }
            if (visited.find(node.low) == visited.end()) {
                q.push(node.low);
                visited.insert(node.low);
            }
        }
    }

    return visited;
}

std::string Walker::bdd_gviz_repr(id_type id) {
    // Prints the BDD in Graphviz format
    // Solid edges for high branches, dashed edges for low branches
    std::unordered_set<id_type> bdd_ids = get_bdd_nodes(id);

    std::string gviz = "digraph G {\n";
    for (const auto& bdd_id : bdd_ids) {
        const Bdd_Node& node = id_to_iter[bdd_id]->first;
        if (node.type == Bdd_Node::Bdd_type::INTERNAL) {
            gviz += "  " + std::to_string(bdd_id) + " [label=\"" + node.var + "\"];\n";
            gviz += "  " + std::to_string(bdd_id) + " -> " + std::to_string(node.high) + " [style=\"solid\"];\n";
            gviz += "  " + std::to_string(bdd_id) + " -> " + std::to_string(node.low) + " [style=\"dashed\"];\n";
        } else if (node.type == Bdd_Node::Bdd_type::TRUE) {
            gviz += "  " + std::to_string(bdd_id) + " [label=\"TRUE\"];\n";
        } else if (node.type == Bdd_Node::Bdd_type::FALSE) {
            gviz += "  " + std::to_string(bdd_id) + " [label=\"FALSE\"];\n";
        }
    }

    gviz += "}\n";
    return gviz;
}