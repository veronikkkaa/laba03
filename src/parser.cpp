#include "parser.hpp"
#include "utils.hpp"
#include <stdexcept>

Parser::Parser(const std::string& expression, const std::set<std::string>& variables)
    : lexer(expression), allowed_vars(variables) {
    next_token();
}

void Parser::next_token() {
    current = lexer.next();
}

bool Parser::is_op(const std::string& op) const {
    return current.type == lexem_t::OP && current.value == op;
}

bool Parser::is_paren(const std::string& p) const {
    return current.type == lexem_t::PAREN && current.value == p;
}

NodePtr Parser::parse_expression() {
    NodePtr value = parse_term();

    while (is_op("+") || is_op("-")) {
        std::string op = current.value;
        next_token();
        NodePtr rhs = parse_term();
        value = std::make_shared<BinaryNode>(op, value, rhs);
    }

    return value;
}

NodePtr Parser::parse_term() {
    NodePtr value = parse_unary();

    while (is_op("*") || is_op("/")) {
        std::string op = current.value;
        next_token();
        NodePtr rhs = parse_unary();
        value = std::make_shared<BinaryNode>(op, value, rhs);
    }

    return value;
}

NodePtr Parser::parse_unary() {
    if (is_op("+")) {
        next_token();
        return parse_unary();
    }

    if (is_op("-")) {
        next_token();
        return std::make_shared<UnaryNode>("-", parse_unary());
    }

    return parse_power();
}

NodePtr Parser::parse_power() {
    NodePtr left = parse_primary();

    if (is_op("^")) {
        next_token();
        NodePtr right = parse_unary();
        return std::make_shared<BinaryNode>("^", left, right);
    }

    return left;
}

NodePtr Parser::parse_primary() {
    if (current.type == lexem_t::NUMBER) {
        double value = std::stod(current.value);
        next_token();
        return std::make_shared<NumberNode>(value);
    }

    if (current.type == lexem_t::IDENT) {
        std::string name = current.value;
        next_token();

        if (is_paren("(")) {
            if (!is_builtin_function(name)) {
                throw std::runtime_error("Unknown function: " + name);
            }

            next_token();
            NodePtr arg = parse_expression();

            if (!is_paren(")")) {
                throw std::runtime_error("Expected )");
            }

            next_token();
            return std::make_shared<FunctionNode>(name, arg);
        }

        if (!allowed_vars.count(name)) {
            throw std::runtime_error("Unknown variable: " + name);
        }

        return std::make_shared<VariableNode>(name);
    }

    if (is_paren("(")) {
        next_token();
        NodePtr value = parse_expression();

        if (!is_paren(")")) {
            throw std::runtime_error("Expected )");
        }

        next_token();
        return value;
    }

    throw std::runtime_error("Bad expression");
}

NodePtr Parser::parse() {
    NodePtr result = parse_expression();

    if (current.type != lexem_t::EOEX) {
        throw std::runtime_error("Extra tokens");
    }

    return result;
}
