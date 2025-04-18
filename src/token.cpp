#include "token.h"

#include <unordered_map>

const std::unordered_map<std::string, Token::Type> keyword_map = {
    {"bvar", Token::Type::BVAR},
    {"set", Token::Type::SET},
    {"true", Token::Type::TRUE},
    {"false", Token::Type::FALSE},
    {"display_tree", Token::Type::TREE_DISPLAY},
    {"display_graph", Token::Type::GRAPH_DISPLAY},
    {"is_sat", Token::Type::IS_SAT},
    {"source", Token::Type::SOURCE},
};

constexpr bool is_lexeme_char(char c) { return isalpha(c) || isdigit(c) || c == '_' || c == '.'; }

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
            case ',':
                tokens.emplace_back(Token::Type::COMMA, ",");
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
                    while (is_lexeme_char(c)) {
                        identifier += c;
                        c = source[++i];
                    }
                    if (keyword_map.find(identifier) != keyword_map.end()) {
                        tokens.emplace_back(keyword_map.at(identifier), identifier);
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