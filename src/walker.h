#pragma once
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <string>
#include <variant>

#include "ast.h"

// Runtime BDD Structure
struct Bdd_Node;
using id_type = uint32_t;
using bdd_set_type = std::set<Bdd_Node>;
using iter_type = bdd_set_type::iterator;
using from_human_map_type = std::map<id_type, iter_type>;  // human_ids -> iter
using to_human_map_type = std::map<Bdd_Node, id_type>;    // iter -> human_ids

struct Bdd_Node {
    enum class Bdd_type {
        TRUE,
        FALSE,
        INTERNAL,
    };

    Bdd_type type{};
    std::string var{};  // variable name if internal
    id_type high;
    id_type low;  // only used for internal nodes, set to the end of the set if not used

    auto operator<=>(const Bdd_Node&) const = default;
};

// Varible Types
// Each variable is either a BDD symbol or a variable that represents a binary decision diagram
struct Bvar_ptype {
    // Binary variable type
    std::string name{};
};

struct Bdd_ptype {
    // Expression type
    std::string name{};
    id_type id{};
};

using Ptype = std::variant<Bvar_ptype, Bdd_ptype>;
enum class Ptype_type : size_t { BVAR = 0,
                                 BDD = 1 };

class Walker {
    // An instance of the tree-walk interpreter
    // Manages the environment of the interpreter and available BDDs in the memory

   private:
    id_type counter{};  // monotonically increasing human indices
    bdd_set_type bdd_set;
    from_human_map_type from_human_map;
    to_human_map_type to_human_map;

    iter_type iter_to_false;
    iter_type iter_to_true;

    std::map<id_type, std::vector<id_type>> bdd_parents;  // for reductions
    std::unordered_map<std::string, Ptype> globals;
    void walk_decl_stmt(const decl_stmt& statement);

    iter_type new_bdd_node(Bdd_Node node);

   public:
    Walker() {
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

    void walk(const stmt& statement);

    id_type construct_bdd(const expr& x);

    id_type and_bdd(id_type a, id_type b);
    id_type or_bdd(id_type a, id_type b);
    id_type negate_bdd(id_type a);
};
