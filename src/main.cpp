#include <iostream>
#include <string>
#include <vector>

#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "token.h"

// Evaulates a list of statements
void evaluate(const std::string& statements) {
    std::vector<Token> stream = scan_to_tokens(statements);
    LOG(INFO) << "Tokens: ";
    for (const auto& token : stream) {
        LOG(INFO) << token.repr();
    }
}
int main() {
    absl::InitializeLog();
    absl::SetStderrThreshold(absl::LogSeverity::kInfo);
    LOG(INFO) << "Application Started";
    std::cout << "Binary Decision Diagram Engine" << std::endl;

    // REPL
    while (true) {
        std::string input;
        std::cout << "> ";
        while (input.empty() || input.back() != ';') {
            std::string line;
            std::getline(std::cin, line);
            input += line;
        }

        std::cout << "Input: " << input << std::endl;
        evaluate(input);
    }
}