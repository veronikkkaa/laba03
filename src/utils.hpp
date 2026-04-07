#include "utils.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <set>
#include <stdexcept>

std::string to_lower_copy(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

bool is_builtin_function(const std::string& s) {
    static const std::set<std::string> funcs = {
        "sin", "cos", "tan",
        "asin", "acos", "atan",
        "exp", "log", "sqrt"
    };
    return funcs.count(to_lower_copy(s)) > 0;
}

std::string number_to_string(double x) {
    if (std::fabs(x) < 1e-12) x = 0.0;

    std::ostringstream out;
    out << std::setprecision(15) << x;
    std::string s = out.str();

    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
    }

    if (s == "-0") s = "0";
    if (s.empty()) s = "0";

    return s;
}

void ensure_finite(double x, const std::string& msg) {
    if (std::isnan(x) || std::isinf(x)) {
        throw std::runtime_error(msg);
    }
}
