#ifndef SENDDB_FORMATUTILS_HPP
#define SENDDB_FORMATUTILS_HPP

#include <string>

class FormatUtils {
public:
    static std::string formatFloat(double value, int decimalPlaces, const std::string& extra = "");
};

#endif