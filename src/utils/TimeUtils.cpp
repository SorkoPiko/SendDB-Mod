#include "TimeUtils.hpp"

#include <time.h>
#include <Geode/loader/Log.hpp>

std::string plural(const int num) {
    static std::string s;
    s = num == 1 ? "" : "s";
    return s;
}

std::string ago(const int num, const std::string& unit) {
    static std::string s;
    s = std::to_string(num) + " " + unit + (num == 1 ? "" : "s") + " ago";
    return s;
}

tm getLocalTime(const long long timestamp) {
    tm timeInfo;
    time_t t = timestamp / 1000;
#ifdef _WIN32
    localtime_s(&timeInfo, &t);
#else
    localtime_r(&t, &timeInfo);
#endif
    return timeInfo;
}

long long TimeUtils::getCurrentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return nowMs;
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

std::string TimeUtils::timestampAgo(const long long timestamp) {
    const auto seconds = timestamp / 1000;

    const auto now = std::chrono::system_clock::now();
    const auto sendTime = std::chrono::system_clock::from_time_t(seconds);
    const auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - sendTime).count();

    std::string relativeTime;
    if (diff < 60) {
        relativeTime = ago(diff, "second");
    } else if (diff < 60 * 60) {
        relativeTime = ago(diff / 60, "minute");
    } else if (diff < 24 * 60 * 60) {
        relativeTime = ago(diff / (60 * 60), "hour");
    } else if (diff < 30 * 24 * 60 * 60) {
        relativeTime = ago(diff / (24 * 60 * 60), "day");
    } else if (diff < 365 * 24 * 60 * 60) {
        relativeTime = ago(diff / (30 * 24 * 60 * 60), "month");
    } else {
        relativeTime = ago(diff / (365 * 24 * 60 * 60), "year");
    }
    return relativeTime;
}