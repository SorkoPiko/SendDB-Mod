#pragma once

#include <unordered_map>
#include <Geode/Prelude.hpp>
#include <model/APIRequest.hpp>

#include <model/APIResponse.hpp>
#include <utils/HashUtils.hpp>

using namespace geode::prelude;

struct LeaderboardKey {
    std::optional<RateFilter> rateFilter;
    std::optional<GamemodeFilter> gamemodeFilter;

    bool operator==(const LeaderboardKey& other) const {
        return rateFilter == other.rateFilter && gamemodeFilter == other.gamemodeFilter;
    }

    static LeaderboardKey fromQuery(const LeaderboardQuery& query) {
        return {
            query.rateFilter,
            query.gamemodeFilter
        };
    }
};

struct LeaderboardLevelKey {
    int index;
    LeaderboardKey leaderboardKey;

    bool operator==(const LeaderboardLevelKey& other) const {
        return index == other.index && leaderboardKey == other.leaderboardKey;
    }
};

template <>
struct std::hash<LeaderboardKey> {
    size_t operator()(const LeaderboardKey& key) const noexcept {
        size_t seed = 0;
        hash_combine(seed, key.rateFilter);
        hash_combine(seed, key.gamemodeFilter);
        return seed;
    }
};

template <>
struct std::hash<LeaderboardLevelKey> {
    size_t operator()(const LeaderboardLevelKey& key) const noexcept {
        const size_t h1 = std::hash<int>()(key.index);
        const size_t h2 = std::hash<LeaderboardKey>()(key.leaderboardKey);

        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

template <typename T>
struct OptionalCacheEntry {
    std::optional<T> data;
    int64_t timestamp;
};

template <typename T>
struct CacheEntry {
    T data;
    int64_t timestamp;
};

class APICache {
    int cacheDuration;
    std::unordered_map<int, OptionalCacheEntry<Level>> levelCache;
    std::unordered_map<int, OptionalCacheEntry<BatchLevel>> batchLevelCache;
    std::unordered_map<int, OptionalCacheEntry<Creator>> creatorCache;

    std::unordered_map<LeaderboardKey, CacheEntry<int>> leaderboardCountCache;
    std::unordered_map<LeaderboardLevelKey, CacheEntry<LeaderboardLevel>> leaderboardLevelCache;

    CacheEntry<int> trendingLeaderboardCountCache;
    std::unordered_map<int, CacheEntry<TrendingLeaderboardLevel>> trendingLeaderboardLevelCache;

public:
    explicit APICache(const int duration) : cacheDuration(duration) {}

    void cacheLevel(int levelID, const std::optional<Level>& level);
    void cacheBatchLevel(int levelID, const std::optional<BatchLevel>& batchLevel);
    void cacheCreator(int creatorID, const std::optional<Creator>& creator);
    void cacheLeaderboard(const LeaderboardQuery& query, const LeaderboardResponse& response);
    void cacheTrendingLeaderboard(const TrendingLeaderboardQuery& query, const TrendingLeaderboardResponse& response);

    std::optional<std::optional<Level>> getLevel(int levelID) const;
    std::optional<std::optional<BatchLevel>> getBatchLevel(int levelID) const;
    std::optional<std::optional<Creator>> getCreator(int creatorID) const;

    std::optional<LeaderboardResponse> getLeaderboard(const LeaderboardQuery& query) const;
    std::optional<TrendingLeaderboardResponse> getTrendingLeaderboard(const TrendingLeaderboardQuery& query) const;

    void setCacheDuration(const int duration) {
        cacheDuration = duration;
    }
};