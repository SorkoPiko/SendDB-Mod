#include "FormatUtils.hpp"

#include <fmt/format.h>

std::string FormatUtils::formatFloat(const double value, const int decimalPlaces, const std::string& extra) {
    const std::string format = "{:" + extra + "." + std::to_string(decimalPlaces) + "f}";
    std::string formatted = fmt::vformat(format, fmt::make_format_args(value));
    while (formatted.back() == '0') {
        formatted.pop_back();
    }
    if (formatted.back() == '.') formatted.pop_back();
    return formatted;
}