#pragma once
#include <sstream>
#include "../src/parser.h"
class ParserTester {
    std::ostringstream parser_error_stream{};

   public:
    ParserTester() = default;

    // Test the parser with a given input string
    std::vector<stmt> feed(const std::string& input) {
        parse_result_t estmts = parse(input);
        if (!estmts.has_value()) {
            for (const auto& error : estmts.error()) {
                parser_error_stream << error.what() << '\n';
            }
            return {};
        }
        return std::move(*estmts);
    }

    // Get the parser error output
    std::string get_parser_error() {
        std::string error = parser_error_stream.str();
        parser_error_stream.str("");  // Clear the error stream
        return error;
    }
};