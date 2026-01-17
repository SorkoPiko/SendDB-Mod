#ifndef SENDDB_APICACHE_HPP
#define SENDDB_APICACHE_HPP

#include <map>
#include <Geode/Prelude.hpp>

#include <model/APIResponse.hpp>

using namespace geode::prelude;

struct CachedLevel {
    std::optional<Level> level;
    int64_t timestamp;
};

struct CachedBatchLevel {
    std::optional<BatchLevel> batchLevel;
    int64_t timestamp;
};

class APICache {
    int cacheDuration = 300;
    std::map<int, CachedLevel> levelCache;
    std::map<int, CachedBatchLevel> batchLevelCache;

public:
    explicit APICache(const int duration) : cacheDuration(duration) {}

    std::optional<std::optional<Level>> getLevel(int levelID);

    std::optional<std::optional<BatchLevel>> getBatchLevel(int levelID);
    void cacheLevel(int levelID, const std::optional<Level>& level);
    void cacheBatchLevel(int levelID, const std::optional<BatchLevel>& batchLevel);

    void setCacheDuration(const int duration) {
        cacheDuration = duration;
    }
};

#endif