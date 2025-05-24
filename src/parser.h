#pragma once

#include <expected>
#include <vector>
#include <span>

#include "ast.h"
#include "engine_exceptions.h"

// Type alias for a constant iterator over expressions
using const_iter = std::vector<Token>::const_iterator;
using const_span = std::span<const Token>;

// Parses a vector of expressions into an Abstract Syntax Tree (AST)
using parse_result_t =
    std::expected<std::vector<stmt>, std::vector<ParserException>>;
parse_result_t parse(const std::vector<Token>& tokens);

// Parses a single statement
stmt parse_statement(const_span& sp);

// Parses a Declaration Statement
decl_stmt parse_decl(const_span& sp);

// Parses an Assignment Statement
assign_stmt parse_assign(const_span& sp);

// Parses a Display Statement
func_call_stmt parse_func_call(const_span& sp);

// Parse an Expression Statement
expr_stmt parse_expr_stmt(const_span& sp);

// Parses an Expression
std::shared_ptr<expr> parse_expr(const_span& sp);

// Parses an Implication Expression
std::shared_ptr<expr> parse_implication(const_span& sp);

// Parses a Disjunction Expression
std::shared_ptr<expr> parse_disjunct(const_span& sp);

// Parses a Conjunct Expression
std::shared_ptr<expr> parse_conjunct(const_span& sp);

// Prases a Quantifier Expression
std::shared_ptr<expr> parse_quantifier(const_span& sp);

// Parses a Unary Expression
std::shared_ptr<expr> parse_unary(const_span& sp);

// Parses a Primary Expression
std::shared_ptr<expr> parse_primary(const_span& sp);

// Parses an Identifier
std::shared_ptr<identifier> parse_ident(const_span& sp);

// Parses a Literal
std::shared_ptr<literal> parse_literal(const_span& sp);
