#include "APICache.hpp"

#include <ctime>
#include <ranges>

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
    levelCache[levelID] = OptionalCacheEntry{ level, currentTime };
}

void APICache::cacheBatchLevel(const int levelID, const std::optional<BatchLevel>& batchLevel) {
    const time_t currentTime = std::time(nullptr);
    batchLevelCache[levelID] = OptionalCacheEntry{ batchLevel, currentTime };
}

void APICache::cacheCreator(const int creatorID, const std::optional<Creator>& creator) {
    const time_t currentTime = std::time(nullptr);
    creatorCache[creatorID] = OptionalCacheEntry{ creator, currentTime };
}

void APICache::cacheLeaderboard(const LeaderboardQuery& query, const LeaderboardResponse& response) {
    const time_t currentTime = std::time(nullptr);
    const LeaderboardKey key = LeaderboardKey::fromQuery(query);
    for (int i = 0; i < response.levels.size(); ++i) {
        const int index = i + query.offset;
        leaderboardLevelCache[{index, key}] = { response.levels[i], currentTime };
    }
    leaderboardCountCache[key] = { response.total, currentTime };
}

void APICache::cacheTrendingLeaderboard(const TrendingLeaderboardQuery& query, const TrendingLeaderboardResponse& response) {
    const time_t currentTime = std::time(nullptr);
    for (int i = 0; i < response.levels.size(); ++i) {
        const int index = i + query.offset;
        trendingLeaderboardLevelCache[index] = { response.levels[i], currentTime };
    }
    trendingLeaderboardCountCache = { response.total, currentTime };
}

void APICache::cacheCreatorLeaderboard(const CreatorLeaderboardQuery& query, const CreatorLeaderboardResponse& response) {
    const time_t currentTime = std::time(nullptr);
    for (int i = 0; i < response.creators.size(); ++i) {
        const int index = i + query.offset;
        leaderboardCreatorCache[index] = { response.creators[i], currentTime };
    }
    creatorLeaderboardCountCache = { response.total, currentTime };
}

template <typename T>
std::optional<std::optional<T>> getCachedEntry(
    const std::unordered_map<int, OptionalCacheEntry<T>>& cache,
    const int id,
    const int cacheDuration
) {
    const time_t currentTime = std::time(nullptr);
    if (const auto it = cache.find(id); it != cache.end()) {
        const auto& [data, timestamp] = it->second;
        if (currentTime - timestamp <= cacheDuration) {
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
    if (levelOpt.has_value() && levelOpt->has_value()) {
        return fromLevel(levelOpt->value());
    }

    return std::nullopt;
}

std::optional<std::optional<Creator>> APICache::getCreator(const int creatorID) const {
    return getCachedEntry(creatorCache, creatorID, cacheDuration);
}

std::optional<std::optional<LeaderboardCreator>> APICache::getLeaderboardCreator(const int creatorID) const {
    const time_t currentTime = std::time(nullptr);
    for (const auto& entry: leaderboardCreatorCache | std::views::values) {
        const auto& [creator, timestamp] = entry;
        if (creator.playerID == creatorID && currentTime - timestamp <= cacheDuration) {
            return creator;
        }
    }

    if (const auto it = leaderboardCreatorCache.find(creatorID); it != leaderboardCreatorCache.end()) {
        const auto& [data, timestamp] = it->second;
        if (currentTime - timestamp <= cacheDuration) {
            return data;
        }
    }

    const std::optional<std::optional<Creator>> creatorOpt = getCreator(creatorID);
    if (creatorOpt.has_value() && creatorOpt->has_value()) {
        const Creator& c = creatorOpt->value();
        return LeaderboardCreator {
            "Unknown",
            c.playerID,
            c.accountID,
            static_cast<int>(c.levels.size()),
            c.send_count,
            c.trending_score,
            c.rank,
            c.trending_rank
        };
    }

    return std::nullopt;
}

std::optional<LeaderboardResponse> APICache::getLeaderboard(const LeaderboardQuery& query) const {
    const time_t currentTime = std::time(nullptr);
    const LeaderboardKey key = LeaderboardKey::fromQuery(query);
    std::vector<LeaderboardLevel> levels;
    for (int i = 0; i < query.limit; ++i) {
        const int index = i + query.offset;
        if (const auto it = leaderboardLevelCache.find({index, key}); it != leaderboardLevelCache.end()) {
            const auto& [level, timestamp] = it->second;
            if (currentTime - timestamp <= cacheDuration) {
                levels.push_back(level);
            } else {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    }

    int total = 0;
    if (const auto it = leaderboardCountCache.find(key); it != leaderboardCountCache.end()) {
        const auto& [count, timestamp] = it->second;
        if (currentTime - timestamp <= cacheDuration) {
            total = count;
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }

    return LeaderboardResponse{ total, levels };
}

std::optional<TrendingLeaderboardResponse> APICache::getTrendingLeaderboard(const TrendingLeaderboardQuery& query) const {
    const time_t currentTime = std::time(nullptr);
    std::vector<TrendingLeaderboardLevel> levels;
    for (int i = 0; i < query.limit; ++i) {
        const int index = i + query.offset;
        if (const auto it = trendingLeaderboardLevelCache.find(index); it != trendingLeaderboardLevelCache.end()) {
            const auto& [level, timestamp] = it->second;
            if (currentTime - timestamp <= cacheDuration) {
                levels.push_back(level);
            } else {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    }

    int total = 0;
    const auto& [count, timestamp] = trendingLeaderboardCountCache;
    if (currentTime - timestamp <= cacheDuration) {
        total = count;
    } else {
        return std::nullopt;
    }

    return TrendingLeaderboardResponse{ total, levels };
}

std::optional<CreatorLeaderboardResponse> APICache::getCreatorLeaderboard(const CreatorLeaderboardQuery& query) const {
    const time_t currentTime = std::time(nullptr);
    std::vector<LeaderboardCreator> creators;
    for (int i = 0; i < query.limit; ++i) {
        const int index = i + query.offset;
        if (const auto it = leaderboardCreatorCache.find(index); it != leaderboardCreatorCache.end()) {
            const auto& [creator, timestamp] = it->second;
            if (currentTime - timestamp <= cacheDuration) {
                creators.push_back(creator);
            } else {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    }

    int total = 0;
    const auto& [count, timestamp] = creatorLeaderboardCountCache;
    if (currentTime - timestamp <= cacheDuration) {
        total = count;
    } else {
        return std::nullopt;
    }

    return CreatorLeaderboardResponse{ total, creators };
}