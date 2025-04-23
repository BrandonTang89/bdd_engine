#pragma once
#include <absl/base/log_severity.h>

constexpr bool echo_input = false;    // Set to true to echo input
constexpr bool print_tokens = false;  // Set to true to print tokens
constexpr bool print_ast = false;     // Set to true to print AST
constexpr absl::LogSeverity warning_level = absl::LogSeverity::kWarning;