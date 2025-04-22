
#include <variant>
#include <vector>

#include "../src/parser.h"
#include "../src/token.h"
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Parse Valid") {
    std::string input = R"(
        bvar x y z;
        set a = x & y;
        set b = a | z;
        set c = exists x (a & b);
        display_tree a;
    )";

    std::vector<Token> tokens = scan_to_tokens(input);
    std::vector<stmt> statements = parse(tokens);

    REQUIRE(statements.size() == 5);
    REQUIRE(std::holds_alternative<decl_stmt>(statements[0]));
    REQUIRE(std::holds_alternative<assign_stmt>(statements[1]));
    REQUIRE(std::holds_alternative<assign_stmt>(statements[2]));
    REQUIRE(std::holds_alternative<assign_stmt>(statements[3]));
    REQUIRE(std::holds_alternative<func_call_stmt>(statements[4]));
}

TEST_CASE("Invalid Declaration") {
    SECTION("Declaration with commas") {
        std::string input = R"(
            bvar x, y, z;
        )";
        std::vector<Token> tokens = scan_to_tokens(input);
        std::vector<stmt> statements = parse(tokens);

    }
}