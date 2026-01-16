#include "TimeUtils.hpp"

std::string TimeUtils::timestampToDate(const long long timestamp) {
    const auto timeT = timestamp / 1000;
    tm timeInfo;
#ifdef _WIN32
    localtime_s(&timeInfo, &timeT);
#else
    localtime_r(&timeT, &timeInfo);
#endif

    char timeBuffer[64];
    strftime(timeBuffer, sizeof(timeBuffer), "%I:%M%p %d/%m/%Y", &timeInfo);
    return timeBuffer;
}