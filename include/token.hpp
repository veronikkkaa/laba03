#pragma once

#include <string>

enum class lexem_t {
    NUMBER,
    IDENT,
    OP,
    PAREN,
    EOEX
};

struct Token {
    lexem_t type;       
    std::string value;  

    std::string view() const { 
        return value;
    }
};
