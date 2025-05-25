
#include <variant>
#include <vector>

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "catch2/catch_test_macros.hpp"
#include "parser_tester.h"

TEST_CASE("Parse Valid") {
    std::string input = R"(
        bvar x y z;
        set a = x & y;
        set b = a | z;
        set c = exists x (a & b);
        display_tree a;
    )";

    std::vector<stmt> statements = LexerParserTester().feed(input);
    REQUIRE(statements.size() == 5);
    REQUIRE(std::holds_alternative<decl_stmt>(statements[0]));
    REQUIRE(std::holds_alternative<assign_stmt>(statements[1]));
    REQUIRE(std::holds_alternative<assign_stmt>(statements[2]));
    REQUIRE(std::holds_alternative<assign_stmt>(statements[3]));
    REQUIRE(std::holds_alternative<func_call_stmt>(statements[4]));
}

TEST_CASE("Invalid Declaration") {
    LexerParserTester parser_tester;
    SECTION("Declaration with commas") {
        std::string input = R"(
            bvar x, y, z;
        )";
        parser_tester.feed(input);
        std::string error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));
    }

    SECTION("Missing Parts of Substitution") {
        std::string input = R"(
            bvar x y z;
            sub {x: y};
        )";
        parser_tester.feed(input);
        std::string error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));

        input = R"(
            sub {x: y, y:};
        )";
        parser_tester.feed(input);
        error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));

        input = R"(
            sub x;
        )";
        parser_tester.feed(input);
        error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));

        input = R"(
            sub {: x;
        )";
        parser_tester.feed(input);
        error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));

        input = R"(
            sub {y x} y;
        )";
        parser_tester.feed(input);
        error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));
    }

    SECTION("Assignment without =") {
        std::string input = R"(
            set a true;
        )";
        parser_tester.feed(input);
        std::string error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));
    }

    SECTION("Multiple Equality") {
        parser_tester.feed("set a = x == y == z;");
        std::string error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));
    }

    SECTION("Mixing Equality and Inequality") {
        parser_tester.feed("set a = x == y != z;");
        std::string error = parser_tester.get_parser_error();
        REQUIRE(absl::StrContains(error, "ParserException"));
    }

    SECTION("Multiple errors detected at once") {
        std::string input = R"(
            bvar x, y, z;
            set a x y;
        )";
        parser_tester.feed(input);
        std::string error = parser_tester.get_parser_error();
        std::vector<std::string> error_lines =
            absl::StrSplit(error, '\n', absl::SkipEmpty());

        REQUIRE(error_lines.size() == 2);
        for (const auto& line : error_lines) {
            REQUIRE(absl::StrContains(line, "ParserException"));
        }
    }
}