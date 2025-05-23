
#include <fstream>

#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "catch2/catch_test_macros.hpp"
#include "interp_tester.h"

TEST_CASE("Assignments and Usage") {
    InterpTester interp;
    interp.feed("bvar x y z;");

    SECTION("Simple Assignments") {
        interp.feed("set a = x & y;");
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        interp.feed("set b = x | z;");
        REQUIRE(interp.expr_tree_repr("b,") ==
                "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Reusing Assigned Variables") {
        interp.feed("set a = x & y;");
        interp.feed("set b = a | z;");
        REQUIRE(interp.expr_tree_repr("b") ==
                "x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : "
                "(FALSE))");

        interp.feed("set c = !a & z;");
        REQUIRE(interp.expr_tree_repr("c") ==
                "x ? (y ? (FALSE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : "
                "(FALSE))");
    }

    SECTION("Overwriting Variables") {
        interp.feed("set a = x & y;");
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        interp.feed("set a = x | z;");
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Complex Assignments") {
        interp.feed("set a = x & y | z;");
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : "
                "(FALSE))");

        interp.feed("set b = !a & x | y;");
        REQUIRE(interp.expr_tree_repr("b") ==
                "x ? (y ? (TRUE) : (z ? (FALSE) : (TRUE))) : (y ? (TRUE) : "
                "(FALSE))");
    }

    SECTION("Assignments with Implies") {
        interp.feed("set a = x -> y;");
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (TRUE)");

        interp.feed("set b = !(x -> y);");
        REQUIRE(interp.expr_tree_repr("b") ==
                "x ? (y ? (FALSE) : (TRUE)) : (FALSE)");

        interp.feed("set c = (x & y) -> z;");
        REQUIRE(interp.expr_tree_repr("c") ==
                "x ? (y ? (z ? (TRUE) : (FALSE)) : (TRUE)) : (TRUE)");

        interp.feed("set d = x -> (y -> z);");
        REQUIRE(interp.expr_tree_repr("d") ==
                "x ? (y ? (z ? (TRUE) : (FALSE)) : (TRUE)) : (TRUE)");

        interp.feed("set e = x -> y -> z;");
        REQUIRE(interp.expr_tree_repr("e") ==
                "x ? (y ? (z ? (TRUE) : (FALSE)) : (TRUE)) : (TRUE)");
    }

    SECTION("Assignment with Equality") {
        interp.feed("set a = (x == y);");
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (y ? (FALSE) : (TRUE))");

        interp.feed("set b = (x -> y) == (x -> y);");
        REQUIRE(interp.expr_tree_repr("b") == "TRUE");

        REQUIRE(interp.expr_tree_repr("x -> y == y -> z") ==
                "x ? (y ? (z ? (TRUE) : (FALSE)) : (FALSE)) : (y ? (z ? "
                "(TRUE) : (FALSE)) : (TRUE))");
    }

    SECTION("Assignment with Inequality") {
        interp.feed("set a = (x != y);");
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (FALSE) : (TRUE)) : (y ? (TRUE) : (FALSE))");

        interp.feed("set b = (x -> y) != (x -> y);");
        REQUIRE(interp.expr_tree_repr("b") == "FALSE");

        REQUIRE(interp.expr_tree_repr("x -> y != y -> z") ==
                "x ? (y ? (z ? (FALSE) : (TRUE)) : (TRUE)) : (y ? (z ? "
                "(FALSE) : (TRUE)) : (FALSE))");
    }
}

TEST_CASE("Constructing Expression with Quantifiers") {
    InterpTester interp;
    interp.feed("bvar x y z w;");

    SECTION("Single Bound Variable Quantifier") {
        REQUIRE(interp.expr_tree_repr("exists (x) true") == "TRUE");
        REQUIRE(interp.expr_tree_repr("forall (x) true") == "TRUE");
        REQUIRE(interp.expr_tree_repr("exists (x) false") == "FALSE");
        REQUIRE(interp.expr_tree_repr("forall (x) false") == "FALSE");

        REQUIRE(interp.expr_tree_repr("forall (x) x") == "FALSE");
        REQUIRE(interp.expr_tree_repr("exists (x) x") == "TRUE");
        REQUIRE(interp.expr_tree_repr("forall (x) (x & y)") == "FALSE");
        REQUIRE(interp.expr_tree_repr("exists (x) (x & y)") ==
                "y ? (TRUE) : (FALSE)");
    }

    SECTION("Single Bound Variable Semantic Sugar") {
        REQUIRE(interp.expr_tree_repr("forall x x | x") ==
                "x ? (TRUE) : (FALSE)");
        REQUIRE(interp.expr_tree_repr("forall x (x | x)") == "FALSE");
        REQUIRE(interp.expr_tree_repr("exists x (x & y)") ==
                "y ? (TRUE) : (FALSE)");
    }

    SECTION("Multiple Bound Variables") {
        REQUIRE(interp.expr_tree_repr("forall (x y) (x | y)") == "FALSE");
        REQUIRE(interp.expr_tree_repr("exists (x y) (x & y)") == "TRUE");
        REQUIRE(interp.expr_tree_repr("forall (y x w) z") ==
                "z ? (TRUE) : (FALSE)");
        REQUIRE(interp.expr_tree_repr("exists (x y) (x & y & !z);") ==
                "z ? (FALSE) : (TRUE)");
    }

    SECTION("Showing Precedence") {
        REQUIRE(interp.expr_tree_repr("forall (x) x | forall (y) y") ==
                "FALSE");
        REQUIRE(interp.expr_tree_repr("exists (x) x & exists (y) y") == "TRUE");
    }
}

TEST_CASE("Satisfiability Tests") {
    InterpTester interp;
    interp.feed("bvar x y z;");

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
    interp.feed("bvar x y z;");
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
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }
}

TEST_CASE("Declaration Errors") {
    InterpTester interp;
    interp.feed("bvar x y z;");

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

TEST_CASE("Multiple Errors") {
    InterpTester interp;

    SECTION("We should stop execution on the first error") {
        interp.feed("set a = a; bvar x;");

        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));

        interp.feed("bvar x;");
        REQUIRE(absl::StrContains(interp.get_output(),
                                  "Declared Symbolic Variable"));
    }

    SECTION("Error recovery works as expected") {
        interp.feed("set a = invalid;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));

        interp.feed("bvar x;");
        REQUIRE(absl::StrContains(interp.get_output(),
                                  "Declared Symbolic Variable"));
    }
}

TEST_CASE("Source Function") {
    InterpTester interp;

    SECTION("Valid Source Code") {
        std::string source_code = R"(
            bvar x y z;
            set a = x & y;
            set b = a | z;
            display_tree(a);
        )";

        std::ofstream source_file("test_source_code.txt");
        source_file << source_code;
        source_file.close();

        interp.feed("source test_source_code.txt;");

        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        std::remove("test_source_code.txt");
    }

    SECTION("Nonexistent Source File") {
        interp.feed("source nonexistent_file.txt;");
        REQUIRE(absl::StrContains(interp.get_output(), "Failed to open file"));
    }
}

TEST_CASE("Using IDs as Expressions") {
    InterpTester interp;
    interp.feed("bvar x y z;");

    SECTION("Valid ID Usage") {
        interp.feed("x & y;");
        auto out = interp.get_output();
        std::vector<std::string> numbers =
            absl::StrSplit(out, ' ', [](absl::string_view s) {
                int number;
                return absl::SimpleAtoi(s, &number);
            });
        REQUIRE(numbers.size() > 0);
        int id = 0;
        absl::SimpleAtoi(numbers[0], &id);

        REQUIRE(interp.expr_tree_repr(std::format("z & {};", id)) ==
                "x ? (y ? (z ? (TRUE) : (FALSE)) : (FALSE)) : (FALSE)");
    }

    SECTION("Invalid ID Usage") {
        interp.feed("x & 100;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }
}