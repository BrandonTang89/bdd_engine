#pragma once
#include <cassert>
#include <cstdint>

struct token {
    enum class Type : std::uint8_t {
        // Single-character Tokens
        LEFT_PAREN,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        LAND,
        LOR,
        SEMICOLON,
        COLON,
        COMMA,

        // Single or Double Character Tokens
        EQUAL,
        EQUAL_EQUAL,
        BANG,
        BANG_EQUAL,
        ARROW,
        MINUS,  // currently unused

        // Multiple character tokens
        IDENTIFIER,
        ID,

        // Keywords
        BVAR,
        SET,
        TRUE,
        FALSE,

        // Special Keywords
        SUBSTITUTE,

        // Special Keywords for functions
        TREE_DISPLAY,
        GRAPH_DISPLAY,
        IS_SAT,
        SOURCE,

        // Special Keywords for quantifiers
        EXISTS,
        FORALL,

        // Special Keywords for memory management
        CLEAR_CACHE,
        PRESERVE,
        UNPRESERVE,
        PRESERVE_ALL,
        UNPRESERVE_ALL,
        SWEEP,
    };

    Type type;
    std::string lexeme;                     // the text of the token
    std::optional<uint32_t> token_value{};  // optional value for number tokens

    token(const Type type, const std::string& lexeme,
          const std::optional<int> value = std::nullopt)
        : type(type), lexeme(lexeme), token_value(value) {
        assert(type != Type::ID || value.has_value());
    }

    std::string repr() const {
        return "Token(" + std::to_string(static_cast<int>(type)) + ", " +
               lexeme +
               (token_value.has_value()
                    ? ", " + std::to_string(token_value.value())
                    : "") +
               ")";
    }
};