#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

bool doubleEquals(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

std::vector<Token> tokenizeAll(const std::string& s) {
    Lexer lexer(s);
    std::vector<Token> tokens;

    while (true) {
        Token t = lexer.next();
        tokens.push_back(t);
        if (t.type == lexem_t::EOEX) {
            break;
        }
    }

    return tokens;
}

NodePtr parseExpr(const std::string& expr, const std::set<std::string>& allowed = {}) {
    Parser parser(expr, allowed);
    return parser.parse();
}

double evalExpr(const std::string& expr,
                const std::map<std::string, double>& vars = {},
                const std::set<std::string>& allowed = {}) {
    NodePtr tree = parseExpr(expr, allowed);
    return tree->eval(vars);
}

double evalDerivative(const std::string& expr,
                      const std::string& var,
                      const std::map<std::string, double>& vars = {},
                      const std::set<std::string>& allowed = {}) {
    NodePtr tree = parseExpr(expr, allowed);
    NodePtr d = full_simplify(tree->diff(var));
    return d->eval(vars);
}

void assertThrowsContains(const std::string& expectedSubstring, const std::function<void()>& fn) {
    try {
        fn();
        assert(false && "Expected exception was not thrown");
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        assert(msg.find(expectedSubstring) != std::string::npos);
    }
}

void testLexer() {
    std::cout << "lexer\n";

    {
        std::vector<Token> tokens = tokenizeAll("x + 2 * 3");
        assert(tokens.size() >= 6);
        assert(tokens[0].type == lexem_t::IDENT);
        assert(tokens[0].value == "x");
        assert(tokens[1].type == lexem_t::OP);
        assert(tokens[1].value == "+");
        assert(tokens[2].type == lexem_t::NUMBER);
        assert(tokens[2].value == "2");
        std::cout << "  ok: basic tokens\n";
    }

    {
        std::vector<Token> tokens = tokenizeAll("sin(x)");
        assert(tokens.size() >= 5);
        assert(tokens[0].type == lexem_t::IDENT);
        assert(tokens[0].value == "sin");
        assert(tokens[1].type == lexem_t::PAREN);
        assert(tokens[1].value == "(");
        std::cout << "  ok: function tokens\n";
    }

    {
        std::vector<Token> tokens = tokenizeAll("3.14");
        assert(tokens[0].type == lexem_t::NUMBER);
        assert(tokens[0].value == "3.14");
        std::cout << "  ok: decimal numbers\n";
    }

    {
        assertThrowsContains("Invalid number", []() {
            tokenizeAll("002.5");
        });
        std::cout << "  ok: leading zeros rejected\n";
    }

    {
        assertThrowsContains("Invalid number", []() {
            tokenizeAll(".5");
        });
        std::cout << "  ok: dot-leading number rejected\n";
    }
}

void testParser() {
    std::cout << "parser\n";

    {
        NodePtr expr = parseExpr("2 + 3");
        assert(expr != nullptr);
        std::cout << "  ok: simple expression\n";
    }

    {
        double result = evalExpr("2 + 3 * 4");
        assert(doubleEquals(result, 14.0));
        std::cout << "  ok: operator precedence\n";
    }

    {
        double result = evalExpr("(2 + 3) * 4");
        assert(doubleEquals(result, 20.0));
        std::cout << "  ok: parentheses\n";
    }

    {
        double result = evalExpr("2^3^2");
        assert(doubleEquals(result, 512.0));
        std::cout << "  ok: right-associative power\n";
    }

    {
        double result = evalExpr("-3^2");
        assert(doubleEquals(result, -9.0));
        std::cout << "  ok: unary minus precedence\n";
    }
}

void testEvaluator() {
    std::cout << "evaluator\n";

    {
        double result = evalExpr("2 + 3");
        assert(doubleEquals(result, 5.0));
        std::cout << "  ok: addition\n";
    }

    {
        double result = evalExpr("2 * 3");
        assert(doubleEquals(result, 6.0));
        std::cout << "  ok: multiplication\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 3.0}, {"y", 4.0}};
        std::set<std::string> allowed = {"x", "y"};
        double result = evalExpr("x * x + y * 2", vars, allowed);
        assert(doubleEquals(result, 17.0));
        std::cout << "  ok: variables\n";
    }

    {
        double result = evalExpr("2 ^ 3");
        assert(doubleEquals(result, 8.0));
        std::cout << "  ok: power\n";
    }

    {
        double result = evalExpr("sin(0)");
        assert(doubleEquals(result, 0.0));
        std::cout << "  ok: sin\n";
    }

    {
        double result = evalExpr("sqrt(4)");
        assert(doubleEquals(result, 2.0));
        std::cout << "  ok: sqrt\n";
    }

    {
        double result = evalExpr("log(1)");
        assert(doubleEquals(result, 0.0));
        std::cout << "  ok: log\n";
    }
}

void testDerivative() {
    std::cout << "derivative\n";

    {
        double result = evalDerivative("5", "x");
        assert(doubleEquals(result, 0.0));
        std::cout << "  ok: constant\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 5.0}};
        std::set<std::string> allowed = {"x"};
        double result = evalDerivative("x", "x", vars, allowed);
        assert(doubleEquals(result, 1.0));
        std::cout << "  ok: variable\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 3.0}};
        std::set<std::string> allowed = {"x"};
        double result = evalDerivative("x^2", "x", vars, allowed);
        assert(doubleEquals(result, 6.0));
        std::cout << "  ok: x^2\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 0.0}};
        std::set<std::string> allowed = {"x"};
        double result = evalDerivative("sin(x)", "x", vars, allowed);
        assert(doubleEquals(result, 1.0));
        std::cout << "  ok: sin(x)\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 1.0}};
        std::set<std::string> allowed = {"x"};
        double result = evalDerivative("log(x)", "x", vars, allowed);
        assert(doubleEquals(result, 1.0));
        std::cout << "  ok: log(x)\n";
    }
}

void testComplexExpressions() {
    std::cout << "complex expressions\n";

    {
        std::map<std::string, double> vars = {{"x", 3.0}, {"y", 4.0}};
        std::set<std::string> allowed = {"x", "y"};
        double result = evalExpr("x*x + y*2", vars, allowed);
        assert(doubleEquals(result, 17.0));
        std::cout << "  ok: x*x + y*2\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 3.0}};
        std::set<std::string> allowed = {"x"};
        double result = evalExpr("-x + 5", vars, allowed);
        assert(doubleEquals(result, 2.0));
        std::cout << "  ok: unary minus\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 0.0}};
        std::set<std::string> allowed = {"x"};
        double result = evalExpr("sin(cos(x))", vars, allowed);
        assert(doubleEquals(result, std::sin(1.0)));
        std::cout << "  ok: nested functions\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 5.0}};
        std::set<std::string> allowed = {"x"};
        double result = evalExpr("(x + 1) * (x - 1)", vars, allowed);
        assert(doubleEquals(result, 24.0));
        std::cout << "  ok: product of sums\n";
    }
}

void testErrors() {
    std::cout << "errors\n";

    {
        assertThrowsContains("Unknown variable", []() {
            std::map<std::string, double> vars = {{"x", 1.0}};
            std::set<std::string> allowed = {"x", "z"};
            evalExpr("x + z", vars, allowed);
        });
        std::cout << "  ok: unknown variable\n";
    }

    {
        assertThrowsContains("Unexpected end of expression", []() {
            parseExpr("x +", {"x"});
        });
        std::cout << "  ok: syntax error\n";
    }

    {
        assertThrowsContains("Unknown function", []() {
            parseExpr("unknown(x)", {"x"});
        });
        std::cout << "  ok: unknown function\n";
    }

    {
        assertThrowsContains("Division by zero", []() {
            evalExpr("1 / 0");
        });
        std::cout << "  ok: division by zero\n";
    }

    {
        assertThrowsContains("Domain error: sqrt", []() {
            evalExpr("sqrt(-1)");
        });
        std::cout << "  ok: sqrt domain error\n";
    }

    {
        assertThrowsContains("Domain error: log", []() {
            evalExpr("log(0)");
        });
        std::cout << "  ok: log domain error\n";
    }

    {
        assertThrowsContains("Invalid number", []() {
            tokenizeAll("0002");
        });
        std::cout << "  ok: invalid number\n";
    }
}

int main() {
    testLexer();
    testParser();
    testEvaluator();
    testDerivative();
    testComplexExpressions();
    testErrors();

    std::cout << "done\n";
    return 0;
}
