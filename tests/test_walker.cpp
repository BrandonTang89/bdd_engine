
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

TEST_CASE("Constructing Expressions with Substitutions") {
    InterpTester interp;
    interp.feed("bvar x y z w;");

    SECTION("Basic Substitutions") {
        // Simple variable substitution
        REQUIRE(interp.expr_tree_repr("sub {x: y} x") ==
                "y ? (TRUE) : (FALSE)");

        // Multiple substitutions
        REQUIRE(interp.expr_tree_repr("sub {x: y, y: z} (x & y)") ==
                "y ? (z ? (TRUE) : (FALSE)) : (FALSE)");

        // Constants in substitution
        REQUIRE(interp.expr_tree_repr("sub {x: true, y: false} (x & y)") ==
                "FALSE");
    }

    SECTION("Duplicate Substitutions") {
        // Last substitution takes precedence
        REQUIRE(interp.expr_tree_repr("sub {x: y, x: z} x") ==
                "z ? (TRUE) : (FALSE)");

        // Complex expressions with duplicates
        // All the substitutions apply simultaneously
        REQUIRE(interp.expr_tree_repr("sub {x: y & z, x: z | w} (x & y)") ==
                "y ? (z ? (TRUE) : (w ? (TRUE) : (FALSE))) : (FALSE)");
    }

    SECTION("Nested Substitutions in Lists") {
        // Nested in substitution list
        REQUIRE(interp.expr_tree_repr("sub {x: sub {y: z} (y & w)} x") ==
                "z ? (w ? (TRUE) : (FALSE)) : (FALSE)");

        // Multiple nested substitutions
        REQUIRE(interp.expr_tree_repr(
                    "sub {x: sub {y: z} y, y: sub {z: w} z} (x & y)") ==
                "z ? (w ? (TRUE) : (FALSE)) : (FALSE)");
    }

    SECTION("Nested Substitutions in Body") {
        // Substitution in the target expression
        REQUIRE(interp.expr_tree_repr("sub {x: y} (sub {y: z} x)") ==
                "y ? (TRUE) : (FALSE)");

        // Complex nested case
        REQUIRE(interp.expr_tree_repr("sub {y: w} (sub {x: y} (x & z))") ==
                "z ? (w ? (TRUE) : (FALSE)) : (FALSE)");
    }

    SECTION("Nested Substitutions in Both") {
        // Substitutions in both places
        REQUIRE(interp.expr_tree_repr("sub {x: sub {y: w} y} (sub {z: x} z)") ==
                "w ? (TRUE) : (FALSE)");

        // Complex nested case in both
        REQUIRE(interp.expr_tree_repr(
                    "sub {x: sub {y: w} (y & z)} (sub {w: x} (w | y))") ==
                "y ? (TRUE) : (z ? (w ? (TRUE) : (FALSE)) : (FALSE))");
    }

    SECTION("Substituting Multiple Variables") {
        // Multiple substitutions at once
        REQUIRE(interp.expr_tree_repr("sub {x: y, y: z, z: w} (x & y & z)") ==
                "y ? (z ? (w ? (TRUE) : (FALSE)) : (FALSE)) : (FALSE)");
    }

    SECTION("Substituting Constants") {
        // Substituting with constants
        REQUIRE(interp.expr_tree_repr("sub {x: true, y: false} (x -> y)") ==
                "FALSE");

        // Mixed constants and variables
        REQUIRE(interp.expr_tree_repr("sub {x: true, y: z} (x & y)") ==
                "z ? (TRUE) : (FALSE)");
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

TEST_CASE("Clear Cache Function") {
    InterpTester interp;

    SECTION("Clearing Cache") {
        interp.feed("bvar x y z;");
        interp.feed("set a = x & y;");
        interp.feed("set b = a | z;");

        // Ensure cache is populated
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
        REQUIRE(interp.expr_tree_repr("b") ==
                "x ? (y ? (TRUE) : (z ? (TRUE) : (FALSE))) : (z ? (TRUE) : "
                "(FALSE))");

        interp.get_output();

        // Clear cache
        interp.feed("clear_cache;");

        // After clearing, the cache should be empty
        REQUIRE(absl::StrContains(interp.get_output(), "Cleared"));

        // Variables should still be valid
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
    }
}

TEST_CASE("Preservation and Garbage Collection") {
    InterpTester interp;
    interp.feed("bvar x y z;");

    SECTION("Basic Preservation and Sweeping") {
        // Create some BDDs
        interp.feed("set a = x & y;");
        interp.feed("set b = x | z;");
        interp.feed("set c = a & b;");

        // Preserve one BDD and sweep
        interp.feed("preserve a;");
        interp.feed("sweep;");

        // 'a' should still be accessible, but 'b' and 'c' should be gone
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
        interp.feed("b;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));

        interp.feed("c;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }

    SECTION("Preserve Multiple BDDs") {
        interp.feed("set a = x & y;");
        interp.feed("set b = x | z;");
        interp.feed("set c = a & b;");

        // Preserve multiple BDDs and sweep
        interp.feed("preserve a b;");
        interp.feed("sweep;");

        // 'a' and 'b' should still be accessible, but 'c' should be gone
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
        REQUIRE(interp.expr_tree_repr("b") ==
                "x ? (TRUE) : (z ? (TRUE) : (FALSE))");

        interp.feed("c;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }
    SECTION("Preserve All BDDs") {
        interp.feed("set a = x & y;");
        interp.feed("set b = x | z;");
        interp.feed("set c = a & b;");

        // Preserve all BDDs and sweep
        interp.feed("preserve_all;");
        interp.feed("sweep;");

        // All BDDs should still be accessible
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
        REQUIRE(interp.expr_tree_repr("b") ==
                "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
        REQUIRE(interp.expr_tree_repr("c") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
    }

    SECTION("Unpreserve Specific BDDs") {
        interp.feed("set a = x & y;");
        interp.feed("set b = x | z;");
        interp.feed("set c = a & b;");

        // Preserve all and then unpreserve specific BDDs
        interp.feed("preserve_all;");
        interp.feed("unpreserve b;");
        interp.feed("sweep;");

        // 'a' and 'c' should still be accessible, but 'b' should be gone
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
        interp.feed("b;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
        REQUIRE(interp.expr_tree_repr("c") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
    }

    SECTION("Unpreserve Multiple BDDs") {
        interp.feed("set a = x & y;");
        interp.feed("set b = x | z;");
        interp.feed("set c = a & b;");

        // Preserve all and then unpreserve multiple BDDs
        interp.feed("preserve_all;");
        interp.feed("unpreserve a c;");
        interp.feed("sweep;");

        // Only 'b' should be accessible
        interp.feed("a;");

        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
        REQUIRE(interp.expr_tree_repr("b") ==
                "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
        interp.feed("c;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }

    SECTION("Unpreserve All BDDs") {
        interp.feed("set a = x & y;");
        interp.feed("set b = x | z;");
        interp.feed("set c = a & b;");

        // Preserve all, then unpreserve all, and sweep
        interp.feed("preserve_all;");
        interp.feed("unpreserve_all;");
        interp.feed("sweep;");

        // No BDDs should be accessible
        interp.feed("a;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));

        interp.feed("b;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));

        interp.feed("c;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }

    SECTION("Preserve After Unpreserving") {
        interp.feed("set a = x & y;");
        interp.feed("set b = x | z;");

        // Unpreserve all, then preserve one, and sweep
        interp.feed("preserve_all;");
        interp.feed("unpreserve_all;");
        interp.feed("preserve a;");
        interp.feed("sweep;");

        // Only 'a' should be accessible
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        interp.feed("b;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }
    SECTION("Create BDDs After Sweeping") {
        interp.feed("set a = x & y;");
        interp.feed("sweep;");  // This will remove 'a' since it's not preserved

        // Create new BDDs after sweeping
        interp.feed("set b = x | z;");

        // 'a' should be gone, but 'b' should be accessible
        interp.feed("a;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));

        REQUIRE(interp.expr_tree_repr("b") ==
                "x ? (TRUE) : (z ? (TRUE) : (FALSE))");
    }

    SECTION("Sweep Multiple Times") {
        interp.feed("set a = x & y;");
        interp.feed("preserve a;");
        interp.feed("sweep;");

        // Create more BDDs and sweep again
        interp.feed("set b = x | z;");
        interp.feed("sweep;");

        // 'a' should still be accessible, but 'b' should be gone
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");

        interp.feed("b;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }

    SECTION("Preserve Non-existent BDDs") {
        // Try to preserve a non-existent BDD
        interp.feed("preserve nonexistent;");
        REQUIRE(absl::StrContains(interp.get_output(), "Variable not found"));

        // Create a BDD after attempting to preserve a non-existent one
        interp.feed("set a = x & y;");
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
    }

    SECTION("Preserve Symbolic Variables") {
        // Try to preserve a symbolic variable
        interp.feed("preserve x;");
        REQUIRE(
            absl::StrContains(interp.get_output(), "Variable is not a BDD"));

        // Create and preserve a real BDD
        interp.feed("set a = x & y;");
        interp.feed("preserve a;");
        interp.feed("sweep;");

        // The BDD should still be accessible
        REQUIRE(interp.expr_tree_repr("a") ==
                "x ? (y ? (TRUE) : (FALSE)) : (FALSE)");
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
                int number = 0;
                return absl::SimpleAtoi(s, &number);
            });
        REQUIRE(numbers.size() > 0);
        int id = 0;
        [[maybe_unused]] bool success = absl::SimpleAtoi(numbers[0], &id);

        REQUIRE(interp.expr_tree_repr(std::format("z & {};", id)) ==
                "x ? (y ? (z ? (TRUE) : (FALSE)) : (FALSE)) : (FALSE)");
    }

    SECTION("Invalid ID Usage") {
        interp.feed("x & 100;");
        REQUIRE(absl::StrContains(interp.get_output(), "ExecutionException"));
    }
}