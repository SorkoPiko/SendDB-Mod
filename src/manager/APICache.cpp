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

void APICache::cacheLevel(const int levelID, const std::optional<Level>& level) {
    const time_t currentTime = std::time(nullptr);
    levelCache[levelID] = CacheEntry{level, currentTime};
}

void APICache::cacheBatchLevel(const int levelID, const std::optional<BatchLevel>& batchLevel) {
    const time_t currentTime = std::time(nullptr);
    batchLevelCache[levelID] = CacheEntry{batchLevel, currentTime};
}

void APICache::cacheCreator(const int creatorID, const std::optional<Creator>& creator) {
    const time_t currentTime = std::time(nullptr);
    creatorCache[creatorID] = CacheEntry{creator, currentTime};
}

template <typename T>
std::optional<std::optional<T>> getCachedEntry(
    const std::map<int, CacheEntry<T>>& cache,
    const int id,
    const int cacheDuration
) {
    if (const auto it = cache.find(id); it != cache.end()) {
        const auto& [data, timestamp] = it->second;
        if (const time_t currentTime = std::time(nullptr); currentTime - timestamp <= cacheDuration) {
            return data;
        }
    }
    return std::nullopt;
}

std::optional<std::optional<Level>> APICache::getLevel(const int levelID) const {
    return getCachedEntry(levelCache, levelID, cacheDuration);
}

std::optional<std::optional<BatchLevel>> APICache::getBatchLevel(const int levelID) const {
    if (const auto batchLevelOpt = getCachedEntry(batchLevelCache, levelID, cacheDuration)) {
        return batchLevelOpt;
    }

    const std::optional<std::optional<Level>> levelOpt = getLevel(levelID);
    if (levelOpt.has_value()) {
        const auto& level = levelOpt.value();
        if (level.has_value()) {
            return fromLevel(level.value());
        }
        return std::nullopt;
    }

    return std::nullopt;
}

std::optional<std::optional<Creator>> APICache::getCreator(const int creatorID) const {
    return getCachedEntry(creatorCache, creatorID, cacheDuration);
}
