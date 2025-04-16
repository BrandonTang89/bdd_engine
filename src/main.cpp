#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "parser.h"
#include "token.h"
#include "walker.h"

constexpr bool echo_input = false;    // Set to true to echo input
constexpr bool print_tokens = false;  // Set to true to print tokens
constexpr bool print_ast = true;      // Set to true to print AST

// Evaluates a list of statements
static void evaluate(const std::string& user_input, Walker& walker) {
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

static void repl(Walker& walker) {
    // REPL
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

ABSL_FLAG(std::optional<std::string>, source, std::nullopt, "Input script to execute.");

int main(int argc, char* argv[]) {
#ifndef NDEBUG
    std::cout << ("Debug configuration!\n");
#endif

    // Set Up
    absl::SetProgramUsageMessage("Usage: " + std::string(*argv) +
                                 " [--source <input_file>] [--help] [--version]");
    absl::InitializeLog();
    absl::SetStderrThreshold(absl::LogSeverity::kWarning);  // set logging
    absl::ParseCommandLine(argc, argv);

    // Start of Program
    Walker walker;

    std::optional<std::string> source = absl::GetFlag(FLAGS_source);
    if (source.has_value()) {
        const std::string& input = source.value();
        std::string user_input = "source " + input + ";";
        evaluate(user_input, walker);
        std::cout << walker.get_output();
    } else {
        repl(walker);
    }
}