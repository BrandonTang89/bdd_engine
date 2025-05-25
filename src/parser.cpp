// Recursive Descent Parser
#include "parser.h"

#include "absl/log/log.h"
#include "ast.h"
#include "lexer.h"

// Parses a vector of tokens into an AST
parse_result_t parse(const std::vector<token>& tokens) {
    // Tokenise the input string
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
            while (!sp.empty() && sp.front().type != token::Type::SEMICOLON) {
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

stmt parse_statement(const_span& sp) {
    switch (sp.front().type) {
        case token::Type::BVAR:
            return parse_decl(sp);
        case token::Type::SET:
            return parse_assign(sp);
        case token::Type::TREE_DISPLAY:
        case token::Type::GRAPH_DISPLAY:
        case token::Type::IS_SAT:
        case token::Type::SOURCE:
            return parse_func_call(sp);
        default:  // assume expr statement
            return parse_expr_stmt(sp);
    }
}

// Parse a Declaration Statement
decl_stmt parse_decl(const_span& sp) {
    decl_stmt decl;
    sp = sp.subspan(1);  // Move the iterator forward
    if (sp.front().type != token::Type::IDENTIFIER) {
        throw ParserException("Expected identifier after 'bvar'", sp.front(),
                              __func__);
    }

    while (sp.front().type == token::Type::IDENTIFIER) {
        decl.identifiers.push_back(sp.front());
        sp = sp.subspan(1);
    }

    if (sp.front().type == token::Type::SEMICOLON) {
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

    if (sp.front().type != token::Type::EQUAL) {
        throw ParserException("Expected '=' after identifier", sp.front(),
                              __func__);
    }
    sp = sp.subspan(1);  // Skip the '=' token

    assign.value = parse_expr(sp);
    if (sp.front().type != token::Type::SEMICOLON) {
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

        if (sp.front().type == token::Type::SEMICOLON) break;
    }
    sp = sp.subspan(1);  // Skip the ';' token
    return call;
}

// Parse an Expression Statement
expr_stmt parse_expr_stmt(const_span& sp) {
    expr_stmt expr;
    expr.expression = parse_expr(sp);

    if (sp.front().type != token::Type::SEMICOLON) {
        throw ParserException("Expected ';' after expression", sp.front(),
                              __func__);
    }

    sp = sp.subspan(1);  // Skip the ';' token
    return expr;
}

std::shared_ptr<expr> parse_expr(const_span& sp) {
    return parse_substitute(sp);
}

std::shared_ptr<expr> parse_substitute(const_span& sp) {
    // 'sub' '{' (IDENTIFIER ':' expr (',' IDENTIFIER ':' expr)*)? '}' expr
    if (sp.front().type == token::Type::SUBSTITUTE) {
        sp = sp.subspan(1);  // Skip the 'substitute' token
        if (sp.front().type != token::Type::LEFT_BRACE) {
            throw ParserException("Expected '{' after 'substitute'", sp.front(),
                                  __func__);
        }
        sp = sp.subspan(1);  // Skip the '{' token

        substitution_map subs;
        while (sp.front().type == token::Type::IDENTIFIER) {
            auto ident = parse_ident(sp);
            if (sp.front().type != token::Type::COLON) {
                throw ParserException("Expected ':' after identifier",
                                      sp.front(), __func__);
            }
            sp = sp.subspan(1);  // Skip the ':' token
            auto value = parse_expr(sp);

            // Add to the substitution map
            subs[ident->name.lexeme] = std::move(value);

            if (sp.front().type == token::Type::COMMA) {
                sp = sp.subspan(1);  // Skip the ',' token
            } else if (sp.front().type == token::Type::RIGHT_BRACE) {
                break;  // End of substitutions
            } else {
                throw ParserException("Expected ',' or '}' after substitution",
                                      sp.front(), __func__);
            }
        }
        if (sp.front().type != token::Type::RIGHT_BRACE) {
            throw ParserException("Expected '}' after substitutions",
                                  sp.front(), __func__);
        }
        sp = sp.subspan(1);  // Skip the '}' token
        auto body = parse_expr(sp);

        return std::make_shared<expr>(
            sub_expr{std::move(subs), std::move(body)});
    }
    // If no 'substitute' keyword, just parse the expression
    return parse_equality(sp);
}

// Handle the equivalence (==) and XOR (!=) operations
std::shared_ptr<expr> parse_equality(const_span& sp) {
    auto left{parse_implication(sp)};

    if (sp.front().type == token::Type::EQUAL_EQUAL) {
        // p == q is converted to (p & q) | (!p & !q)
        const auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '==' token
        auto right = parse_implication(sp);

        // Create (p & q)
        auto p_and_q = std::make_shared<expr>(
            bin_expr{left, right, token{token::Type::LAND, "&"}});

        // Create (!p & !q)
        auto not_p = std::make_shared<expr>(
            unary_expr{std::move(left), token{token::Type::BANG, "!"}});
        auto not_q = std::make_shared<expr>(
            unary_expr{std::move(right), token{token::Type::BANG, "!"}});
        auto not_p_and_not_q = std::make_shared<expr>(bin_expr{
            std::move(not_p), std::move(not_q), token{token::Type::LAND, "&"}});

        // Create (p & q) | (!p & !q)
        return std::make_shared<expr>(bin_expr{std::move(p_and_q),
                                               std::move(not_p_and_not_q),
                                               token{token::Type::LOR, "|"}});
    }

    else if (sp.front().type == token::Type::BANG_EQUAL) {
        // p != q -> (p & !q) | (!p & q)
        const auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '!=' token
        auto right = parse_implication(sp);

        auto not_p = std::make_shared<expr>(
            unary_expr{left, token{token::Type::BANG, "!"}});
        auto not_q = std::make_shared<expr>(
            unary_expr{right, token{token::Type::BANG, "!"}});

        // Create (p & !q)
        auto p_and_not_q = std::make_shared<expr>(bin_expr{
            std::move(left), std::move(not_q), token{token::Type::LAND, "&"}});
        // Create (!p & q)
        auto not_p_and_q = std::make_shared<expr>(bin_expr{
            std::move(not_p), std::move(right), token{token::Type::LAND, "&"}});

        // Create (p & !q) | (!p & q)
        return std::make_shared<expr>(bin_expr{std::move(p_and_not_q),
                                               std::move(not_p_and_q),
                                               token{token::Type::LOR, "|"}});
    }

    return left;
}

// Parse an implication
// Looks for implications with right-associativity
// syntactic sugar for '(not p) | q'
std::shared_ptr<expr> parse_implication(const_span& sp) {
    auto premise{parse_disjunct(sp)};
    if (!sp.empty() && sp.front().type == token::Type::ARROW) {
        const auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '->' token
        auto conclusion = parse_implication(sp);
        return std::make_shared<expr>(
            bin_expr{std::make_shared<expr>(
                         unary_expr{premise, token{token::Type::BANG, "!"}}),
                     std::move(conclusion), token{token::Type::LOR, "|"}});
    }
    return premise;
}

std::shared_ptr<expr> parse_disjunct(const_span& sp) {
    auto conjunct{parse_conjunct(sp)};
    while (sp.front().type == token::Type::LOR) {
        const auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '|' token
        auto right = parse_conjunct(sp);
        conjunct = std::make_shared<expr>(
            bin_expr{std::move(conjunct), std::move(right), op});
    }
    return conjunct;
}

// Parse a Conjunct Expression
std::shared_ptr<expr> parse_conjunct(const_span& sp) {
    auto unary_expr{parse_quantifier(sp)};
    while (sp.front().type == token::Type::LAND) {
        const auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '&' token
        auto right = parse_quantifier(sp);
        unary_expr = std::make_shared<expr>(
            bin_expr{std::move(unary_expr), std::move(right), op});
    }
    return unary_expr;
}

// Parse a Quantifier Expression
std::shared_ptr<expr> parse_quantifier(const_span& sp) {
    // exists '(' IDENTIFIER+ ')' unary
    // forall '(' IDENTIFIER+ ')' unary

    if (sp.front().type == token::Type::EXISTS ||
        sp.front().type == token::Type::FORALL) {
        const auto quantifier = sp.front();
        sp = sp.subspan(1);  // Skip the quantifier token
        std::vector<token> bound_vars;

        switch (sp.front().type) {
            case token::Type::IDENTIFIER:
                bound_vars.push_back(sp.front());
                sp = sp.subspan(1);  // Skip the identifier token
                break;
            case token::Type::LEFT_PAREN:
                sp = sp.subspan(1);  // Skip the '(' token

                while (sp.front().type == token::Type::IDENTIFIER) {
                    bound_vars.push_back(sp.front());
                    sp = sp.subspan(1);  // Skip the identifier token
                }

                if (sp.front().type != token::Type::RIGHT_PAREN) {
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
        return std::make_shared<expr>(quantifier_expr{
            quantifier, std::move(bound_vars), std::move(body)});
    } else {
        // no quantifier
        return parse_unary(sp);
    }
}

// Parse a Unary Expression
std::shared_ptr<expr> parse_unary(const_span& sp) {
    if (sp.front().type == token::Type::BANG) {
        const auto op = sp.front();
        sp = sp.subspan(1);  // Skip the '!' token
        auto operand = parse_unary(sp);
        return std::make_shared<expr>(unary_expr{std::move(operand), op});
    }
    return parse_primary(sp);
}

// Parse a Primary Expression
std::shared_ptr<expr> parse_primary(const_span& sp) {
    if (sp.front().type == token::Type::IDENTIFIER) {
        const auto id = parse_ident(sp);
        return std::make_shared<expr>(*id);
    } else if (sp.front().type == token::Type::ID ||
               sp.front().type == token::Type::TRUE ||
               sp.front().type == token::Type::FALSE) {
        const auto lit = parse_literal(sp);
        return std::make_shared<expr>(*lit);
    } else if (sp.front().type == token::Type::LEFT_PAREN) {
        sp = sp.subspan(1);  // Skip the '(' token
        auto expr = parse_expr(sp);
        sp = sp.subspan(1);  // Skip the ')' token
        return expr;
    }
    throw ParserException("Expected identifier, literal, or '('", sp.front(),
                          __func__);
}

// Parse an Identifier
std::shared_ptr<identifier> parse_ident(const_span& sp) {
    if (sp.front().type != token::Type::IDENTIFIER) {
        throw ParserException("Expected identifier", sp.front(), __func__);
    }
    auto id = std::make_shared<identifier>(sp.front());
    sp = sp.subspan(1);  // Skip the identifier token
    return id;
}

// Parse a Literal
std::shared_ptr<literal> parse_literal(const_span& sp) {
    if (sp.front().type != token::Type::TRUE &&
        sp.front().type != token::Type::FALSE &&
        sp.front().type != token::Type::ID) {
        throw ParserException("Expected literal", sp.front(), __func__);
    }
    auto lit = std::make_shared<literal>(sp.front());
    sp = sp.subspan(1);  // Skip the literal token
    return lit;
}
