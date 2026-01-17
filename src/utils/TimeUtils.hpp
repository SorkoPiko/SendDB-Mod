#ifndef SENDDB_TIMEUTILS_HPP
#define SENDDB_TIMEUTILS_HPP

#include <string>

class TimeUtils {
public:
    static std::string timestampToDateTime(long long timestamp);
    static std::string timestampToDate(long long timestamp);
};

#endif