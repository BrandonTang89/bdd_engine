// Recursive Descent Parser
#include "parser.h"

#include "absl/log/log.h"
#include "ast.h"
#include "token.h"

// Parses a vector of tokens into an AST
parse_result_t parse(const std::vector<Token>& tokens) {
    // Tokenize the input string
    auto sp = std::span(tokens);
    std::vector<stmt> statements;
    std::vector<ParserException> errors;
    bool has_errors = false;
    while (!sp.empty()) {
        try {
            statements.emplace_back(parse_statement(sp));
        } catch (const ParserException& e) {
            has_errors = true;
            errors.push_back(e);
            while (!sp.empty() && sp.front().type != Token::Type::SEMICOLON) {
                sp = sp.subspan(1);  // Skip to the next statement
            }
            if (!sp.empty()) {
                sp = sp.subspan(1);  // Skip the ';' token
            }
        } catch (const std::exception& e) {
            LOG(ERROR) << "Unhandled Parsing Error: " << e.what();
            break;
        }
    }
    if (has_errors) {
        return std::unexpected(errors);  // Return the errors if any occurred
    }
    return statements;  // Return the parsed statements
}
// Combines the lexer and the parser
parse_result_t parse(const std::string& input) {
    // Tokenize the input string
    const auto tokens = scan_to_tokens(input);
    return parse(tokens);
}

// // Parses a single statement
// stmt parse_statement(const_span& sp) {
//     switch (sp.front().type) {
//         case Token::Type::BVAR:
//             return parse_decl(sp);
//         case Token::Type::SET:
//             return parse_assign(sp);
//         case Token::Type::TREE_DISPLAY:
//         case Token::Type::GRAPH_DISPLAY:
//         case Token::Type::IS_SAT:
//         case Token::Type::SOURCE:
//             return parse_func_call(sp);
//         default:  // assume expr statement
//             return parse_expr_stmt(sp);
//     }
// }

stmt parse_statement(const_span& sp) {
    switch (sp.front().type) {
        case Token::Type::BVAR:
            return parse_decl(sp);
        case Token::Type::SET:
            return parse_assign(sp);
        case Token::Type::TREE_DISPLAY:
        case Token::Type::GRAPH_DISPLAY:
        case Token::Type::IS_SAT:
        case Token::Type::SOURCE:
            return parse_func_call(sp);
        default:  // assume expr statement
            return parse_expr_stmt(sp);
    }
}
// Parse a Declaration Statement
decl_stmt parse_decl(const_span& sp) {
    decl_stmt decl;
    sp = sp.subspan(1);  // Move the iterator forward
    if (sp.front().type != Token::Type::IDENTIFIER) {
        throw ParserException("Expected identifier after 'bvar'", sp.front(),
                              __func__);
    }

    while (sp.front().type == Token::Type::IDENTIFIER) {
        decl.identifiers.push_back(sp.front());
        sp = sp.subspan(1);
    }

    if (sp.front().type == Token::Type::SEMICOLON) {
        sp = sp.subspan(1);  // Skip the ';'
        return decl;
    } else {
        throw ParserException("Expected ';' after identifiers", sp.front(),
                              __func__);
    }
}

// Parse an Assignment Statement
assign_stmt parse_assign(const_span& sp) {
    assign_stmt assign;
    sp = sp.subspan(1);  // skip the 'set' token
    assign.target = parse_ident(sp);

    if (sp.front().type != Token::Type::EQUAL) {
        throw ParserException("Expected '=' after identifier", sp.front(),
                              __func__);
    }
    sp = sp.subspan(1);  // Skip the '=' token

    assign.value = parse_expr(sp);
    if (sp.front().type != Token::Type::SEMICOLON) {
        throw ParserException("Expected ';' after assignment", sp.front(),
                              __func__);
    }
    sp = sp.subspan(1);  // Skip the ';' token
    return assign;
}

// Parse a Function Call Statement
func_call_stmt parse_func_call(const_span& sp) {
    func_call_stmt call{sp.front(), {}};
    sp = sp.subspan(1);  // Skip the function name token

    while (true) {
        call.arguments.push_back(parse_expr(sp));

        if (sp.front().type == Token::Type::SEMICOLON) break;
    }
    sp = sp.subspan(1);  // Skip the ';' token
    return call;
}

// Parse an Expression Statement
expr_stmt parse_expr_stmt(const_span& sp) {
    expr_stmt expr;
    expr.expression = parse_expr(sp);

    if (sp.front().type != Token::Type::SEMICOLON) {
        throw ParserException("Expected ';' after expression", sp.front(),
                              __func__);
    }

    sp = sp.subspan(1);  // Skip the ';' token
    return expr;
}

// Parse an Expression Statement
// Looks for implications with right-associativity
// syntactic sugar for '(not p) | q'
std::unique_ptr<expr> parse_expr(const_span& sp) {
    auto premise{parse_disjunct(sp)};
    if (!sp.empty() && sp.front().type == Token::Type::ARROW) {
        const auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '->' token
        auto conclusion = parse_expr(sp);
        return std::make_unique<expr>(
            bin_expr{std::make_unique<expr>(unary_expr{
                         std::move(premise), Token{Token::Type::BANG, "!"}}),
                     std::move(conclusion), Token{Token::Type::LOR, "|"}});
    }
    return premise;
}

std::unique_ptr<expr> parse_disjunct(const_span& sp) {
    auto conjunct{parse_conjunct(sp)};
    while (sp.front().type == Token::Type::LOR) {
        auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '|' token
        auto right = parse_conjunct(sp);
        conjunct = std::make_unique<expr>(
            bin_expr{std::move(conjunct), std::move(right), op});
    }
    return conjunct;
}

// Parse a Conjunct Expression
std::unique_ptr<expr> parse_conjunct(const_span& sp) {
    auto unary_expr{parse_quantifier(sp)};
    while (sp.front().type == Token::Type::LAND) {
        auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '&' token
        auto right = parse_quantifier(sp);
        unary_expr = std::make_unique<expr>(
            bin_expr{std::move(unary_expr), std::move(right), op});
    }
    return unary_expr;
}

// Parse a Quantifier Expression
std::unique_ptr<expr> parse_quantifier(const_span& sp) {
    // exists '(' IDENTIFIER+ ')' unary
    // forall '(' IDENTIFIER+ ')' unary

    if (sp.front().type == Token::Type::EXISTS ||
        sp.front().type == Token::Type::FORALL) {
        auto quantifier = sp.front();
        sp = sp.subspan(1);  // Skip the quantifier token
        std::vector<Token> bound_vars;

        switch (sp.front().type) {
            case Token::Type::IDENTIFIER:
                bound_vars.push_back(sp.front());
                sp = sp.subspan(1);  // Skip the identifier token
                break;
            case Token::Type::LEFT_PAREN:
                sp = sp.subspan(1);  // Skip the '(' token

                while (sp.front().type == Token::Type::IDENTIFIER) {
                    bound_vars.push_back(sp.front());
                    sp = sp.subspan(1);  // Skip the identifier token
                }

                if (sp.front().type != Token::Type::RIGHT_PAREN) {
                    throw ParserException("Expected ')' after bound variables",
                                          sp.front(), __func__);
                }
                sp = sp.subspan(1);  // Skip the ')' token
                break;
            default:
                throw ParserException(
                    "[Expected '(' or identifier after quantifier", sp.front(),
                    __func__);
        }

        auto body = parse_unary(sp);
        return std::make_unique<expr>(quantifier_expr{
            quantifier, std::move(bound_vars), std::move(body)});
    } else {
        // no quantifier
        return parse_unary(sp);
    }
}

// Parse a Unary Expression
std::unique_ptr<expr> parse_unary(const_span& sp) {
    if (sp.front().type == Token::Type::BANG) {
        auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '!' token
        auto operand = parse_unary(sp);
        return std::make_unique<expr>(unary_expr{std::move(operand), op});
    }
    return parse_primary(sp);
}

// Parse a Primary Expression
std::unique_ptr<expr> parse_primary(const_span& sp) {
    if (sp.front().type == Token::Type::IDENTIFIER) {
        auto id = parse_ident(sp);
        return std::make_unique<expr>(*id);
    } else if (sp.front().type == Token::Type::TRUE ||
               sp.front().type == Token::Type::FALSE) {
        auto lit = parse_literal(sp);
        return std::make_unique<expr>(*lit);
    } else if (sp.front().type == Token::Type::LEFT_PAREN) {
        sp = sp.subspan(1);  // Skip the '(' token
        auto expr = parse_expr(sp);
        sp = sp.subspan(1);  // Skip the ')' token
        return expr;
    }
    throw ParserException("Expected identifier, literal, or '('", sp.front(),
                          __func__);
}

// Parse an Identifier
std::unique_ptr<identifier> parse_ident(const_span& sp) {
    if (sp.front().type != Token::Type::IDENTIFIER) {
        throw ParserException("Expected identifier", sp.front(), __func__);
    }
    auto id = std::make_unique<identifier>(sp.front());
    sp = sp.subspan(1);  // Skip the identifier token
    return id;
}

// Parse a Literal
std::unique_ptr<literal> parse_literal(const_span& sp) {
    if (sp.front().type != Token::Type::TRUE &&
        sp.front().type != Token::Type::FALSE) {
        throw ParserException("Expected literal", sp.front(), __func__);
    }
    auto lit = std::make_unique<literal>(sp.front());
    sp = sp.subspan(1);  // Skip the literal token
    return lit;
}
