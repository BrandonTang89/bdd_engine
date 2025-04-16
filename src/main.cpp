#include <iostream>
#include <optional>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "repl.h"
#include "walker.h"

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