#ifndef SENDDB_APIREQUEST_HPP
#define SENDDB_APIREQUEST_HPP

#include <Geode/Prelude.hpp>
#include <matjson/std.hpp>

#include <vector>

using namespace geode::prelude;

struct BatchRequest {
    std::vector<int32_t> level_ids;
};

enum class RateFilter {
    Rated,
    Unrated
};

enum class GamemodeFilter {
    Classic,
    Platformer
};

struct LeaderboardQuery {
    int32_t limit;
    int32_t offset;
    std::optional<RateFilter> rateFilter;
    std::optional<GamemodeFilter> gamemodeFilter;

    bool operator==(const LeaderboardQuery& other) const {
        return limit == other.limit &&
               offset == other.offset &&
               rateFilter == other.rateFilter &&
               gamemodeFilter == other.gamemodeFilter;
    }
};

template <>
struct matjson::Serialize<BatchRequest> {
    static Value toJson(const BatchRequest& request) {
        return makeObject({
            { "level_ids", request.level_ids }
        });
    }
};

template <>
struct matjson::Serialize<RateFilter> {
    static Value toJson(const RateFilter& filter) {
        switch (filter) {
            case RateFilter::Rated:
                return "Rated";
            case RateFilter::Unrated:
                return "Unrated";
        }
        return nullptr;
    }
};

template <>
struct matjson::Serialize<GamemodeFilter> {
    static Value toJson(const GamemodeFilter& filter) {
        switch (filter) {
            case GamemodeFilter::Classic:
                return "Classic";
            case GamemodeFilter::Platformer:
                return "Platformer";
        }
        return nullptr;
    }
};

template <>
struct matjson::Serialize<LeaderboardQuery> {
    static Value toJson(const LeaderboardQuery& filter) {
        Value obj = makeObject({
            { "limit", filter.limit },
            { "offset", filter.offset }
        });

        if (filter.rateFilter.has_value()) {
            obj["rate_filter"] = filter.rateFilter.value();
        }
        if (filter.gamemodeFilter.has_value()) {
            obj["gamemode_filter"] = filter.gamemodeFilter.value();
        }

        return obj;
    }
};

#endif