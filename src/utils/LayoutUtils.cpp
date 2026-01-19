#include "LayoutUtils.hpp"

#include <algorithm>
#include <chrono>
#include <vector>
#include <Geode/loader/Log.hpp>

ChartAxisLayout LayoutUtils::calculateTimeAxisLayout(const int startTimestamp, const int timeRange) {
    constexpr int maxDays = 7;
    constexpr int targetTicksPerLabel = 5;

    constexpr int halfDaysPerDay = 2;
    constexpr int secondsPerHalfDay = 12 * 60 * 60;

    const int timeRangeHalfDays = timeRange / secondsPerHalfDay;
    const int timeRangeDays = timeRangeHalfDays / halfDaysPerDay;

    int labelIntervalDays = std::max(1, (timeRangeDays + maxDays - 1) / maxDays);

    constexpr int niceIntervals[] = {1, 2, 3, 5, 7, 10, 14, 15, 20, 30, 60, 90, 120, 180, 365};
    for (const int interval : niceIntervals) {
        if (interval >= labelIntervalDays) {
            labelIntervalDays = interval;
            break;
        }
    }

    while (timeRangeDays / labelIntervalDays >= maxDays) {
        auto it = std::ranges::find(niceIntervals, labelIntervalDays);
        if (it != std::end(niceIntervals) - 1) {
            labelIntervalDays = *++it;
        } else {
            labelIntervalDays += 30;
        }
    }

    const int labelIntervalHalfDays = labelIntervalDays * halfDaysPerDay;

    std::vector<int> divisors;
    for (int i = 1; i <= labelIntervalDays; i++) {
        if (labelIntervalDays % i == 0) {
            divisors.push_back(i);
        }
    }

    int bestDivisor = 1;
    int bestDiff = std::abs(labelIntervalDays / 1 - targetTicksPerLabel);

    for (const int divisor : divisors) {
        const int ticksPerLabel = labelIntervalDays / divisor;
        if (const int diff = std::abs(ticksPerLabel - targetTicksPerLabel); diff < bestDiff) {
            bestDiff = diff;
            bestDivisor = divisor;
        }
    }

    const int tickIntervalHalfDays = bestDivisor * halfDaysPerDay;
    const int gridLineIntervalHalfDays = labelIntervalHalfDays / 2;

    const auto now = std::chrono::system_clock::now();

#ifdef _WIN32
    const auto localTime = std::chrono::zoned_time{std::chrono::current_zone(), now};
    const auto offset = localTime.get_info().offset;
    const int timezoneOffsetSeconds = std::chrono::duration_cast<std::chrono::seconds>(offset).count();
#else
    time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    tm local_tm;
    localtime_r(&now_time_t, &local_tm);
    const int timezoneOffsetSeconds = static_cast<int>(local_tm.tm_gmtoff);
#endif

    const int timezoneOffsetHalfDays = timezoneOffsetSeconds / secondsPerHalfDay;
    const int startHalfDays = startTimestamp / secondsPerHalfDay;
    const int localStartHalfDays = startHalfDays + timezoneOffsetHalfDays;

    const int daysSinceEpoch = localStartHalfDays / halfDaysPerDay;
    const int alignedLocalMidnight = daysSinceEpoch * halfDaysPerDay;

    const int ticksSinceMidnight = (localStartHalfDays - alignedLocalMidnight) / tickIntervalHalfDays;
    const int alignedLocalTick = alignedLocalMidnight + ticksSinceMidnight * tickIntervalHalfDays;
    const int firstTickOffset = alignedLocalTick - timezoneOffsetHalfDays - startHalfDays;

    const int labelsSinceMidnight = (localStartHalfDays - alignedLocalMidnight) / labelIntervalHalfDays;
    const int alignedLocalLabel = alignedLocalMidnight + labelsSinceMidnight * labelIntervalHalfDays;
    const int firstLabelOffset = alignedLocalLabel - timezoneOffsetHalfDays - startHalfDays;

    const int gridTicksSinceMidnight = (localStartHalfDays - alignedLocalMidnight) / gridLineIntervalHalfDays;
    const int alignedLocalGrid = alignedLocalMidnight + gridTicksSinceMidnight * gridLineIntervalHalfDays;
    const int firstGridOffset = alignedLocalGrid - timezoneOffsetHalfDays - startHalfDays;

    return {
        secondsPerHalfDay,
        {
            tickIntervalHalfDays,
            firstTickOffset
        },
        {
            labelIntervalHalfDays,
            firstLabelOffset
        },
        {
            gridLineIntervalHalfDays,
            firstGridOffset
        }
    };
}

int LayoutUtils::minStartOffset(ChartAxisLayout layout) {
    return std::min({
        layout.tick.startOffset,
        layout.label.startOffset,
        layout.gridLine.startOffset
    });
}