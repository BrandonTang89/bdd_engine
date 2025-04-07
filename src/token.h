#pragma once

#include <string>
#include <vector>

struct Token {
    enum class Type {
        // Single-character Tokens
        LEFT_PAREN,
        RIGHT_PAREN,
        LAND,
        LOR,
        SEMICOLON,

        // Single or Double Character Tokens
        EQUAL,
        EQUAL_EQUAL,
        BANG,
        BANG_EQUAL,

        // Keywords
        IDENTIFIER,
        BVAR,
        TRUE,
        FALSE,

        // Special Keywords (since we don't do functions)
        DISPLAY,

    };

    Type type;
    std::string lexeme;  // the text of the token

    Token(Type type, const std::string& lexeme) : type(type), lexeme(lexeme) {}

    std::string repr() const {
        return "Token(" + std::to_string(static_cast<int>(type)) + ", " + lexeme + ")";
    }
};

std::vector<Token> scan_to_tokens(const std::string& source); 