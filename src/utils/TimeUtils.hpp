#pragma once

#include <string>

class TimeUtils {
public:
    static long long getCurrentTimestamp();

    static std::string timestampToDateTime(long long timestamp);
    static std::string timestampToDate(long long timestamp);
    static std::string timestampAgo(long long timestamp);
};