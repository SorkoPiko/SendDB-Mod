#ifndef SENDDB_APIRESPONSE_HPP
#define SENDDB_APIRESPONSE_HPP

#include <Geode/Prelude.hpp>
#include <matjson.hpp>

#include <optional>
#include <vector>

using namespace geode::prelude;

struct Send {
    int64_t timestamp;

    bool operator==(const Send& other) const {
        return timestamp == other.timestamp;
    }
};

struct CreatorLevel {
    int32_t levelID;
    int32_t send_count;
};

struct Rate {
    int32_t difficulty;
    int32_t points;
    int32_t stars;
    int64_t timestamp;
    bool accurate;
};

struct Level {
    int32_t levelID;
    std::vector<Send> sends;
    bool accurate;
    bool platformer;
    int32_t length;
    double trending_score;
    int32_t rank;
    int32_t rate_rank;
    int32_t gamemode_rank;
    int32_t joined_rank;
    int32_t trending_rank;
    std::optional<Rate> rate;
};

struct BatchLevel {
    int32_t levelID;
    int32_t send_count;
    bool accurate;
    bool platformer;
    int32_t length;
    int32_t rank;
    double trending_score;
    std::optional<Rate> rate;
};

struct Creator {
    int32_t playerID;
    int32_t accountID;
    std::vector<CreatorLevel> levels;
    int32_t send_count;
    int32_t recent_sends;
    double send_count_stddev;
    double trending_score;
    int32_t trending_level_count;
    int64_t latest_send;
    int32_t rank;
    int32_t trending_rank;
};

struct BatchResponse {
    std::vector<BatchLevel> levels;
};

struct LeaderboardLevel {
    int32_t levelID;
    int32_t send_count;
    int32_t rank;
};

struct LeaderboardResponse {
    int32_t total;
    std::vector<LeaderboardLevel> levels;
};

template <>
struct matjson::Serialize<Send> {
    static Result<Send> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int64_t timestamp, value["timestamp"].asInt());
        return Ok(Send { timestamp });
    }
};

template <>
struct matjson::Serialize<CreatorLevel> {
    static Result<CreatorLevel> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int32_t levelID, value["level_id"].asInt());
        GEODE_UNWRAP_INTO(const int32_t send_count, value["send_count"].asInt());
        return Ok(CreatorLevel { levelID, send_count });
    }
};

template <>
struct matjson::Serialize<Rate> {
    static Result<Rate> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int32_t difficulty, value["difficulty"].asInt());
        GEODE_UNWRAP_INTO(const int32_t points, value["points"].asInt());
        GEODE_UNWRAP_INTO(const int32_t stars, value["stars"].asInt());
        GEODE_UNWRAP_INTO(const int64_t timestamp, value["timestamp"].asInt());
        GEODE_UNWRAP_INTO(const bool accurate, value["accurate"].asBool());
        return Ok(Rate { difficulty, points, stars, timestamp, accurate });
    }
};

template <>
struct matjson::Serialize<Level> {
    static Result<Level> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int32_t levelID, value["level_id"].asInt());
        GEODE_UNWRAP_INTO(std::vector<Value> sends_json, value["sends"].asArray());
        std::vector<Send> sends;
        for (const Value& send_val : sends_json) {
            GEODE_UNWRAP_INTO(const Send send, send_val.as<Send>());
            sends.push_back(send);
        }
        GEODE_UNWRAP_INTO(const bool accurate, value["accurate"].asBool());
        GEODE_UNWRAP_INTO(const bool platformer, value["platformer"].asBool());
        GEODE_UNWRAP_INTO(const int32_t length, value["length"].asInt());
        GEODE_UNWRAP_INTO(const double trending_score, value["trending_score"].asDouble());
        GEODE_UNWRAP_INTO(const int32_t rank, value["rank"].asInt());
        GEODE_UNWRAP_INTO(const int32_t rate_rank, value["rate_rank"].asInt());
        GEODE_UNWRAP_INTO(const int32_t gamemode_rank, value["gamemode_rank"].asInt());
        GEODE_UNWRAP_INTO(const int32_t joined_rank, value["joined_rank"].asInt());
        GEODE_UNWRAP_INTO(const int32_t trending_rank, value["trending_rank"].asInt());

        std::optional<Rate> rate;
        if (value.contains("rate") && !value["rate"].isNull()) {
            GEODE_UNWRAP_INTO(const Rate r, value["rate"].as<Rate>());
            rate = r;
        }

        return Ok(Level { levelID, sends, accurate, platformer, length, trending_score, rank, rate_rank, gamemode_rank, joined_rank, trending_rank, rate });
    }
};

template <>
struct matjson::Serialize<BatchLevel> {
    static Result<BatchLevel> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int32_t levelID, value["level_id"].asInt());
        GEODE_UNWRAP_INTO(const int32_t send_count, value["send_count"].asInt());
        GEODE_UNWRAP_INTO(const bool accurate, value["accurate"].asBool());
        GEODE_UNWRAP_INTO(const bool platformer, value["platformer"].asBool());
        GEODE_UNWRAP_INTO(const int32_t length, value["length"].asInt());
        GEODE_UNWRAP_INTO(const int32_t rank, value["rank"].asInt());
        GEODE_UNWRAP_INTO(const double trending_score, value["trending_score"].asDouble());

        std::optional<Rate> rate;
        if (value.contains("rate") && !value["rate"].isNull()) {
            GEODE_UNWRAP_INTO(const Rate r, value["rate"].as<Rate>());
            rate = r;
        }

        return Ok(BatchLevel { levelID, send_count, accurate, platformer, length, rank, trending_score, rate });
    }
};

template <>
struct matjson::Serialize<Creator> {
    static Result<Creator> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int32_t playerID, value["player_id"].asInt());
        GEODE_UNWRAP_INTO(const int32_t accountID, value["account_id"].asInt());
        GEODE_UNWRAP_INTO(std::vector<Value> levels_json, value["levels"].asArray());
        std::vector<CreatorLevel> levels;
        for (const Value& level_val : levels_json) {
            GEODE_UNWRAP_INTO(const CreatorLevel level, level_val.as<CreatorLevel>());
            levels.push_back(level);
        }
        GEODE_UNWRAP_INTO(const int32_t send_count, value["send_count"].asInt());
        GEODE_UNWRAP_INTO(const int32_t recent_sends, value["recent_sends"].asInt());
        GEODE_UNWRAP_INTO(const double send_count_stddev, value["send_count_stddev"].asDouble());
        GEODE_UNWRAP_INTO(const double trending_score, value["trending_score"].asDouble());
        GEODE_UNWRAP_INTO(const int32_t trending_level_count, value["trending_level_count"].asInt());
        GEODE_UNWRAP_INTO(const int64_t latest_send, value["latest_send"].asInt());
        GEODE_UNWRAP_INTO(const int32_t rank, value["rank"].asInt());
        GEODE_UNWRAP_INTO(const int32_t trending_rank, value["trending_rank"].asInt());
        return Ok(Creator { playerID, accountID, levels, send_count, recent_sends, send_count_stddev, trending_score, trending_level_count, latest_send, rank, trending_rank });
    }
};

template <>
struct matjson::Serialize<BatchResponse> {
    static Result<BatchResponse> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const std::vector<Value> levels_json, value["levels"].asArray());
        std::vector<BatchLevel> levels;
        for (const Value& level_val : levels_json) {
            GEODE_UNWRAP_INTO(const BatchLevel level, level_val.as<BatchLevel>());
            levels.push_back(level);
        }
        return Ok(BatchResponse { levels });
    }
};

template <>
struct matjson::Serialize<LeaderboardLevel> {
    static Result<LeaderboardLevel> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int32_t levelID, value["level_id"].asInt());
        GEODE_UNWRAP_INTO(const int32_t send_count, value["send_count"].asInt());
        GEODE_UNWRAP_INTO(const int32_t rank, value["rank"].asInt());
        return Ok(LeaderboardLevel { levelID, send_count, rank });
    }
};

template <>
struct matjson::Serialize<LeaderboardResponse> {
    static Result<LeaderboardResponse> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int32_t total, value["total"].asInt());
        GEODE_UNWRAP_INTO(const std::vector<Value> levels_json, value["levels"].asArray());
        std::vector<LeaderboardLevel> levels;
        for (const Value& level_val : levels_json) {
            GEODE_UNWRAP_INTO(const LeaderboardLevel level, level_val.as<LeaderboardLevel>());
            levels.push_back(level);
        }
        return Ok(LeaderboardResponse { total, levels });
    }
};

#endif