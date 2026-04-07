#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <stdexcept>

#include "parser.hpp"
int main() {
    std::string line;
    std::getline(std::cin, line);

    try {
        std::istringstream in(line);

        std::string command;
        in >> command;
        command = to_lower_copy(command);

        int n;
        in >> n;

        if (!in || n < 0) {
            throw std::runtime_error("Bad input");
        }

        std::vector<std::string> names(n);
        std::set<std::string> allowed;

        for (int i = 0; i < n; i++) {
            in >> names[i];
            if (!in) {
                throw std::runtime_error("Bad input");
            }
            names[i] = to_lower_copy(names[i]);
            allowed.insert(names[i]);
        }

        std::map<std::string, double> vars;
        for (int i = 0; i < n; i++) {
            double v;
            in >> v;
            if (!in) {
                throw std::runtime_error("Bad input");
            }
            vars[names[i]] = v;
        }

        std::string expr;
        std::getline(in >> std::ws, expr);

        if (expr.empty()) {
            throw std::runtime_error("Bad expression");
        }

        Parser parser(expr, allowed);
        NodePtr tree = parser.parse();

        if (command == "evaluate") {
            std::cout << number_to_string(tree->eval(vars));
        } else if (command == "derivative") {
            if (names.empty()) {
                throw std::runtime_error("No variable for derivative");
            }
            NodePtr d = full_simplify(tree->diff(names[0]));
            std::cout << d->str();
        } else if (command == "evaluate_derivative") {
            if (names.empty()) {
                throw std::runtime_error("No variable for derivative");
            }
            NodePtr d = full_simplify(tree->diff(names[0]));
            std::cout << number_to_string(d->eval(vars));
        } else if (command == "pretty_print") {
            tree->pretty_print(std::cout);
        } else {
            std::cout << "ERROR Unsupported command";
        }

    } catch (const std::exception& e) {
        std::cout << "ERROR " << e.what();
    }

    return 0;
}
