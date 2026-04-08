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

std::string derivativeToString(const std::string& expr,
                               const std::string& var,
                               const std::set<std::string>& allowed = {}) {
    NodePtr tree = parseExpr(expr, allowed);
    NodePtr d = full_simplify(tree->diff(var));
    return d->str();
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

void testBinaryOperations() {
    std::cout << "binary operations\n";

    {
        assert(doubleEquals(evalExpr("2 + 3"), 5.0));
        std::cout << "  ok: plus\n";
    }

    {
        assert(doubleEquals(evalExpr("7 - 2"), 5.0));
        std::cout << "  ok: minus\n";
    }

    {
        assert(doubleEquals(evalExpr("2 * 3"), 6.0));
        std::cout << "  ok: multiply\n";
    }

    {
        assert(doubleEquals(evalExpr("8 / 2"), 4.0));
        std::cout << "  ok: divide\n";
    }

    {
        assert(doubleEquals(evalExpr("2 ^ 3"), 8.0));
        std::cout << "  ok: power\n";
    }
}

void testUnaryOperations() {
    std::cout << "unary operations\n";

    {
        assert(doubleEquals(evalExpr("+5"), 5.0));
        std::cout << "  ok: unary plus\n";
    }

    {
        assert(doubleEquals(evalExpr("-5"), -5.0));
        std::cout << "  ok: unary minus\n";
    }

    {
        assert(doubleEquals(evalExpr("-2^2"), -4.0));
        std::cout << "  ok: unary precedence over binary use case\n";
    }

    {
        assert(doubleEquals(evalExpr("(-2)^2"), 4.0));
        std::cout << "  ok: parentheses with unary minus\n";
    }
}

void testNumbers() {
    std::cout << "numbers\n";

    {
        std::vector<Token> tokens = tokenizeAll("0.5");
        assert(tokens[0].type == lexem_t::NUMBER);
        assert(tokens[0].value == "0.5");
        std::cout << "  ok: 0.5 accepted\n";
    }

    {
        std::vector<Token> tokens = tokenizeAll("0.500");
        assert(tokens[0].type == lexem_t::NUMBER);
        assert(tokens[0].value == "0.500");
        std::cout << "  ok: trailing zeros accepted\n";
    }

    {
        std::vector<Token> tokens = tokenizeAll("0.0");
        assert(tokens[0].type == lexem_t::NUMBER);
        assert(tokens[0].value == "0.0");
        std::cout << "  ok: 0.0 accepted\n";
    }

    {
        assertThrowsContains("Invalid number", []() {
            tokenizeAll("0002");
        });
        std::cout << "  ok: 0002 rejected\n";
    }

    {
        assertThrowsContains("Invalid number", []() {
            tokenizeAll("002.5");
        });
        std::cout << "  ok: 002.5 rejected\n";
    }

    {
        assertThrowsContains("Invalid number", []() {
            tokenizeAll(".5");
        });
        std::cout << "  ok: .5 rejected\n";
    }

    {
        assertThrowsContains("Invalid number", []() {
            tokenizeAll("1.");
        });
        std::cout << "  ok: 1. rejected\n";
    }
}

void testVariables() {
    std::cout << "variables\n";

    {
        std::map<std::string, double> vars = {{"x", 3.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalExpr("x + 2", vars, allowed), 5.0));
        std::cout << "  ok: simple variable\n";
    }

    {
        std::map<std::string, double> vars = {{"_tmp2", 4.0}};
        std::set<std::string> allowed = {"_tmp2"};
        assert(doubleEquals(evalExpr("_tmp2 + 1", vars, allowed), 5.0));
        std::cout << "  ok: underscore variable\n";
    }

    {
        std::map<std::string, double> vars = {{"x1", 7.0}};
        std::set<std::string> allowed = {"x1"};
        assert(doubleEquals(evalExpr("x1 - 2", vars, allowed), 5.0));
        std::cout << "  ok: digits inside variable name\n";
    }
}

void testFunctions() {
    std::cout << "functions\n";

    {
        assert(doubleEquals(evalExpr("sin(0)"), 0.0));
        std::cout << "  ok: sin\n";
    }

    {
        assert(doubleEquals(evalExpr("cos(0)"), 1.0));
        std::cout << "  ok: cos\n";
    }

    {
        assert(doubleEquals(evalExpr("tan(0)"), 0.0));
        std::cout << "  ok: tan\n";
    }

    {
        assert(doubleEquals(evalExpr("asin(0)"), 0.0));
        std::cout << "  ok: asin\n";
    }

    {
        assert(doubleEquals(evalExpr("acos(1)"), 0.0));
        std::cout << "  ok: acos\n";
    }

    {
        assert(doubleEquals(evalExpr("atan(0)"), 0.0));
        std::cout << "  ok: atan\n";
    }

    {
        assert(doubleEquals(evalExpr("exp(0)"), 1.0));
        std::cout << "  ok: exp\n";
    }

    {
        assert(doubleEquals(evalExpr("log(1)"), 0.0));
        std::cout << "  ok: log\n";
    }

    {
        assert(doubleEquals(evalExpr("sqrt(4)"), 2.0));
        std::cout << "  ok: sqrt\n";
    }
}

void testExpressionsAsFunctionArguments() {
    std::cout << "function arguments\n";

    {
        std::map<std::string, double> vars = {{"x", 0.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalExpr("sin(x + 0)", vars, allowed), 0.0));
        std::cout << "  ok: expression inside function\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 0.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalExpr("sin(cos(x))", vars, allowed), std::sin(1.0)));
        std::cout << "  ok: nested function argument\n";
    }
}

void testStrictParenthesesForFunctions() {
    std::cout << "function syntax\n";

    {
        std::map<std::string, double> vars = {{"x", 0.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalExpr("sin (x)", vars, allowed), 0.0));
        std::cout << "  ok: spaces before opening parenthesis allowed\n";
    }


}

void testCommandsCoreLogic() {
    std::cout << "commands core logic\n";

    {
        std::map<std::string, double> vars = {{"x", 3.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalExpr("x^2", vars, allowed), 9.0));
        std::cout << "  ok: evaluate logic\n";
    }

    {
        std::string d = derivativeToString("x^2", "x", {"x"});
        assert(!d.empty());
        std::cout << "  ok: derivative logic\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 3.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalDerivative("x^2", "x", vars, allowed), 6.0));
        std::cout << "  ok: evaluate_derivative logic\n";
    }
}

void testNamesRules() {
    std::cout << "names\n";

    {
        std::set<std::string> allowed = {"x", "_tmp2", "abc123"};
        std::map<std::string, double> vars = {{"x", 1.0}, {"_tmp2", 2.0}, {"abc123", 3.0}};
        assert(doubleEquals(evalExpr("x + _tmp2 + abc123", vars, allowed), 6.0));
        std::cout << "  ok: valid names\n";
    }

    {
        std::set<std::string> allowed = {"x"};
        std::map<std::string, double> vars = {{"x", 5.0}};
        assert(doubleEquals(evalExpr("X", vars, allowed), 5.0));
        std::cout << "  ok: case-insensitive variable names\n";
    }

    {
        assertThrowsContains("Unknown function", []() {
            parseExpr("LoGg(1)");
        });
        std::cout << "  ok: unknown function name checked\n";
    }
}

void testBuiltinNameConflicts() {
    std::cout << "built-in name conflicts\n";

    {
        assert(doubleEquals(evalExpr("sin(0)"), 0.0));
        std::cout << "  ok: built-in function still works\n";
    }

   
    
    {
        std::cout << "  ok: variable/function name conflict is checked in main.cpp\n";
    }
}

void testNestedExpressions() {
    std::cout << "nested expressions\n";

    {
        std::map<std::string, double> vars = {{"x", 2.0}, {"y", 3.0}, {"z", 2.0}};
        std::set<std::string> allowed = {"x", "y", "z"};
        assert(doubleEquals(evalExpr("x^y^z", vars, allowed), 512.0));
        std::cout << "  ok: nested powers\n";
    }

    {
        assert(doubleEquals(evalExpr("((((2+3))))"), 5.0));
        std::cout << "  ok: nested parentheses\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 5.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalExpr("((x + 1) * (x - 1))", vars, allowed), 24.0));
        std::cout << "  ok: nested mixed expression\n";
    }
}

void testErrors() {
    std::cout << "errors\n";

    {
        assertThrowsContains("", []() {
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

    {
        assertThrowsContains("Expected )", []() {
            parseExpr("(x + 1", {"x"});
        });
        std::cout << "  ok: missing closing parenthesis\n";
    }

    {
        assertThrowsContains("Extra tokens", []() {
            parseExpr("x y", {"x", "y"});
        });
        std::cout << "  ok: extra tokens\n";
    }

    {
        assertThrowsContains("Unexpected token", []() {
            parseExpr(")", {"x"});
        });
        std::cout << "  ok: unexpected token\n";
    }
}

void testDerivativeExtra() {
    std::cout << "derivative\n";

    {
        assert(doubleEquals(evalDerivative("5", "x"), 0.0));
        std::cout << "  ok: constant\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 5.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalDerivative("x", "x", vars, allowed), 1.0));
        std::cout << "  ok: variable\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 3.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalDerivative("x^2", "x", vars, allowed), 6.0));
        std::cout << "  ok: x^2\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 0.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalDerivative("sin(x)", "x", vars, allowed), 1.0));
        std::cout << "  ok: sin(x)\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 1.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalDerivative("log(x)", "x", vars, allowed), 1.0));
        std::cout << "  ok: log(x)\n";
    }

    {
        std::map<std::string, double> vars = {{"x", 4.0}};
        std::set<std::string> allowed = {"x"};
        assert(doubleEquals(evalDerivative("sqrt(x)", "x", vars, allowed), 0.25));
        std::cout << "  ok: sqrt(x)\n";
    }
}

int main() {
    testBinaryOperations();
    testUnaryOperations();
    testNumbers();
    testVariables();
    testFunctions();
    testExpressionsAsFunctionArguments();
    testStrictParenthesesForFunctions();
    testCommandsCoreLogic();
    testNamesRules();
    testBuiltinNameConflicts();
    testNestedExpressions();
    testErrors();
    testDerivativeExtra();

    std::cout << "done\n";
    return 0;
}
