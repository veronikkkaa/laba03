#include <cctype>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "parser.hpp"
#include "utils.hpp"

struct Request {
    std::string command;
    int n = 0;
    std::vector<std::string> names;
    std::map<std::string, double> vars;
    std::string expr;
};

static std::string trim(const std::string& s) {
    size_t left = 0;
    while (left < s.size() && std::isspace(static_cast<unsigned char>(s[left]))) {
        ++left;
    }

    size_t right = s.size();
    while (right > left && std::isspace(static_cast<unsigned char>(s[right - 1]))) {
        --right;
    }

    return s.substr(left, right - left);
}

static bool is_valid_identifier(const std::string& s) {
    if (s.empty()) {
        return false;
    }

    unsigned char first = static_cast<unsigned char>(s[0]);
    if (!(std::isalpha(first) || s[0] == '_')) {
        return false;
    }

    for (char ch : s) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (!(std::isalnum(c) || ch == '_')) {
            return false;
        }
    }

    return true;
}

static std::vector<std::string> split_lines(const std::string& input) {
    std::vector<std::string> lines;
    std::istringstream in(input);
    std::string line;

    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }

    return lines;
}

static std::vector<std::string> split_by_semicolon(const std::string& input) {
    std::vector<std::string> result;
    std::string current;

    for (char ch : input) {
        if (ch == ';') {
            std::string part = trim(current);
            if (!part.empty()) {
                result.push_back(part);
            }
            current.clear();
        } else {
            current += ch;
        }
    }

    std::string tail = trim(current);
    if (!tail.empty()) {
        result.push_back(tail);
    }

    return result;
}

static void validate_command(const std::string& command) {
    if (command != "evaluate" &&
        command != "derivative" &&
        command != "evaluate_derivative") {
        throw std::runtime_error("Unsupported command");
    }
}

static void validate_variable_name(const std::string& name) {
    if (!is_valid_identifier(name)) {
        throw std::runtime_error("Invalid variable name: " + name);
    }
    if (is_builtin_function(name)) {
        throw std::runtime_error("Variable name conflicts with builtin function: " + name);
    }
}

static Request parse_inline_request(const std::string& text) {
    std::istringstream in(text);
    Request req;

    if (!(in >> req.command)) {
        throw std::runtime_error("Bad input");
    }
    req.command = to_lower_copy(req.command);
    validate_command(req.command);

    if (!(in >> req.n) || req.n < 0) {
        throw std::runtime_error("Bad input");
    }

    req.names.resize(req.n);
    std::set<std::string> used_names;

    for (int i = 0; i < req.n; ++i) {
        if (!(in >> req.names[i])) {
            throw std::runtime_error("Bad input");
        }

        req.names[i] = to_lower_copy(req.names[i]);
        validate_variable_name(req.names[i]);

        if (!used_names.insert(req.names[i]).second) {
            throw std::runtime_error("Duplicate variable: " + req.names[i]);
        }
    }

    for (int i = 0; i < req.n; ++i) {
        double value;
        if (!(in >> value)) {
            throw std::runtime_error("Bad input");
        }
        req.vars[req.names[i]] = value;
    }

    std::getline(in >> std::ws, req.expr);
    req.expr = trim(req.expr);

    if (req.expr.empty()) {
        throw std::runtime_error("Bad expression");
    }

    return req;
}

static Request parse_multiline_request(const std::string& input) {
    std::vector<std::string> lines = split_lines(input);
    size_t pos = 0;

    auto next_nonempty_line = [&]() -> std::string {
        while (pos < lines.size() && trim(lines[pos]).empty()) {
            ++pos;
        }
        if (pos >= lines.size()) {
            throw std::runtime_error("Bad input");
        }
        return trim(lines[pos++]);
    };

    Request req;

    req.command = to_lower_copy(next_nonempty_line());
    validate_command(req.command);

    {
        std::string n_line = next_nonempty_line();
        std::istringstream ns(n_line);

        if (!(ns >> req.n) || req.n < 0) {
            throw std::runtime_error("Bad input");
        }

        ns >> std::ws;
        if (!ns.eof()) {
            throw std::runtime_error("Bad input");
        }
    }

    req.names.resize(req.n);
    std::set<std::string> used_names;

    if (req.n > 0) {
        std::string names_line = next_nonempty_line();
        std::istringstream names_in(names_line);

        for (int i = 0; i < req.n; ++i) {
            if (!(names_in >> req.names[i])) {
                throw std::runtime_error("Bad input");
            }

            req.names[i] = to_lower_copy(req.names[i]);
            validate_variable_name(req.names[i]);

            if (!used_names.insert(req.names[i]).second) {
                throw std::runtime_error("Duplicate variable: " + req.names[i]);
            }
        }

        names_in >> std::ws;
        if (!names_in.eof()) {
            throw std::runtime_error("Bad input");
        }
    }

    if (req.n > 0) {
        std::string values_line = next_nonempty_line();
        std::istringstream values_in(values_line);

        for (int i = 0; i < req.n; ++i) {
            double value;
            if (!(values_in >> value)) {
                throw std::runtime_error("Bad input");
            }
            req.vars[req.names[i]] = value;
        }

        values_in >> std::ws;
        if (!values_in.eof()) {
            throw std::runtime_error("Bad input");
        }
    }

    req.expr = next_nonempty_line();
    if (req.expr.empty()) {
        throw std::runtime_error("Bad expression");
    }

    return req;
}

static std::vector<Request> parse_requests(const std::string& input) {
    if (input.find(';') != std::string::npos) {
        std::vector<std::string> parts = split_by_semicolon(input);
        if (parts.empty()) {
            throw std::runtime_error("Bad input");
        }

        std::vector<Request> requests;
        requests.reserve(parts.size());

        for (const std::string& part : parts) {
            requests.push_back(parse_inline_request(part));
        }

        return requests;
    }

    return {parse_multiline_request(input)};
}

static std::string execute_request(const Request& req) {
    std::set<std::string> allowed(req.names.begin(), req.names.end());
    Parser parser(req.expr, allowed);
    NodePtr tree = parser.parse();

    if (req.command == "evaluate") {
        return number_to_string(tree->eval(req.vars));
    }

    if (req.command == "derivative") {
        if (req.names.empty()) {
            throw std::runtime_error("No variable for derivative");
        }
        NodePtr d = full_simplify(tree->diff(req.names[0]));
        return d->str();
    }

    if (req.command == "evaluate_derivative") {
        if (req.names.empty()) {
            throw std::runtime_error("No variable for derivative");
        }
        NodePtr d = full_simplify(tree->diff(req.names[0]));
        return number_to_string(d->eval(req.vars));
    }

    throw std::runtime_error("Unsupported command");
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    try {
        std::string input(
            (std::istreambuf_iterator<char>(std::cin)),
            std::istreambuf_iterator<char>()
        );

        if (trim(input).empty()) {
            throw std::runtime_error("Bad input");
        }

        std::vector<Request> requests = parse_requests(input);

        for (size_t i = 0; i < requests.size(); ++i) {
            std::cout << execute_request(requests[i]);
            if (i + 1 < requests.size()) {
                std::cout << '\n';
            }
        }
    } catch (const std::exception& e) {
        std::cout << "ERROR " << e.what();
    }

    return 0;
}
