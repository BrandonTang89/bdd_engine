
#include "walker.h"

std::string Walker::bdd_repr(id_type id) {
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