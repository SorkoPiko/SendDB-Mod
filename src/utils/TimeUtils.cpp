#include "TimeUtils.hpp"

#include <time.h>

tm getLocalTime(const long long timestamp) {
    const auto timeT = timestamp / 1000;
    tm timeInfo;
    time_t t = timeT;
#ifdef _WIN32
    localtime_s(&timeInfo, &t);
#else
    localtime_r(&t, &timeInfo);
#endif
    return timeInfo;
}

std::string TimeUtils::timestampToDateTime(const long long timestamp) {
    const auto timeInfo = getLocalTime(timestamp);

    char timeBuffer[64];
    strftime(timeBuffer, sizeof(timeBuffer), "%I:%M%p %d/%m/%Y", &timeInfo);
    return timeBuffer;
}

std::string TimeUtils::timestampToDate(const long long timestamp) {
    const auto timeInfo = getLocalTime(timestamp);

    char timeBuffer[64];
    strftime(timeBuffer, sizeof(timeBuffer), "%d/%m/%Y", &timeInfo);
    return timeBuffer;
}