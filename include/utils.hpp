#pragma once

#include <string>

std::string to_lower_copy(std::string s);
bool is_builtin_function(const std::string& s);
std::string number_to_string(double x);
void ensure_finite(double x, const std::string& msg);
