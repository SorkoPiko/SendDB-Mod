#include "LayoutUtils.hpp"

#include <algorithm>
#include <chrono>
#include <vector>
#include <Geode/loader/Log.hpp>

XAxisLayout LayoutUtils::calculateXAxisLayout(const int startTimestamp, const int timeRange) {
    constexpr int maxDays = 8;
    constexpr int daySeconds = 24 * 60 * 60;
    constexpr int targetTicksPerLabel = 5;

    const int timeRangeDays = timeRange / daySeconds;

    int labelIntervalDays = std::max(1, (timeRangeDays + maxDays - 1) / maxDays);

    constexpr int niceIntervals[] = {1, 2, 3, 5, 7, 10, 14, 15, 20, 30, 60, 90, 120, 180, 365};
    for (const int interval : niceIntervals) {
        if (interval >= labelIntervalDays) {
            labelIntervalDays = interval;
            break;
        }
    }

    const int labelIntervalSeconds = labelIntervalDays * daySeconds;

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

    const int tickIntervalDays = bestDivisor;
    const int tickIntervalSeconds = tickIntervalDays * daySeconds;
    const int gridLineIntervalSeconds = labelIntervalSeconds / 2;

    const auto now = std::chrono::system_clock::now();
    const auto localTime = std::chrono::zoned_time{std::chrono::current_zone(), now};
    const auto offset = localTime.get_info().offset;
    const int timezoneOffsetSeconds = std::chrono::duration_cast<std::chrono::seconds>(offset).count();

    const int localStartSeconds = startTimestamp + timezoneOffsetSeconds;

    const int daysSinceEpoch = localStartSeconds / daySeconds;
    const int alignedLocalMidnight = daysSinceEpoch * daySeconds;

    const int ticksSinceMidnight = (localStartSeconds - alignedLocalMidnight) / tickIntervalSeconds;
    const int alignedLocalSeconds = alignedLocalMidnight + ticksSinceMidnight * tickIntervalSeconds;
    const int alignedStartSeconds = alignedLocalSeconds - timezoneOffsetSeconds;
    const int firstTickOffset = alignedStartSeconds - startTimestamp;

    const int labelsSinceMidnight = (localStartSeconds - alignedLocalMidnight) / labelIntervalSeconds;
    const int alignedLabelLocalSeconds = alignedLocalMidnight + (labelsSinceMidnight * labelIntervalSeconds);
    const int alignedLabelStartSeconds = alignedLabelLocalSeconds - timezoneOffsetSeconds;
    const int firstLabelOffset = alignedLabelStartSeconds - startTimestamp;

    const int gridTicksSinceMidnight = (localStartSeconds - alignedLocalMidnight) / gridLineIntervalSeconds;
    const int alignedGridLocalSeconds = alignedLocalMidnight + gridTicksSinceMidnight * gridLineIntervalSeconds;
    const int alignedGridStartSeconds = alignedGridLocalSeconds - timezoneOffsetSeconds;
    const int firstGridOffset = alignedGridStartSeconds - startTimestamp;

    return {
        tickIntervalSeconds,
        labelIntervalSeconds,
        gridLineIntervalSeconds,
        firstTickOffset,
        firstLabelOffset,
        firstGridOffset
    };
}
