#include <iostream>
#include <vector>
#include <map>
#include <set>
#include "parser.hpp"
#include "utils.hpp"

int main() {
    std::string command;
    std::cin >> command;
    command = to_lower_copy(command);

    int n;
    std::cin >> n;

    std::vector<std::string> names(n);
    std::set<std::string> allowed;

    for (int i = 0; i < n; i++) {
        std::cin >> names[i];
        names[i] = to_lower_copy(names[i]);
        allowed.insert(names[i]);
    }

    std::map<std::string, double> vars;
    for (int i = 0; i < n; i++) {
        double v;
        std::cin >> v;
        vars[names[i]] = v;
    }

    std::string expr;
    std::cin >> std::ws;
    std::getline(std::cin, expr);

    try {
        Parser parser(expr, allowed);
        NodePtr tree = parser.parse();

        if (command == "evaluate") {
            std::cout << number_to_string(tree->eval(vars));
        } 
        else if (command == "derivative") {
            NodePtr d = full_simplify(tree->diff(names[0]));
            std::cout << d->str();
        } 
        else if (command == "evaluate_derivative") {
            NodePtr d = full_simplify(tree->diff(names[0]));
            std::cout << number_to_string(d->eval(vars));
        }
        else if (command == "pretty_print") {
            tree->pretty_print(std::cout);
        }
        else {
            std::cout << "ERROR Unsupported command";
        }

    } catch (const std::exception& e) {
        std::cout << "ERROR " << e.what();
    }

    return 0;
}
