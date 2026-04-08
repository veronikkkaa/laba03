#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& input) : s(input), pos(0) {}

void Lexer::skip_spaces() {
    while (pos < s.size() &&
           std::isspace(static_cast<unsigned char>(s[pos]))) {
        pos++;
    }
}

Token Lexer::read_number() {
    size_t start = pos;

    // случай 0.xxx
    if (s[pos] == '0') {
        pos++;

        // запрет: 0002, 01
        if (pos < s.size() &&
            std::isdigit(static_cast<unsigned char>(s[pos]))) {
            throw std::runtime_error("Invalid number");
        }

        // дробная часть
        if (pos < s.size() && s[pos] == '.') {
            pos++;

            if (pos >= s.size() ||
                !std::isdigit(static_cast<unsigned char>(s[pos]))) {
                throw std::runtime_error("Invalid number");
            }

            while (pos < s.size() &&
                   std::isdigit(static_cast<unsigned char>(s[pos]))) {
                pos++;
            }
        }

        return {lexem_t::NUMBER, s.substr(start, pos - start)};
    }

    // целая часть
    while (pos < s.size() &&
           std::isdigit(static_cast<unsigned char>(s[pos]))) {
        pos++;
    }

    // дробная часть
    if (pos < s.size() && s[pos] == '.') {
        pos++;

        if (pos >= s.size() ||
            !std::isdigit(static_cast<unsigned char>(s[pos]))) {
            throw std::runtime_error("Invalid number");
        }

        while (pos < s.size() &&
               std::isdigit(static_cast<unsigned char>(s[pos]))) {
            pos++;
        }
    }

    return {lexem_t::NUMBER, s.substr(start, pos - start)};
}

Token Lexer::read_identifier() {
    size_t start = pos;

    while (pos < s.size() &&
           (std::isalnum(static_cast<unsigned char>(s[pos])) ||
            s[pos] == '_')) {
        pos++;
    }

    std::string val = s.substr(start, pos - start);

    for (char& c : val) {
        c = static_cast<char>(
            std::tolower(static_cast<unsigned char>(c)));
    }

    return {lexem_t::IDENT, val};
}

Token Lexer::next() {
    skip_spaces();

    if (pos >= s.size()) {
        return {lexem_t::EOEX, ""};
    }

    char c = s[pos];

    // числа
    if (std::isdigit(static_cast<unsigned char>(c))) {
        return read_number();
    }

    // запрет .5
    if (c == '.') {
        throw std::runtime_error("Invalid number");
    }

    // идентификаторы
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return read_identifier();
    }

    // операции
    if (c == '+' || c == '-' || c == '*' ||
        c == '/' || c == '^') {
        pos++;
        return {lexem_t::OP, std::string(1, c)};
    }

    // скобки
    if (c == '(' || c == ')') {
        pos++;
        return {lexem_t::PAREN, std::string(1, c)};
    }

    // ошибка
    throw std::runtime_error(
        std::string("Unknown symbol: ") + c
    );
}
