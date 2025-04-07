#include <iostream>
#include <string>
#include <vector>

#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "token.h"
#include "parser.h"
#include "walker.h"

// Evaulates a list of statements
void evaluate(const std::string& user_input, Walker& walker) {
    std::vector<Token> stream = scan_to_tokens(user_input);
    // LOG(INFO) << "Tokens: ";
    // for (const auto& token : stream) {
    //     LOG(INFO) << token.repr();
    // }

    std::vector<stmt> statements = parse(stream);
    for (const auto& statement: statements) {
        LOG(INFO) << stmt_repr(statement);
        walker.walk(statement);
    }
}


int main() {
    absl::InitializeLog();
    absl::SetStderrThreshold(absl::LogSeverity::kInfo);
    LOG(INFO) << "Application Started";
    std::cout << "Binary Decision Diagram Engine" << std::endl;

    // REPL
    Walker walker;
    while (true) {
        std::string input;
        std::cout << "> ";
        while (input.empty() || input.back() != ';') {
            std::string line;
            std::getline(std::cin, line);
            input += line;
        }

        std::cout << "Input: " << input << std::endl;
        evaluate(input, walker);
    }
}