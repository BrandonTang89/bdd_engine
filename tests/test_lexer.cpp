#include <variant>
#include <vector>

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "catch2/catch_test_macros.hpp"
#include "parser_tester.h"

TEST_CASE("Lex Valid") {
    std::string input = R"(
        bvar x y z;
        set a = x & y;
        set b = a | z;
        set c = exists x (a & b);
        display_tree a;
    )";

    auto lexer_parser_tester = LexerParserTester();
    lexer_parser_tester.feed(input);

    REQUIRE(lexer_parser_tester.get_lexer_error().empty());
}

TEST_CASE("Lex Invalid") {
    LexerParserTester lexer_parser_tester;
    SECTION("Unknown Symbols") {
        std::string input = R"(
            x + y;
        )";
        lexer_parser_tester.feed(input);
        std::string error = lexer_parser_tester.get_lexer_error();
        REQUIRE(absl::StrContains(error, "LexerException"));
    }
}