#pragma once

#include <string>
#include "token.hpp"

class Lexer {
private:
    std::string s;
    size_t pos = 0; 

    void skip_spaces(); 
    Token read_number();
    Token read_identifier();

public:
    explicit Lexer(const std::string& input);

    Token next();
};
