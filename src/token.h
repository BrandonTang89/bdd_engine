#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Token {
    enum class Type : std::uint8_t {
        // Single-character Tokens
        LEFT_PAREN,
        RIGHT_PAREN,
        LAND,
        LOR,
        SEMICOLON,
        COMMA,

        // Single or Double Character Tokens
        EQUAL,
        EQUAL_EQUAL,
        BANG,
        BANG_EQUAL,
        ARROW,
        MINUS, // currently unused

        // Multiple character tokens
        IDENTIFIER,

        // Keywords
        BVAR,
        SET,
        TRUE,
        FALSE,

        // Special Keywords for functions
        TREE_DISPLAY,
        GRAPH_DISPLAY,
        IS_SAT,
        SOURCE,

        // Special Keywords for quantifiers
        EXISTS,
        FORALL,
    };

    Type type;
    std::string lexeme;  // the text of the token

    Token(const Type type, const std::string& lexeme) : type(type), lexeme(lexeme) {}

    std::string repr() const {
        return "Token(" + std::to_string(static_cast<int>(type)) + ", " + lexeme + ")";
    }
};

std::vector<Token> scan_to_tokens(const std::string& source);