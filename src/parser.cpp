// Recursive Descent Parser
#include "parser.h"

#include <optional>

#include "absl/log/log.h"
#include "ast.h"
#include "token.h"

// Parses a vector of tokens into an AST
std::optional<std::vector<stmt>> parse(const std::vector<Token>& tokens,
                                       std::ostream& error_stream) {
    // Tokenize the input string
    const_iter it = tokens.begin();
    std::vector<stmt> statements;
    bool has_errors = false;
    while (it != tokens.end()) {
        try {
            statements.emplace_back(parse_statement(it));
        } catch (const ParserException& e) {
            has_errors = true;
            error_stream << e.what() << "\n";
            while (it != tokens.end() && it->type != Token::Type::SEMICOLON) {
                ++it;  // Skip to the next statement
            }
            if (it != tokens.end()) {
                ++it;  // Skip the ';' token
            }
        } catch (const std::exception& e) {
            LOG(ERROR) << "Unhandled Parsing Error: " << e.what();
            break;
        }
    }
    if (has_errors) {
        return std::nullopt;  // Return nullopt if there were parsing errors
    }
    return statements;  // Return the parsed statements
}
// Combines the lexer and the parser
std::optional<std::vector<stmt>> parse(const std::string& input, std::ostream& error_stream) {
    // Tokenize the input string
    auto tokens = scan_to_tokens(input);
    return parse(tokens, error_stream);
}

// // Parses a single statement
stmt parse_statement(const_iter& it) {
    switch (it->type) {
        case Token::Type::BVAR:
            return parse_decl(it);
        case Token::Type::SET:
            return parse_assign(it);
        case Token::Type::TREE_DISPLAY:
        case Token::Type::GRAPH_DISPLAY:
        case Token::Type::IS_SAT:
        case Token::Type::SOURCE:
            return parse_func_call(it);
        default:  // assume expr statement
            return parse_expr_stmt(it);
    }
}

// Parse a Declaration Statement
decl_stmt parse_decl(const_iter& it) {
    decl_stmt decl;
    ++it;  // Skip the 'bvar' token
    if (it->type != Token::Type::IDENTIFIER) {
        throw ParserException("Expected identifier after 'bvar'", *it, __func__);
    }

    while (it->type == Token::Type::IDENTIFIER) {
        decl.identifiers.push_back(*it);
        ++it;
    }

    if (it->type == Token::Type::SEMICOLON) {
        ++it;  // Skip the ';'
        return decl;
    } else {
        throw ParserException("Expected ';' after identifiers", *it, __func__);
    }
}

// Parse an Assignment Statement
assign_stmt parse_assign(const_iter& it) {
    assign_stmt assign;
    it++;  // skip the 'set' token
    assign.target = parse_ident(it);

    if (it->type != Token::Type::EQUAL) {
        throw ParserException("Expected '=' after identifier", *it, __func__);
    }
    ++it;  // Skip the '=' token

    assign.value = parse_expr(it);
    if (it->type != Token::Type::SEMICOLON) {
        throw ParserException("Expected ';' after assignment", *it, __func__);
    }
    ++it;  // Skip the ';' token
    return assign;
}

// Parse a Function Call Statement
func_call_stmt parse_func_call(const_iter& it) {
    func_call_stmt call{*it, {}};
    ++it;  // Skip the function name token

    while (true) {
        call.arguments.push_back(parse_expr(it));

        if (it->type == Token::Type::SEMICOLON) break;
    }
    ++it;  // Skip the ';' token
    return call;
}

// Parse an Expression Statement
expr_stmt parse_expr_stmt(const_iter& it) {
    expr_stmt expr;
    expr.expression = parse_expr(it);

    if (it->type != Token::Type::SEMICOLON) {
        throw ParserException("Expected ';' after expression", *it, __func__);
    }

    ++it;  // Skip the ';' token
    return expr;
}

// Parse an Expression Statement
std::unique_ptr<expr> parse_expr(const_iter& it) {
    auto conjuct{parse_conjuct(it)};
    while (it->type == Token::Type::LOR) {
        auto op = *it;
        ++it;  // Skip the '|' token
        auto right = parse_conjuct(it);
        conjuct = std::make_unique<expr>(bin_expr{std::move(conjuct), std::move(right), op});
    }

    return conjuct;
}

// Parse a Conjunct Expression
std::unique_ptr<expr> parse_conjuct(const_iter& it) {
    auto unary_expr{parse_quantifier(it)};
    while (it->type == Token::Type::LAND) {
        auto op = *it;
        ++it;  // Skip the '&' token
        auto right = parse_quantifier(it);
        unary_expr = std::make_unique<expr>(bin_expr{std::move(unary_expr), std::move(right), op});
    }
    return unary_expr;
}

// Parse a Quantifier Expression
std::unique_ptr<expr> parse_quantifier(const_iter& it) {
    // exists '(' IDENTIFIER+ ')' unary
    // forall '(' IDENTIFIER+ ')' unary

    if (it->type == Token::Type::EXISTS || it->type == Token::Type::FORALL) {
        auto quantifier = *it;
        ++it;  // Skip the quantifier token
        std::vector<Token> bound_vars;

        switch (it->type) {
            case Token::Type::IDENTIFIER:
                bound_vars.push_back(*it);
                ++it;  // Skip the identifier token
                break;
            case Token::Type::LEFT_PAREN:
                ++it;  // Skip the '(' token

                while (it->type == Token::Type::IDENTIFIER) {
                    bound_vars.push_back(*it);
                    ++it;  // Skip the identifier token
                }

                if (it->type != Token::Type::RIGHT_PAREN) {
                    throw ParserException("Expected ')' after bound variables", *it, __func__);
                }
                ++it;  // Skip the ')' token
                break;
            default:
                throw ParserException("[Expected '(' or identifier after quantifier", *it,
                                      __func__);
        }

        auto body = parse_unary(it);
        return std::make_unique<expr>(
            quantifier_expr{quantifier, std::move(bound_vars), std::move(body)});
    } else {
        // no quantifier
        return parse_unary(it);
    }
}

// Parse a Unary Expression
std::unique_ptr<expr> parse_unary(const_iter& it) {
    if (it->type == Token::Type::BANG) {
        auto op = *it;
        ++it;  // Skip the '!' token
        auto operand = parse_unary(it);
        return std::make_unique<expr>(unary_expr{std::move(operand), op});
    }
    return parse_primary(it);
}

// Parse a Primary Expression
std::unique_ptr<expr> parse_primary(const_iter& it) {
    if (it->type == Token::Type::IDENTIFIER) {
        auto id = parse_ident(it);
        return std::make_unique<expr>(*id);
    } else if (it->type == Token::Type::TRUE || it->type == Token::Type::FALSE) {
        auto lit = parse_literal(it);
        return std::make_unique<expr>(*lit);
    } else if (it->type == Token::Type::LEFT_PAREN) {
        ++it;  // Skip the '(' token
        auto expr = parse_expr(it);
        ++it;  // Skip the ')' token
        return expr;
    }
    throw ParserException("Expected identifier, literal, or '('", *it, __func__);
}

// Parse an Identifier
std::unique_ptr<identifier> parse_ident(const_iter& it) {
    if (it->type != Token::Type::IDENTIFIER) {
        throw ParserException("Expected identifier", *it, __func__);
    }
    auto id = std::make_unique<identifier>(*it);
    ++it;  // Skip the identifier token
    return id;
}

// Parse a Literal
std::unique_ptr<literal> parse_literal(const_iter& it) {
    if (it->type != Token::Type::TRUE && it->type != Token::Type::FALSE) {
        throw ParserException("Expected literal", *it, __func__);
    }
    auto lit = std::make_unique<literal>(*it);
    ++it;  // Skip the literal token
    return lit;
}
