#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& input) : s(input), pos(0) {}

void Lexer::skip_spaces() { //метод скипа пробелов
    while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) {
        pos++;
    }
}

Token Lexer::read_number() {
    size_t start = pos;

    if (s[pos] == '0') {
        pos++;

        if (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {
            throw std::runtime_error("Invalid number");
        }

        if (pos < s.size() && s[pos] == '.') {
            pos++;

            if (pos >= s.size() || !std::isdigit(static_cast<unsigned char>(s[pos]))) {
                throw std::runtime_error("Invalid number");
            }

            while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {
                pos++;
            }
        }

        return {lexem_t::NUMBER, s.substr(start, pos - start)};
    }

    while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {
        pos++;
    }

    if (pos < s.size() && s[pos] == '.') {
        pos++;

        if (pos >= s.size() || !std::isdigit(static_cast<unsigned char>(s[pos]))) {
            throw std::runtime_error("Invalid number");
        }

        while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) {
            pos++;
        }
    }

    return {lexem_t::NUMBER, s.substr(start, pos - start)};
}

Token Lexer::read_identifier() {
    size_t start = pos;

    while (pos < s.size() &&
           (std::isalnum(static_cast<unsigned char>(s[pos])) || s[pos] == '_')) {
        pos++;
    }

    std::string val = s.substr(start, pos - start);

    for (char& c : val) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    return {lexem_t::IDENT, val};
}

Token Lexer::next() {
    skip_spaces();

    if (pos >= s.size()) {
        return {lexem_t::EOEX, ""};
    }

    char c = s[pos];

    if (std::isdigit(static_cast<unsigned char>(c))) {
        return read_number();
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return read_identifier();
    }

    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
        pos++;
        return {lexem_t::OP, std::string(1, c)};
    }

    if (c == '(' || c == ')') {
        pos++;
        return {lexem_t::PAREN, std::string(1, c)};
    }

    throw std::runtime_error("Unknown symbol");
}
