#include "../src/parser.h"
#include "../src/walker.h"
class InterpTester {
    Walker walker;

   public:
    void feed(const std::string& input) {
        auto tokens = scan_to_tokens(input);
        auto statements = parse(tokens);
        for (const auto& statement : statements) {
            walker.walk(statement);
        }
    }

    id_type interpret_expr(std::string input) {
        input.push_back(';');  // must have some end of line character
        const auto tokens = scan_to_tokens(input);
        auto it = tokens.begin();
        auto ptr_expr = parse_expr(it);
        auto bdd_id = walker.construct_bdd(*ptr_expr);
        return bdd_id;
    }

    std::string expr_tree_repr(std::string input) {
        return walker.bdd_repr(interpret_expr(input));
    }

    std::string get_output() {
        return walker.get_output();
    }

    bool is_sat(std::string input) {
        return walker.is_sat(interpret_expr(input));
    }
};
