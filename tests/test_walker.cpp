#define CATCH_CONFIG_MAIN
#include "../src/parser.h"
#include "../src/walker.h"
#include "catch2/catch_test_macros.hpp"

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

    std::string bdd_repr(id_type id) {
        return walker.bdd_repr(id);
    }

    std::string expression_repr(const std::string& input) {
        auto tokens = scan_to_tokens(input);
        auto statements = parse(tokens);
        if (statements.size() != 1) {
            throw std::runtime_error("Expected a single statement");
        }
        auto statement = std::get<expr_stmt>(std::move(statements[0]));
        auto bdd_id = walker.construct_bdd(*statement.expression);
        return walker.bdd_repr(bdd_id);
    }
};

TEST_CASE("Sanity Test") {
    REQUIRE(1 + 1 == 2);
}

TEST_CASE("BDDs Constructed Are Logically Correct") {
    InterpTester interp;
    interp.feed("bvar x, y, z;");

    SECTION("Literals") {
        REQUIRE(interp.expression_repr("true;") == "TRUE");
        REQUIRE(interp.expression_repr("false;") == "FALSE");
    }

    SECTION("Primary Expressions") {
        REQUIRE(interp.expression_repr("x;") == "x ? (TRUE) : (FALSE)");
        REQUIRE(interp.expression_repr("y;") == "y ? (TRUE) : (FALSE)");
        REQUIRE(interp.expression_repr("z;") == "z ? (TRUE) : (FALSE)");
    }

    SECTION("Unary Expressions") {
        REQUIRE(interp.expression_repr("!x;") == "x ? (FALSE) : (TRUE)");
        REQUIRE(interp.expression_repr("!y;") == "y ? (FALSE) : (TRUE)");
        REQUIRE(interp.expression_repr("!z;") == "z ? (FALSE) : (TRUE)");
    }

    SECTION("Binary Expressions") {
        REQUIRE(interp.expression_repr("x & y;") == "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
        REQUIRE(interp.expression_repr("x | y;") == "x ? (TRUE) : (y ? (TRUE) : (FALSE))");
        REQUIRE(interp.expression_repr("x & true;") == "x ? (TRUE) : (FALSE)");
        REQUIRE(interp.expression_repr("x | false;") == "x ? (TRUE) : (FALSE)");
        REQUIRE(interp.expression_repr("x & false;") == "FALSE");
        REQUIRE(interp.expression_repr("x | true;") == "TRUE");
    }

    SECTION("Complex Expressions") {
        REQUIRE(interp.expression_repr("x & x & y | z;") == "x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))");
        REQUIRE(interp.expression_repr("x | y & z;") == "x ? (TRUE) : (y ? (z ? (TRUE) : (FALSE)) : (FALSE))");
        REQUIRE(interp.expression_repr("!x & y | z;") == "x ? (z ? (TRUE) : (FALSE)) : (y ? (TRUE) : (z ? (TRUE) : (FALSE)))");
    }
}

TEST_CASE("Assignments and Usage") {
    InterpTester interp;
    interp.feed("bvar x, y, z;");

    SECTION("Simple Assignments") {
        interp.feed("set a = x & y;");
        REQUIRE(interp.expression_repr("a;") == "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        interp.feed("set b = x | z;");
        REQUIRE(interp.expression_repr("b,") == "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Reusing Assigned Variables") {
        interp.feed("set a = x & y;");
        interp.feed("set b = a | z;");
        REQUIRE(interp.expression_repr("b;") == "x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))");

        interp.feed("set c = !a & z;");
        REQUIRE(interp.expression_repr("c;") == "x ? (y ? (FALSE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Overwriting Variables") {
        interp.feed("set a = x & y;");
        REQUIRE(interp.expression_repr("a;") == "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        interp.feed("set a = x | z;");
        REQUIRE(interp.expression_repr("a;") == "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Complex Assignments") {
        interp.feed("set a = x & y | z;");
        REQUIRE(interp.expression_repr("a;") == "x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))");

        interp.feed("set b = !a & x | y;");
        REQUIRE(interp.expression_repr("b;") == "x ? (y ? (TRUE) : (z ? (FALSE) : (TRUE))) : (y ? (TRUE) : (FALSE))");
    }
}
