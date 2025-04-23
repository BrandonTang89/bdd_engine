#include <sstream>

#include "../src/parser.h"
class ParserTester {
   public:
    std::ostringstream parser_error_stream{};

    ParserTester() = default;

    // Test the parser with a given input string
    std::vector<stmt> feed(const std::string& input) {
        auto tokens = scan_to_tokens(input);
        auto statements =
            parse(tokens, parser_error_stream)
                .or_else([]() -> std::optional<std::vector<stmt>> { return std::vector<stmt>(); })
                .value();
        return statements;
    }

    // Get the parser error output
    std::string get_parser_error() {
        std::string error = parser_error_stream.str();
        parser_error_stream.str("");  // Clear the error stream
        return error;
    }
};