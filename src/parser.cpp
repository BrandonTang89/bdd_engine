// Recursive Descent Parser
#include "parser.h"

#include "absl/log/log.h"
#include "ast.h"
#include "token.h"

// Parses a vector of tokens into an AST
std::vector<stmt> parse(const std::vector<Token>& tokens) {
    auto it = tokens.begin();
    std::vector<stmt> statements;
    while (it != tokens.end()) {
        try {
            statements.push_back(parse_statement(it));

        } catch (const std::exception& e) {
            LOG(ERROR) << "Error parsing statement: " << e.what();
            break;
        }
    }
    return statements;
}

// Combines the lexer and the parser
std::vector<stmt> parse(const std::string& input) {
    // Tokenize the input string
    auto tokens = scan_to_tokens(input);
    return parse(tokens);
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
            return parse_func_call(it);
        default:  // assume expr statement
            return parse_expr_stmt(it);
            throw std::runtime_error("Unknown statement type");
    }
}

// Parse a Declaration Statement
decl_stmt parse_decl(const_iter& it) {
    decl_stmt decl;
    ++it;  // Skip the 'bvar' token
    while (true) {
        assert(it->type == Token::Type::IDENTIFIER);
        decl.identifiers.push_back(*it);
        ++it;

        if (it->type == Token::Type::SEMICOLON) {
            ++it;  // Skip the ';'
            break;
        } else if (it->type == Token::Type::COMMA) {
            ++it;  // Skip the ','
        } else {
            LOG(ERROR) << "Expected ',' or ';' after identifier";
            throw std::runtime_error("Expected ',' or ';' after identifier");
        }
    }
    return decl;
}

// Parse an Assignment Statement
assign_stmt parse_assign(const_iter& it) {
    assign_stmt assign;
    it++;  // skip the 'set' token
    assign.target = parse_ident(it);
    assert(it->type == Token::Type::EQUAL);
    ++it;  // Skip the '=' token

    assign.value = parse_expr(it);

    assert(it->type == Token::Type::SEMICOLON);
    ++it;  // Skip the ';' token
    return assign;
}

// Parse a Function Call Statement
func_call_stmt parse_func_call(const_iter& it) {
    func_call_stmt call{*it, {}};
    ++it;  // Skip the function name token
    assert(it->type == Token::Type::LEFT_PAREN);

    while (true) {
        call.arguments.push_back(parse_expr(it));
        assert(it->type == Token::Type::COMMA || it->type == Token::Type::RIGHT_PAREN);

        if (it->type == Token::Type::SEMICOLON) break;
    }
    ++it;  // Skip the ';' token
    return call;
}

// Parse an Expression Statement
expr_stmt parse_expr_stmt(const_iter& it) {
    expr_stmt expr;
    expr.expression = parse_expr(it);
    assert(it->type == Token::Type::SEMICOLON);
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
    auto unary_expr{parse_unary(it)};
    while (it->type == Token::Type::LAND) {
        auto op = *it;
        ++it;  // Skip the '&' token
        auto right = parse_unary(it);
        unary_expr = std::make_unique<expr>(bin_expr{std::move(unary_expr), std::move(right), op});
    }
    return unary_expr;
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
    LOG(ERROR) << "Expected identifier or literal";
    throw std::runtime_error("Expected identifier or literal");
}

// Parse an Identifier
std::unique_ptr<identifier> parse_ident(const_iter& it) {
    assert(it->type == Token::Type::IDENTIFIER);
    auto id = std::make_unique<identifier>(*it);
    ++it;  // Skip the identifier token
    return id;
}

// Parse a Literal
std::unique_ptr<literal> parse_literal(const_iter& it) {
    assert(it->type == Token::Type::TRUE || it->type == Token::Type::FALSE);
    auto lit = std::make_unique<literal>(*it);
    ++it;  // Skip the literal token
    return lit;
}
