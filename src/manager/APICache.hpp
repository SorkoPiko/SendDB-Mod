#ifndef SENDDB_APICACHE_HPP
#define SENDDB_APICACHE_HPP

#include <map>
#include <Geode/Prelude.hpp>

#include <model/APIResponse.hpp>

using namespace geode::prelude;

template <typename T>
struct CacheEntry {
    std::optional<T> data;
    int64_t timestamp;
};

class APICache {
    int cacheDuration;
    std::map<int, CacheEntry<Level>> levelCache;
    std::map<int, CacheEntry<BatchLevel>> batchLevelCache;
    std::map<int, CacheEntry<Creator>> creatorCache;

public:
    explicit APICache(const int duration) : cacheDuration(duration) {}

    void cacheLevel(int levelID, const std::optional<Level>& level);
    void cacheBatchLevel(int levelID, const std::optional<BatchLevel>& batchLevel);
    void cacheCreator(int creatorID, const std::optional<Creator>& creator);

    std::optional<std::optional<Level>> getLevel(int levelID) const;
    std::optional<std::optional<BatchLevel>> getBatchLevel(int levelID) const;
    std::optional<std::optional<Creator>> getCreator(int creatorID) const;

    void setCacheDuration(const int duration) {
        cacheDuration = duration;
    }
};

#endif