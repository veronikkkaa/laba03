#include "utils.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>

std::string to_lower_copy(const std::string& s) {
    std::string result = s;

    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) {
                       return static_cast<char>(std::tolower(c));
                   });

    return result;
}

bool is_builtin_function(const std::string& name) {
    static const std::set<std::string> builtin = {
        "sin", "cos", "tan",
        "asin", "acos", "atan",
        "exp", "log", "sqrt"
    };

    return builtin.count(to_lower_copy(name)) > 0;
}

std::string number_to_string(double value) {
    std::ostringstream out;

    out << std::setprecision(10) << value;

    std::string s = out.str();

    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') {
            s.pop_back();
        }
        if (!s.empty() && s.back() == '.') {
            s.pop_back();
        }
    }

    if (s.empty()) {
        return "0";
    }

    return s;
}
