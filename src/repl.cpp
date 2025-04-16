#include "repl.h"

#include <iostream>
#include <vector>

#include "absl/log/log.h"
#include "config.h"
#include "parser.h"
#include "token.h"

void evaluate(const std::string& user_input, Walker& walker) {
    std::vector<Token> stream = scan_to_tokens(user_input);

    if constexpr (echo_input) {
        LOG(WARNING) << "Input: " << user_input << '\n';
    }

    if constexpr (print_tokens) {
        for (const auto& token : stream) {
            LOG(WARNING) << token.repr();
        }
    }

    std::vector<stmt> statements = parse(stream);
    for (const auto& statement : statements) {
        if constexpr (print_ast) LOG(WARNING) << stmt_repr(statement);
        walker.walk(statement);
        std::cout << walker.get_output();
    }
}

void repl(Walker& walker) {
    std::cout << "Binary Decision Diagram Engine" << '\n';
    while (true) {
        std::string input;
        std::cout << "> ";
        while (input.empty() || input.back() != ';') {
            std::string line;
            std::getline(std::cin, line);
            input += line;
        }

        evaluate(input, walker);
    }
}