#pragma once
#include <string>
#include <unordered_map>

#include "absl/hash/hash.h"
#include "ast.h"

// Runtime BDD Structure
using id_type = uint32_t;
struct Bdd_Node {
    enum class Bdd_type {
        TRUE,
        FALSE,
        INTERNAL,
    };

    Bdd_type type{};
    std::string var{};  // variable name if internal
    id_type high;
    id_type low;  // only used for internal nodes

    // Default equality comparison
    auto operator<=>(const Bdd_Node&) const = default;

    // Custom absl hash function
    template <typename H>
    friend H AbslHashValue(H h, const Bdd_Node& node) {
        return H::combine(std::move(h), node.var, node.high, node.low);
    }
};

// Variable Types
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

// Walker Types to hold BDDs
using node_id_map = std::unordered_map<Bdd_Node, id_type, absl::Hash<Bdd_Node>>;
using iter_type = node_id_map::iterator;
using id_iter_map = std::unordered_map<id_type, iter_type>;  // human_ids -> iter

class Walker {
    // An instance of the tree-walk interpreter
    // Manages the environment of the interpreter and available BDDs in the memory

    friend class InterpTester;

   private:
    id_type counter{};  // monotonically increasing human indices
    id_iter_map id_to_iter;
    node_id_map node_to_id;

    iter_type iter_to_false;
    iter_type iter_to_true;


    std::unordered_map<std::string, Ptype> globals;
    void walk_decl_stmt(const decl_stmt& statement);
    void walk_assign_stmt(const assign_stmt& statement);
    void walk_func_call_stmt(const func_call_stmt& statement);
    id_type walk_expr_stmt(const expr_stmt& statement);

    std::vector<std::string> bdd_ordering;                       // for BDD ordering
    std::unordered_map<std::string, uint32_t> bdd_ordering_map;  // for BDD ordering

    id_type construct_bdd(const expr& x);
    id_type get_id(const Bdd_Node& node);

    std::map<std::tuple<id_type, id_type>, id_type> and_memo;
    id_type rec_apply_and(id_type a, id_type b);
    id_type and_bdd(id_type a, id_type b);

    std::map<std::tuple<id_type, id_type>, id_type> or_memo;
    id_type rec_apply_or(id_type a, id_type b);
    id_type or_bdd(id_type a, id_type b);

    std::map<id_type, id_type> not_memo;
    id_type rec_apply_not(id_type a);
    id_type negate_bdd(id_type a);

    std::unordered_map<id_type, bool> is_sat_memo;  // check if BDD is satisfiable
    bool is_sat(id_type a);

    std::unordered_set<id_type> get_bdd_nodes(id_type id);
    std::string bdd_repr(id_type id);
    std::string bdd_gviz_repr(id_type id);

   public:
    Walker();
    void walk(const stmt& statement);
};
