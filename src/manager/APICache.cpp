#include "APICache.hpp"

std::optional<Level> APICache::getLevel(const int levelID) {
    if (const auto it = levelCache.find(levelID); it != levelCache.end()) {
        const auto& [level, timestamp] = it->second;
        if (const time_t currentTime = std::time(nullptr); currentTime - timestamp <= cacheDuration) {
            return level;
        }
        levelCache.erase(it);
    }
    return std::nullopt;
}

void APICache::cacheLevel(const Level& level) {
    const time_t currentTime = std::time(nullptr);
    levelCache[level.levelID] = CachedLevel{level, currentTime};
}