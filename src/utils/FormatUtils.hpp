#pragma once

#include <string>

class FormatUtils {
public:
    static std::string formatFloat(double value, int decimalPlaces, const std::string& extra = "");
};