#pragma once

#include <format>
#include <source_location>
#include "Token.h"

// Lexer Exception Type
class LexerException final : public std::exception {
    std::string message;
    std::string function_name;
    std::source_location location;

    mutable std::string formatted_message;

   public:
    LexerException(
        const std::string& msg, const std::string_view fn_name,
        const std::source_location& loc = std::source_location::current())
        : message(msg), function_name(fn_name), location(loc) {}

    const char* what() const noexcept override {
        formatted_message =
            std::format("LexerException: [{}:{}] {}", function_name,
                        location.line(), message);

        return formatted_message.c_str();
    }
};

// Parser Exception Type
class ParserException final : public std::exception {
    std::string message;
    Token next_token;
    std::string function_name;
    std::source_location location;

    mutable std::string formatted_message;

   public:
    ParserException(
        const std::string& msg, const Token& nxt_token,
        const std::string_view fn_name,
        const std::source_location& loc = std::source_location::current())
        : message(msg),
          next_token(nxt_token),
          function_name(fn_name),
          location(loc) {}

    const char* what() const noexcept override {
        formatted_message = std::format(
            "ParserException: [{}:{}] {} but next token is {}", function_name,
            location.line(), message, next_token.lexeme);

        return formatted_message.c_str();
    }
};

// Execution Exception Type
class ExecutionException final : public std::exception {
    std::string message;
    std::string function_name;
    std::source_location location;

    mutable std::string formatted_message;

   public:
    ExecutionException(
        const std::string& msg, const std::string_view fn_name,
        const std::source_location& loc = std::source_location::current())
        : message(msg), function_name(fn_name), location(loc) {}

    const char* what() const noexcept override {
        formatted_message =
            std::format("ExecutionException: [{}:{}] {}", function_name,
                        location.line(), message);

        return formatted_message.c_str();
    }
};