#include "repl.h"

#include <iostream>
#include <vector>

#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "colours.h"
#include "config.h"
#include "lexer.h"
#include "parser.h"

void evaluate(const std::string& user_input, Walker& walker) {
    const lex_result_t tokens = scan_to_tokens(user_input);
    if (!tokens.has_value()) {
        set_colour(std::cout, Colour::RED);
        std::cout << tokens.error().what() << '\n';
        set_colour(std::cout);
        return;
    }

    if constexpr (echo_input) {
        LOG(WARNING) << "Input: " << user_input << '\n';
    }

    if constexpr (print_tokens) {
        for (const auto& token : *tokens) {
            LOG(WARNING) << token.repr();
        }
    }

    auto estmt = parse(*tokens);
    std::vector<stmt> statements = {};

    if (!estmt.has_value()) {
        set_colour(std::cout, Colour::RED);
        for (const auto& error : estmt.error()) {
            std::cout << error.what() << '\n';
        }
        set_colour(std::cout);
        return;
    }

    walker.walk_statements(*estmt);

    std::cout << walker.get_output();
}

[[noreturn]] void repl(Walker& walker) {
    std::cout << "Binary Decision Diagram Engine" << '\n';

    while (true) {
        std::string input;
        output_with_colour(std::cout, Colour::PURPLE, ">> ");
        while (input.empty() || input.back() != ';') {
            std::string line;
            std::getline(std::cin, line);
            input += line;
            // Strip leading/trailing whitespace
            input = absl::StripAsciiWhitespace(input);
        }

        evaluate(input, walker);
    }
}