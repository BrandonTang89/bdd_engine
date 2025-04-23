#pragma once

#include <cassert>
#include <expected>
#include <vector>

#include "ast.h"
#include "engine_exceptions.h"
#include "token.h"

// Type alias for a constant iterator over expressions
using const_iter = std::vector<Token>::const_iterator;

// Parses a vector of expressions into an Abstract Syntax Tree (AST)
using parse_result_t = std::expected<std::vector<stmt>, std::vector<ParserException>>;
parse_result_t parse(const std::vector<Token>& tokens);
parse_result_t parse(const std::string& input);

// Parses a single statement
stmt parse_statement(const_iter& it);

// Parses a Declaration Statement
decl_stmt parse_decl(const_iter& it);

// Parses an Assignment Statement
assign_stmt parse_assign(const_iter& it);

// Parses a Display Statement
func_call_stmt parse_func_call(const_iter& it);

// Parse an Expression Statement
expr_stmt parse_expr_stmt(const_iter& it);

// Parses an Expression
std::unique_ptr<expr> parse_expr(const_iter& it);

// Parses a Conjunct Expression
std::unique_ptr<expr> parse_conjuct(const_iter& it);

// Prases a Quantifier Expression
std::unique_ptr<expr> parse_quantifier(const_iter& it);

// Parses a Unary Expression
std::unique_ptr<expr> parse_unary(const_iter& it);

// Parses a Primary Expression
std::unique_ptr<expr> parse_primary(const_iter& it);

// Parses an Identifier
std::unique_ptr<identifier> parse_ident(const_iter& it);

// Parses a Literal
std::unique_ptr<literal> parse_literal(const_iter& it);