#pragma once
#include <string>
#include <unordered_map>

#include "absl/hash/hash.h"
#include "ast.h"

// Runtime BDD Structure
using id_type = uint32_t;
struct Bdd_Node {
    enum class Bdd_type : std::uint8_t {
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

// Binary operation types for the memo table
enum class BinOpType : std::uint8_t { AND, OR };

// Variable Types
// Each variable is either a BDD symbol or a variable that represents a binary
// decision diagram
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
enum class Ptype_type : std::uint8_t { BVAR = 0, BDD = 1 };

// Walker Types to hold BDDs
using node_id_map = std::unordered_map<Bdd_Node, id_type, absl::Hash<Bdd_Node>>;
using iter_type = node_id_map::iterator;
using id_iter_map =
    std::unordered_map<id_type, iter_type>;  // human_ids -> iter

class Walker {
    // An instance of the tree-walk interpreter
    // Manages the environment of the interpreter and available BDDs in the
    // memory Side-effect-free, output is written to this->out stream
    friend class InterpTester;

    std::ostringstream out;  // printable output
    id_type counter{};       // monotonically increasing human indices
    id_iter_map id_to_iter;
    node_id_map node_to_id;

    iter_type iter_to_false;
    iter_type iter_to_true;

    // absl::flat_hash_map<std::string, Ptype> globals;
    std::unordered_map<std::string, Ptype> globals;
    void walk_raw(const stmt& statement);  // May throw execution exceptions,
                                           // dispatches to the correct function
    void walk_decl_stmt(const decl_stmt& statement);
    void walk_assign_stmt(const assign_stmt& statement);
    void walk_func_call_stmt(const func_call_stmt& statement);
    void walk_expr_stmt(const expr_stmt& statement);

    std::vector<std::string> bdd_ordering;  // for BDD ordering
    std::unordered_map<std::string, uint32_t>
        bdd_ordering_map;  // for BDD ordering

    id_type construct_bdd(const expr& x);
    id_type get_id(const Bdd_Node& node);

    // BDD Construction
    std::unordered_map<std::tuple<id_type, id_type, BinOpType>, id_type, absl::Hash<std::tuple<id_type, id_type, BinOpType>>> binop_memo;
    id_type rec_apply_and(id_type a, id_type b);
    id_type rec_apply_or(id_type a, id_type b);

    std::map<id_type, id_type> not_memo;
    id_type rec_apply_not(id_type a);

    std::unordered_map<std::tuple<id_type, size_t>, id_type, absl::Hash<std::tuple<id_type, size_t>>> quantifier_memo;
    // (bdd_id, number_of_bound_vars_left)

    template <typename Comb_Fn_Type>
    id_type rec_apply_quant(id_type a, std::span<std::string> bound_vars,
                            Comb_Fn_Type comb_fn);

    // // BDD Viewing
    std::unordered_map<id_type, bool> is_sat_memo;  // check if BDD is satisfiable
    bool is_sat(id_type a);

    std::unordered_set<id_type> get_bdd_nodes(id_type id);
    std::string bdd_repr(id_type id);
    std::string bdd_gviz_repr(id_type id);

    void clear_memos();

   public:
    Walker();
    void walk_single(const stmt& statement);  // Walk AST, handles exceptions
    void walk_statements(
        const std::span<stmt>& statements);  // Returns early on exceptions
    std::string
    get_output();  // clears the output buffer and returns the output
};
