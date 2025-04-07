#include "token.h"

std::vector<Token> scan_to_tokens(const std::string& source) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < source.size()) {
        char c = source[i];

        switch (c) {
            case '(':
                tokens.emplace_back(Token::Type::LEFT_PAREN, "(");
                break;
            case ')':
                tokens.emplace_back(Token::Type::RIGHT_PAREN, ")");
                break;
            case '&':
                tokens.emplace_back(Token::Type::LAND, "&");
                break;
            case '|':
                tokens.emplace_back(Token::Type::LOR, "|");
                break;
            case ';':
                tokens.emplace_back(Token::Type::SEMICOLON, ";");
                break;
            case '=':
                if (i + 1 < source.size() && source[i + 1] == '=') {
                    tokens.back() = Token(Token::Type::EQUAL_EQUAL, "==");
                    ++i;  // Skip the next '='
                } else {
                    tokens.emplace_back(Token::Type::EQUAL, "=");
                }
                break;
            case '!':
                if (i + 1 < source.size() && source[i + 1] == '=') {
                    tokens.emplace_back(Token::Type::BANG_EQUAL, "!=");
                    ++i;  // Skip the next '='
                } else {
                    tokens.emplace_back(Token::Type::BANG, "!");
                }
                break;
            default:
                if (isalpha(c)) {
                    std::string identifier;
                    while (isalnum(c)) {
                        identifier += c;
                        c = source[++i];
                    }
                    if (identifier == "bvar") {
                        tokens.emplace_back(Token::Type::BVAR, identifier);
                    } else if (identifier == "true") {
                        tokens.emplace_back(Token::Type::TRUE, identifier);
                    } else if (identifier == "false") {
                        tokens.emplace_back(Token::Type::FALSE, identifier);
                    } else if (identifier == "display") {
                        tokens.emplace_back(Token::Type::DISPLAY, identifier);
                    } else {
                        tokens.emplace_back(Token::Type::IDENTIFIER, identifier);
                    }
                } else {
                    ++i;  // Skip unrecognized characters
                }
                continue;  // Skip the increment at the end of the loop
        }
        ++i;  // Move to the next character
    }

    return tokens;
}