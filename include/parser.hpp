#pragma once

#include <set>
#include <string>
#include "lexer.hpp"
#include "ast.hpp"

class Parser {
private:
    Lexer lexer;
    Token current;
    std::set<std::string> allowed_vars; 

    void next_token();

    bool is_op(const std::string& op) const;
    bool is_paren(const std::string& p) const;

    NodePtr parse_expression();
    NodePtr parse_term();
    NodePtr parse_unary();
    NodePtr parse_power();
    NodePtr parse_primary();

public:
    Parser(const std::string& expression, const std::set<std::string>& variables);

    NodePtr parse();
};
