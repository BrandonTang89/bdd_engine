#include "lexer.h"

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
    {"exists", Token::Type::EXISTS},
    {"forall", Token::Type::FORALL},
};

constexpr bool is_lexeme_char(const char c) {
    return isalpha(c) || isdigit(c) || c == '_' || c == '.';
}

lex_result_t scan_to_tokens(const std::string& source) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < source.size()) {
        switch (char c = source[i]) {
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
                    tokens.emplace_back(Token::Type::EQUAL_EQUAL, "==");
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
            case '-':
                if (i + 1 < source.size() && source[i + 1] == '>') {
                    tokens.emplace_back(Token::Type::ARROW, "->");
                    ++i;  // Skip the next '>'

                } else {
                    tokens.emplace_back(Token::Type::MINUS, "-");
                }
                break;
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                // Ignore whitespace
                break;
            default:
                if (isalpha(c)) {
                    std::string identifier;
                    while (is_lexeme_char(c)) {
                        identifier += c;
                        c = source[++i];
                    }
                    if (keyword_map.contains(identifier)) {
                        tokens.emplace_back(keyword_map.at(identifier),
                                            identifier);
                    } else {
                        tokens.emplace_back(Token::Type::IDENTIFIER,
                                            std::move(identifier));
                    }
                } else if (isdigit(c)) {
                    uint32_t number = 0;
                    while (isdigit(c)) {
                        number = number * 10 + (c - '0');
                        c = source[++i];
                    }
                    tokens.emplace_back(Token::Type::ID,
                                        std::to_string(number), number);
                } else {
                    return std::unexpected<LexerException>(LexerException(
                        "Unexpected character: " + std::string(1, c),
                        __func__));
                }
                continue;  // Skip the increment at the end of the loop
        }
        ++i;  // Move to the next character
    }
    return tokens;
}