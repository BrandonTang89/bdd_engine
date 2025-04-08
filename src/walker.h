#pragma once
#include <map>
#include <string>
#include <unordered_map>

#include "ast.h"

// Runtime BDD Structure
struct Bdd_Node;
using id_type = uint32_t;
using node_id_map = std::map<Bdd_Node, id_type>;  // iter -> human_ids
using iter_type = node_id_map::iterator;
using id_iter_map = std::unordered_map<id_type, iter_type>;  // human_ids -> iter

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

    auto operator<=>(const Bdd_Node&) const = default;
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

    public:
    Walker();
    std::string bdd_repr(id_type id);
    id_type walk_expr_stmt(const expr_stmt& statement);
    void walk(const stmt& statement);
};
