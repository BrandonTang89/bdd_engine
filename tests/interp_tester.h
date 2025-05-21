#pragma once
#include "../src/parser.h"
#include "../src/walker.h"
class InterpTester {
    std::ostringstream parser_error_stream;
    Walker walker;

   public:
    void feed(const std::string& input) {
        const auto tokens = scan_to_tokens(input);
        auto estmts = parse(tokens);
        assert(estmts.has_value());  // no parser error in interpreter tester

        walker.walk_statements(*estmts);
    }

    id_type interpret_expr(std::string input) {
        input.push_back(';');  // must have some end-of-line character
        const auto tokens = scan_to_tokens(input);
        const_span sp(tokens);
        const auto ptr_expr = parse_expr(sp);
        const auto bdd_id = walker.construct_bdd(*ptr_expr);
        return bdd_id;
    }

    std::string expr_tree_repr(std::string input) {
        return walker.bdd_repr(interpret_expr(std::move(input)));
    }

    std::string get_output() { return walker.get_output(); }

    std::string get_parser_error() {
        std::string error = parser_error_stream.str();
        parser_error_stream.str("");  // Clear the error stream
        return error;
    }

    bool is_sat(std::string input) { return walker.is_sat(interpret_expr(std::move(input))); }
};
