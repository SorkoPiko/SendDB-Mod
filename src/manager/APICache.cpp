#include "APICache.hpp"

BatchLevel fromLevel(const Level& level) {
    return {
        level.levelID,
        static_cast<int32_t>(level.sends.size()),
        level.accurate,
        level.platformer,
        level.length,
        level.rank,
        level.trending_score,
        level.rate
    };
}

std::optional<std::optional<Level>> APICache::getLevel(const int levelID) {
    if (const auto it = levelCache.find(levelID); it != levelCache.end()) {
        const auto& [level, timestamp] = it->second;
        if (const time_t currentTime = std::time(nullptr); currentTime - timestamp <= cacheDuration) {
            return level;
        }
        levelCache.erase(it);
    }
    return std::nullopt;
}

std::optional<std::optional<BatchLevel>> APICache::getBatchLevel(const int levelID) {
    if (const auto it = batchLevelCache.find(levelID); it != batchLevelCache.end()) {
        const auto& [batchLevel, timestamp] = it->second;
        if (const time_t currentTime = std::time(nullptr); currentTime - timestamp <= cacheDuration) {
            return batchLevel;
        }
        batchLevelCache.erase(it);
    }

    const std::optional<std::optional<Level>> levelOpt = getLevel(levelID);
    if (levelOpt.has_value()) {
        const auto level = levelOpt.value();
        if (level.has_value()) {
            return fromLevel(level.value());
        }
        return std::nullopt;
    }

    return std::nullopt;
}

void APICache::cacheLevel(const int levelID, const std::optional<Level>& level) {
    const time_t currentTime = std::time(nullptr);
    levelCache[levelID] = CachedLevel{level, currentTime};
}

void APICache::cacheBatchLevel(const int levelID, const std::optional<BatchLevel>& batchLevel) {
    const time_t currentTime = std::time(nullptr);
    batchLevelCache[levelID] = CachedBatchLevel{batchLevel, currentTime};
}