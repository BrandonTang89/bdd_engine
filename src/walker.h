#pragma once
#include <sstream>
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
    bool preserved{};
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
    node_id_map node_to_id;  // main map that holds the BDD nodes

    iter_type iter_to_false;
    iter_type iter_to_true;

    std::unordered_map<std::string, Ptype> globals;

    std::vector<std::string> bdd_ordering;  // for BDD ordering
    std::unordered_map<std::string, uint32_t> bdd_ordering_map;

    // === Walking Statements ===
    void walk_raw(const stmt& statement);  // May throw execution exceptions,
                                           // dispatches to the correct function
    void walk_decl_stmt(const decl_stmt& statement);
    void walk_assign_stmt(const assign_stmt& statement);
    void walk_func_call_stmt(const func_call_stmt& statement);
    void walk_expr_stmt(const expr_stmt& statement);

    // === BDD Construction ===
    id_type construct_bdd(const expr& x);
    id_type get_id(const Bdd_Node& node);

    std::unordered_map<std::tuple<id_type, id_type, BinOpType>, id_type,
                       absl::Hash<std::tuple<id_type, id_type, BinOpType>>>
        binop_memo;  // (reusable)
    id_type rec_apply_and(id_type a, id_type b);
    id_type rec_apply_or(id_type a, id_type b);

    std::map<id_type, id_type> not_memo;  // (reusable)
    id_type rec_apply_not(id_type a);

    std::unordered_map<std::tuple<id_type, size_t>, id_type,
                       absl::Hash<std::tuple<id_type, size_t>>>
        quantifier_memo;  // (unreusable)

    template <typename Comb_Fn_Type>
    id_type rec_apply_quant(id_type a, std::span<std::string> bound_vars,
                            Comb_Fn_Type comb_fn);

    // ==== Substitution ====
    // Convert BDDs back to Expressions for Substitution
    std::shared_ptr<expr> false_expr{
        std::make_shared<expr>(literal{token{token::Type::FALSE, "false"}})};
    std::shared_ptr<expr> true_expr{
        std::make_shared<expr>(literal{token{token::Type::TRUE, "true"}})};

    // Reconstruct expr from bdd id
    std::shared_ptr<expr> construct_expr(id_type id);
    // Cache expr reconstructions (reusable)
    std::unordered_map<id_type, std::shared_ptr<expr>> id_to_expr_memo;

    // Cache substituted expressions for specific substitutions (unreusable)
    std::unordered_map<std::shared_ptr<expr>, std::shared_ptr<expr>> sub_memo;
    std::shared_ptr<expr> substitute_expr(
        const std::shared_ptr<expr>& x,
        const substitution_map& sub_map);  // substitute variables in expr

    // === BDD Viewing ===
    // check if BDD is satisfiable
    std::unordered_map<id_type, bool> is_sat_memo;
    bool is_sat(id_type a);

    std::unordered_set<id_type> get_bdd_nodes(id_type id);
    std::string bdd_repr(id_type id);
    std::string bdd_gviz_repr(id_type id);

    // === Memory Management ===
    void clear_memos();
    void sweep(); // sweep non-preserved BDDs from memory

   public:
    Walker();
    void walk_single(const stmt& statement);  // Walk AST, handles exceptions
    void walk_statements(
        const std::span<stmt>& statements);  // Returns early on exceptions
    std::string
    get_output();  // clears the output buffer and returns the output
};
