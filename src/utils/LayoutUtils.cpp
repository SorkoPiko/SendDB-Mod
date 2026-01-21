#include "LayoutUtils.hpp"

#include <algorithm>
#include <chrono>
#include <vector>
#include <Geode/loader/Log.hpp>

ChartAxisLayout LayoutUtils::calculateTimeAxisLayout(const int minTimestamp, const int timeRange) {
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

    const int startHalfDays = minTimestamp / secondsPerHalfDay;

    const int daysSinceEpoch = startHalfDays / halfDaysPerDay;
    const int alignedLocalMidnight = daysSinceEpoch * halfDaysPerDay;

    const int ticksSinceMidnight = (startHalfDays - alignedLocalMidnight) / tickIntervalHalfDays;
    const int alignedLocalTick = alignedLocalMidnight + ticksSinceMidnight * tickIntervalHalfDays;

    const int labelsSinceMidnight = (startHalfDays - alignedLocalMidnight) / labelIntervalHalfDays;
    const int alignedLocalLabel = alignedLocalMidnight + labelsSinceMidnight * labelIntervalHalfDays;

    const int gridTicksSinceMidnight = (startHalfDays - alignedLocalMidnight) / gridLineIntervalHalfDays;
    const int alignedLocalGrid = alignedLocalMidnight + gridTicksSinceMidnight * gridLineIntervalHalfDays;

    const int firstTickTimestamp = alignedLocalTick * secondsPerHalfDay;
    const int firstLabelTimestamp = alignedLocalLabel * secondsPerHalfDay;
    const int firstGridTimestamp = alignedLocalGrid * secondsPerHalfDay;

    const float globalStartOffset = static_cast<float>(firstTickTimestamp);

    int tickStartOffset = 0;
    int labelStartOffset = (firstLabelTimestamp - firstTickTimestamp) / secondsPerHalfDay;
    int gridLineStartOffset = (firstGridTimestamp - firstTickTimestamp) / secondsPerHalfDay;

    const int tzOffsetHalfDays = timezoneOffsetSeconds / secondsPerHalfDay;

    // idk
    while (tickStartOffset < tzOffsetHalfDays) {
        tickStartOffset += tickIntervalHalfDays;
    }

    while (labelStartOffset < tzOffsetHalfDays) {
        labelStartOffset += labelIntervalHalfDays;
    }

    while (gridLineStartOffset < tzOffsetHalfDays) {
        gridLineStartOffset += gridLineIntervalHalfDays;
    }

    return {
        static_cast<float>(secondsPerHalfDay),
        globalStartOffset - timezoneOffsetSeconds,
        {
            tickIntervalHalfDays,
            tickStartOffset
        },
        {
            labelIntervalHalfDays,
            labelStartOffset
        },
        {
            gridLineIntervalHalfDays,
            gridLineStartOffset
        }
    };
}

ChartAxisLayout LayoutUtils::calculateNumericAxisLayout(const float minValue, const float maxValue) {
    const float range = maxValue - minValue;

    if (range <= 0) {
        return {1.0f, 0.0f, {1, 0}, {10, 0}, {5, 0}};
    }

    constexpr int targetLabels = 7;

    const float roughInterval = range / targetLabels;
    const float magnitude = std::pow(10.0f, std::floor(std::log10(roughInterval)));
    const float normalized = roughInterval / magnitude;

    float niceNormalized;
    if (normalized <= 1.0f) {
        niceNormalized = 1.0f;
    } else if (normalized <= 2.0f) {
        niceNormalized = 2.0f;
    } else if (normalized <= 2.5f) {
        niceNormalized = 2.5f;
    } else if (normalized <= 5.0f) {
        niceNormalized = 5.0f;
    } else {
        niceNormalized = 10.0f;
    }

    const float labelInterval = niceNormalized * magnitude;

    float unit;
    int labelIntervalUnits;
    int tickIntervalUnits;
    int gridLineIntervalUnits;

    if (niceNormalized == 1.0f) {
        unit = magnitude / 5.0f;
        labelIntervalUnits = 5;
        tickIntervalUnits = 1;
        gridLineIntervalUnits = 2;
    } else if (niceNormalized == 2.0f) {
        unit = magnitude / 2.0f;
        labelIntervalUnits = 4;
        tickIntervalUnits = 1;
        gridLineIntervalUnits = 2;
    } else if (niceNormalized == 2.5f) {
        unit = magnitude / 2.0f;
        labelIntervalUnits = 5;
        tickIntervalUnits = 1;
        gridLineIntervalUnits = 2;
    } else if (niceNormalized == 5.0f) {
        unit = magnitude;
        labelIntervalUnits = 5;
        tickIntervalUnits = 1;
        gridLineIntervalUnits = 2;
    } else {
        unit = magnitude;
        labelIntervalUnits = 10;
        tickIntervalUnits = 2;
        gridLineIntervalUnits = 5;
    }

    const float firstLabel = std::floor(minValue / labelInterval) * labelInterval;
    const float firstTick = std::floor(minValue / (tickIntervalUnits * unit)) * (tickIntervalUnits * unit);
    const float firstGridLine = std::floor(minValue / (gridLineIntervalUnits * unit)) * (gridLineIntervalUnits * unit);

    const float globalStartOffset = firstTick;

    const int labelStartOffset = static_cast<int>(std::round((firstLabel - globalStartOffset) / unit));
    const int tickStartOffset = 0;
    const int gridLineStartOffset = static_cast<int>(std::round((firstGridLine - globalStartOffset) / unit));

    return {
        unit, globalStartOffset,
        {tickIntervalUnits, tickStartOffset},
        {labelIntervalUnits, labelStartOffset},
        {gridLineIntervalUnits, gridLineStartOffset}
    };
}

int LayoutUtils::minStartOffset(ChartAxisLayout layout) {
    return std::min({
        layout.tick.startOffset,
        layout.label.startOffset,
        layout.gridLine.startOffset
    });
}