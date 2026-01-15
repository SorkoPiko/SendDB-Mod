#ifndef SENDDB_APIRESPONSE_HPP
#define SENDDB_APIRESPONSE_HPP

#include <Geode/utils/web.hpp>
#include <Geode/Prelude.hpp>

#include <optional>
#include <vector>

using namespace geode::prelude;

struct Send {
    int64_t timestamp;

    Send& operator=(const Send& other) {
        if (this != &other) {
            timestamp = other.timestamp;
        }
        return *this;
    }
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
    std::optional<Rate> rate;
};

struct BatchResponse {
    std::vector<Level> levels;
};

template <>
struct matjson::Serialize<Send> {
    static Result<Send> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const int64_t timestamp, value["timestamp"].asInt());
        return Ok(Send { timestamp });
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

        std::optional<Rate> rate;
        if (value.contains("rate") && !value["rate"].isNull()) {
            GEODE_UNWRAP_INTO(const Rate r, value["rate"].as<Rate>());
            rate = r;
        }

        return Ok(Level { levelID, sends, accurate, platformer, length, rate });
    }
};

template <>
struct matjson::Serialize<BatchResponse> {
    static Result<BatchResponse> fromJson(const Value& value) {
        GEODE_UNWRAP_INTO(const std::vector<Value> levels_json, value["levels"].asArray());
        std::vector<Level> levels;
        for (const Value& level_val : levels_json) {
            GEODE_UNWRAP_INTO(const Level level, level_val.as<Level>());
            levels.push_back(level);
        }
        return Ok(BatchResponse { levels });
    }
};

#endif