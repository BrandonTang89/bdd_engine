#pragma once
#include <sstream>

#include "../src/parser.h"
class LexerParserTester {
    std::ostringstream parser_error_stream{};
    std::ostringstream lexer_error_stream{};

   public:
    LexerParserTester() = default;

    // Test the parser with a given input string
    std::vector<stmt> feed(const std::string& input) {
        lex_result_t tokens = scan_to_tokens(input);
        if (!tokens.has_value()) {
            lexer_error_stream << tokens.error().what() << '\n';
            return {};
        }
        parse_result_t estmts = parse(*tokens);
        if (!estmts.has_value()) {
            for (const auto& error : estmts.error()) {
                parser_error_stream << error.what() << '\n';
            }
            return {};
        }
        return std::move(*estmts);
    }

    // Get the lexer error output
    std::string get_lexer_error() {
        std::string error = lexer_error_stream.str();
        lexer_error_stream.str("");  // Clear the error stream
        return error;
    }

    // Get the parser error output
    std::string get_parser_error() {
        std::string error = parser_error_stream.str();
        parser_error_stream.str("");  // Clear the error stream
        return error;
    }
};