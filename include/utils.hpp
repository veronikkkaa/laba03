#pragma once

#include <string>

std::string to_lower_copy(const std::string& s);

bool is_builtin_function(const std::string& name);

std::string number_to_string(double value);

void ensure_finite(double value, const std::string& message);
