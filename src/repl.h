#pragma once

#include <string>
#include "walker.h"

// Evaluates a list of statements
void evaluate(const std::string& user_input, Walker& walker);

// REPL function
[[noreturn]] void repl(Walker& walker);