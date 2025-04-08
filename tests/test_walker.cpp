#include <iostream>

#include "absl/strings/match.h"
#include "catch2/catch_test_macros.hpp"
#include "interp_tester.h"

TEST_CASE("Assignments and Usage") {
    InterpTester interp;
    interp.feed("bvar x, y, z;");

    SECTION("Simple Assignments") {
        interp.feed("set a = x & y;");
        REQUIRE(interp.expr_tree_repr("a") == "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        interp.feed("set b = x | z;");
        REQUIRE(interp.expr_tree_repr("b,") == "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Reusing Assigned Variables") {
        interp.feed("set a = x & y;");
        interp.feed("set b = a | z;");
        REQUIRE(interp.expr_tree_repr("b") == "x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))");

        interp.feed("set c = !a & z;");
        REQUIRE(interp.expr_tree_repr("c") == "x ? (y ? (FALSE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Overwriting Variables") {
        interp.feed("set a = x & y;");
        REQUIRE(interp.expr_tree_repr("a") == "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        interp.feed("set a = x | z;");
        REQUIRE(interp.expr_tree_repr("a") == "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Complex Assignments") {
        interp.feed("set a = x & y | z;");
        REQUIRE(interp.expr_tree_repr("a") == "x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : (FALSE))");

        interp.feed("set b = !a & x | y;");
        REQUIRE(interp.expr_tree_repr("b") == "x ? (y ? (TRUE) : (z ? (FALSE) : (TRUE))) : (y ? (TRUE) : (FALSE))");
    }
}

TEST_CASE("Satisfiability Tests") {
    InterpTester interp;
    interp.feed("bvar x, y, z;");

    SECTION("Simple Satisfiability") {
        REQUIRE(interp.is_sat("true") == true);
        REQUIRE(interp.is_sat("false") == false);
        REQUIRE(interp.is_sat("x") == true);
        REQUIRE(interp.is_sat("!x") == true);
    }

    SECTION("Binary Expressions Satisfiability") {
        REQUIRE(interp.is_sat("x & y") == true);
        REQUIRE(interp.is_sat("x | y") == true);
        REQUIRE(interp.is_sat("x & false") == false);
        REQUIRE(interp.is_sat("x | false") == true);
        REQUIRE(interp.is_sat("x & true") == true);
        REQUIRE(interp.is_sat("x | true") == true);
    }

    SECTION("Complex Expressions Satisfiability") {
        REQUIRE(interp.is_sat("x & y & z") == true);
        REQUIRE(interp.is_sat("x & y & !z") == true);
        REQUIRE(interp.is_sat("x & !y & !z") == true);
        REQUIRE(interp.is_sat("!x & !y & !z") == true);
        REQUIRE(interp.is_sat("x & y & false") == false);
        REQUIRE(interp.is_sat("x | y | z") == true);
    }

    SECTION("Assignments and Satisfiability") {
        interp.feed("set a = x & y;");
        REQUIRE(interp.is_sat("a") == true);

        interp.feed("set b = x & false;");
        REQUIRE(interp.is_sat("b") == false);

        interp.feed("set c = x | z;");
        REQUIRE(interp.is_sat("c") == true);

        interp.feed("set d = !x & !y & !z;");
        REQUIRE(interp.is_sat("d") == true);

        interp.feed("set e = x & y & z & false;");
        REQUIRE(interp.is_sat("e") == false);
    }

    SECTION("Negated Expressions Satisfiability") {
        REQUIRE(interp.is_sat("!x & !y & !z") == true);
        REQUIRE(interp.is_sat("!(x | y | z)") == true);
        REQUIRE(interp.is_sat("!(x & y & z)") == true);
        REQUIRE(interp.is_sat("!(x & false)") == true);
        REQUIRE(interp.is_sat("!(x | true)") == false);
    }
}

TEST_CASE("Assignment Errors") {
    InterpTester interp;
    interp.feed("bvar x, y, z;");
    SECTION("Assignment to Symbolic Variable") {
        interp.feed("set x = true;");
        REQUIRE(absl::StrContains(interp.get_output(), "conflict"));

        interp.feed("set y = x;");
        REQUIRE(absl::StrContains(interp.get_output(), "conflict"));

        interp.feed("set x = x;");
        REQUIRE(absl::StrContains(interp.get_output(), "conflict"));
    }

    SECTION("Assignment to Invalid Expression") {
        interp.feed("set a = a;");
        REQUIRE(absl::StrContains(interp.get_output(), "Error"));
    }
}

TEST_CASE("Declaration Errors") {
    InterpTester interp;
    interp.feed("bvar x, y, z;");

    SECTION("Redeclaration of Variables") {
        interp.feed("bvar x;");
        REQUIRE(absl::StrContains(interp.get_output(), "already"));
    }

    SECTION("Declaration of BDD variable") {
        interp.feed("set a = true;");
        interp.feed("bvar a;");
        REQUIRE(absl::StrContains(interp.get_output(), "conflict"));
    }
}