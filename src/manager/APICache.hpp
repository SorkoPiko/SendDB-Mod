#ifndef SENDDB_APICACHE_HPP
#define SENDDB_APICACHE_HPP

#include <map>
#include <Geode/Prelude.hpp>

#include <model/APIResponse.hpp>

using namespace geode::prelude;

struct CachedLevel {
    Level level;
    int64_t timestamp;
};

class APICache {
    int cacheDuration = 300;
    std::map<int, CachedLevel> levelCache;

public:
    explicit APICache(const int duration) : cacheDuration(duration) {}

    std::optional<Level> getLevel(int levelID);
    void cacheLevel(const Level& level);

    void setCacheDuration(const int duration) {
        cacheDuration = duration;
    }
};

#endif