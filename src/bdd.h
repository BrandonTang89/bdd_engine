#pragma once
#include <map>

struct Bdd_Node {
    enum class Bdd_type {
        TRUE,
        FALSE,
        INTERNAL,
    };

    Bdd_type type{};
    std::string var{};  // variable name if internal
    uint32_t high{};    // only used for internal nodes
    uint32_t low{};     // only used for internal nodes

    auto operator<=>(const Bdd_Node&) const = default;
};

using id_type = uint32_t;
using bdd_map_type = std::map<uint32_t, Bdd_Node>;

const Bdd_Node TRUE_NODE{Bdd_Node::Bdd_type::TRUE, "true", 0, 0};
const Bdd_Node FALSE_NODE{Bdd_Node::Bdd_type::FALSE, "false", 0, 0};