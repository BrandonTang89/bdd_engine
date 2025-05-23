#pragma once

#include <expected>
#include <string>
#include <vector>
#include "engine_exceptions.h"

using lex_result_t = std::expected<std::vector<Token>, LexerException>;
lex_result_t scan_to_tokens(const std::string& source);