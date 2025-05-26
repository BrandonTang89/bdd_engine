#include "lexer.h"

#include <unordered_map>

const std::unordered_map<std::string, token::Type> keyword_map = {
    {"bvar", token::Type::BVAR},
    {"set", token::Type::SET},
    {"true", token::Type::TRUE},
    {"false", token::Type::FALSE},
    {"display_tree", token::Type::TREE_DISPLAY},
    {"display_graph", token::Type::GRAPH_DISPLAY},
    {"is_sat", token::Type::IS_SAT},
    {"source", token::Type::SOURCE},
    {"sub", token::Type::SUBSTITUTE},
    {"exists", token::Type::EXISTS},
    {"forall", token::Type::FORALL},
    {"clear_cache", token::Type::CLEAR_CACHE},
    {"preserve", token::Type::PRESERVE},
    {"preserve_all", token::Type::PRESERVE_ALL},
    {"unpreserve", token::Type::UNPRESERVE},
    {"unpreserve_all", token::Type::UNPRESERVE_ALL},
    {"sweep", token::Type::SWEEP},
};

constexpr bool is_lexeme_char(const char c) {
    return isalpha(c) || isdigit(c) || c == '_' || c == '.';
}

lex_result_t scan_to_tokens(const std::string& source) {
    std::vector<token> tokens;
    size_t i = 0;

    while (i < source.size()) {
        switch (char c = source[i]) {
            case '(':
                tokens.emplace_back(token::Type::LEFT_PAREN, "(");
                break;
            case ')':
                tokens.emplace_back(token::Type::RIGHT_PAREN, ")");
                break;
            case '{':
                tokens.emplace_back(token::Type::LEFT_BRACE, "{");
                break;
            case '}':
                tokens.emplace_back(token::Type::RIGHT_BRACE, "}");
                break;
            case '&':
                tokens.emplace_back(token::Type::LAND, "&");
                break;
            case '|':
                tokens.emplace_back(token::Type::LOR, "|");
                break;
            case ';':
                tokens.emplace_back(token::Type::SEMICOLON, ";");
                break;
            case ':':
                tokens.emplace_back(token::Type::COLON, ":");
                break;
            case ',':
                tokens.emplace_back(token::Type::COMMA, ",");
                break;
            case '=':
                if (i + 1 < source.size() && source[i + 1] == '=') {
                    tokens.emplace_back(token::Type::EQUAL_EQUAL, "==");
                    ++i;  // Skip the next '='
                } else {
                    tokens.emplace_back(token::Type::EQUAL, "=");
                }
                break;
            case '!':
                if (i + 1 < source.size() && source[i + 1] == '=') {
                    tokens.emplace_back(token::Type::BANG_EQUAL, "!=");
                    ++i;  // Skip the next '='
                } else {
                    tokens.emplace_back(token::Type::BANG, "!");
                }
                break;
            case '-':
                if (i + 1 < source.size() && source[i + 1] == '>') {
                    tokens.emplace_back(token::Type::ARROW, "->");
                    ++i;  // Skip the next '>'

                } else {
                    tokens.emplace_back(token::Type::MINUS, "-");
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
                        tokens.emplace_back(token::Type::IDENTIFIER,
                                            std::move(identifier));
                    }
                } else if (isdigit(c)) {
                    uint32_t number = 0;
                    while (isdigit(c)) {
                        number = number * 10 + (c - '0');
                        c = source[++i];
                    }
                    tokens.emplace_back(token::Type::ID, std::to_string(number),
                                        number);
                } else {
                    return std::unexpected(LexerException(
                        "Unexpected character: " + std::string(1, c),
                        __func__));
                }
                continue;  // Skip the increment at the end of the loop
        }
        ++i;  // Move to the next character
    }
    return tokens;
}