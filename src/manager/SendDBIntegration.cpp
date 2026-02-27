#include "SendDBIntegration.hpp"

SendDBIntegration* SendDBIntegration::instance = nullptr;

void SendDBIntegration::sendPostRequest(const std::string& url, const matjson::Value& body, const std::function<void(const matjson::Value&)>& callback, TaskHolder<web::WebResponse>& listener) {
    listener.spawn(
        web::WebRequest().header("User-Agent", "SendDB/1.0").bodyJSON(body).post(url),
        [callback](const web::WebResponse& e) {
            if (e.ok()) {
                const auto data = e.json().unwrapOr(matjson::Value{});
                callback(data);
            } else {
                matjson::Value ret;
                matjson::Value defaultVal;
                defaultVal["message"] = "Malformed response";
                if (matjson::Value unwrapped = e.json().unwrapOr(defaultVal); unwrapped.contains("message")) {
                    ret["error"] = unwrapped["message"];
                } else {
                    ret["error"] = defaultVal["message"];
                }
                callback(ret);
            }
        }
    );
}

void SendDBIntegration::sendGetRequest(const std::string& url, const std::function<void(const matjson::Value&)>& callback, TaskHolder<web::WebResponse>& listener) {
    listener.spawn(
        web::WebRequest().header("User-Agent", "SendDB/1.0").get(url),
        [callback](const web::WebResponse& e) {
            if (e.ok()) {
                const auto data = e.json().unwrapOr(matjson::Value{});
                callback(data);
            } else {
                matjson::Value ret;
                matjson::Value defaultVal;
                defaultVal["message"] = fmt::format("Malformed response: {}", e.string().unwrapOrDefault());
                if (matjson::Value unwrapped = e.json().unwrapOr(defaultVal); unwrapped.contains("message")) {
                    ret["error"] = unwrapped["message"];
                } else {
                    ret["error"] = defaultVal["message"];
                }
                callback(ret);
            }
        }
    );
}

void SendDBIntegration::getLevel(const int levelID, const std::function<void(std::optional<Level>)>& callback, TaskHolder<web::WebResponse>& listener) {
    if (const auto cachedLevel = cache.getLevel(levelID)) {
        callback(cachedLevel.value());
        return;
    }

    sendGetRequest(SERVER_URL "/level/" + std::to_string(levelID), [this, callback, levelID](const matjson::Value& data) {
        if (data.contains("error")) {
            const std::string error = data["error"].asString().unwrapOrDefault();
            if (error == "Level not found") {
                cache.cacheLevel(levelID, std::nullopt);
            } else {
                log::error("Failed to get level {}: {}", levelID, error);
            }
            callback(std::nullopt);
        } else {
            if (const Result<Level> result = data.as<Level>(); result.isOk()) {
                const Level& level = result.unwrap();
                cache.cacheLevel(level.levelID, level);
                callback(level);
            } else {
                log::error("Failed to parse level {}: {}", levelID, result.unwrapErr());
                callback(std::nullopt);
            }
        }
    }, listener);
}

std::vector<TaskHolder<web::WebResponse>> SendDBIntegration::getLevels(const std::vector<int>& levelIDs, const std::function<void(std::vector<BatchLevel>)>& callback) {
    std::vector<BatchLevel> cachedLevels;
    std::vector<int> uncachedIDs;

    for (const int levelID : levelIDs) {
        if (auto cachedLevel = cache.getBatchLevel(levelID)) {
            if (cachedLevel->has_value()) {
                cachedLevels.push_back(cachedLevel->value());
            }
        } else {
            uncachedIDs.push_back(levelID);
        }
    }

    if (uncachedIDs.empty()) {
        callback(cachedLevels);
        return {};
    }

    size_t totalChunks = (uncachedIDs.size() + 49) / 50;

    auto allFetchedLevels = std::make_shared<std::vector<BatchLevel>>(cachedLevels);
    auto completedChunks = std::make_shared<std::atomic<size_t>>(0);

    std::vector<TaskHolder<web::WebResponse>> chunkListeners;
    chunkListeners.reserve(totalChunks);

    auto processChunk = [this, allFetchedLevels, callback, completedChunks, totalChunks](const std::vector<int>& chunkIDs, TaskHolder<web::WebResponse>& chunkListener) {
        BatchRequest request = {
            chunkIDs
        };

        sendPostRequest(SERVER_URL "/level/batch", request, [this, allFetchedLevels, callback, completedChunks, totalChunks, chunkIDs](const matjson::Value& data) {
            if (data.contains("error")) {
                log::error("Failed to get levels chunk: {}", data["error"].asString().unwrapOrDefault());
            } else {
                if (const Result<BatchResponse> result = data.as<BatchResponse>(); result.isOk()) {
                    std::vector<BatchLevel> fetchedLevels = result.unwrap().levels;
                    for (const auto& level : fetchedLevels) {
                        cache.cacheBatchLevel(level.levelID, level);
                    }
                    for (const auto& levelID : chunkIDs) {
                        if (std::ranges::none_of(fetchedLevels, [levelID](const BatchLevel& lvl) { return lvl.levelID == levelID; })) {
                            cache.cacheBatchLevel(levelID, std::nullopt);
                        }
                    }
                    allFetchedLevels->insert(allFetchedLevels->end(), fetchedLevels.begin(), fetchedLevels.end());
                } else {
                    log::error("Failed to parse levels chunk: {}", result.unwrapErr());
                }
            }
            ++*completedChunks;
            if (*completedChunks == totalChunks) {
                callback(*allFetchedLevels);
            }
        }, chunkListener);
    };

    for (size_t i = 0; i < uncachedIDs.size(); i += 50) {
        const size_t end = std::min(i + 50, uncachedIDs.size());
        std::vector chunk(uncachedIDs.begin() + i, uncachedIDs.begin() + end);
        chunkListeners.emplace_back();
        processChunk(chunk, chunkListeners.back());
    }

    return chunkListeners;
}

void SendDBIntegration::getCreator(const int creatorID, const std::function<void(std::optional<Creator>)>& callback, TaskHolder<web::WebResponse>& listener) {
    if (const auto cachedCreator = cache.getCreator(creatorID)) {
        callback(cachedCreator.value());
        return;
    }

    sendGetRequest(SERVER_URL "/creator/" + std::to_string(creatorID), [this, callback, creatorID](const matjson::Value& data) {
        if (data.contains("error")) {
            const std::string error = data["error"].asString().unwrapOrDefault();
            if (error == "Creator not found") {
                cache.cacheLevel(creatorID, std::nullopt);
            } else {
                log::error("Failed to get creator {}: {}", creatorID, error);
            }
            callback(std::nullopt);
        } else {
            if (const Result<Creator> result = data.as<Creator>(); result.isOk()) {
                const Creator& creator = result.unwrap();
                cache.cacheCreator(creator.playerID, creator);
                callback(creator);
            } else {
                log::error("Failed to parse creator {}: {}", creatorID, result.unwrapErr());
                callback(std::nullopt);
            }
        }
    }, listener);
}

void SendDBIntegration::getLeaderboard(const LeaderboardQuery& query, const std::function<void(std::optional<LeaderboardResponse>)>& callback, TaskHolder<web::WebResponse>& listener) {
    if (const auto cachedResponse = cache.getLeaderboard(query)) {
        callback(cachedResponse);
        return;
    }

    sendPostRequest(SERVER_URL "/leaderboard", query, [this, query, callback](const matjson::Value& data) {
        if (data.contains("error")) {
            log::error("Failed to get leaderboard: {}", data["error"].asString().unwrapOrDefault());
            callback(std::nullopt);
        } else {
            if (const Result<LeaderboardResponse> result = data.as<LeaderboardResponse>(); result.isOk()) {
                const LeaderboardResponse& response = result.unwrap();
                cache.cacheLeaderboard(query, response);
                callback(response);
            } else {
                log::error("Failed to parse leaderboard response: {}", result.unwrapErr());
                callback(std::nullopt);
            }
        }
    }, listener);
}

void SendDBIntegration::getTrendingLeaderboard(const TrendingLeaderboardQuery& query, const std::function<void(std::optional<TrendingLeaderboardResponse>)>& callback, TaskHolder<web::WebResponse>& listener) {
    if (const auto cachedResponse = cache.getTrendingLeaderboard(query)) {
        callback(cachedResponse);
        return;
    }

    sendPostRequest(SERVER_URL "/leaderboard/trending", query, [this, query, callback](const matjson::Value& data) {
        if (data.contains("error")) {
            log::error("Failed to get trending leaderboard: {}", data["error"].asString().unwrapOrDefault());
            callback(std::nullopt);
        } else {
            if (const Result<TrendingLeaderboardResponse> result = data.as<TrendingLeaderboardResponse>(); result.isOk()) {
                const TrendingLeaderboardResponse& response = result.unwrap();
                cache.cacheTrendingLeaderboard(query, response);
                callback(response);
            } else {
                log::error("Failed to parse trending leaderboard response: {}", result.unwrapErr());
                callback(std::nullopt);
            }
        }
    }, listener);
}

void SendDBIntegration::getCreatorLeaderboard(const CreatorLeaderboardQuery& query, const std::function<void(std::optional<CreatorLeaderboardResponse>)>& callback, TaskHolder<web::WebResponse>& listener) {
    if (const auto cachedResponse = cache.getCreatorLeaderboard(query)) {
        callback(cachedResponse);
        return;
    }

    sendPostRequest(SERVER_URL "/leaderboard/creators", query, [this, query, callback](const matjson::Value& data) {
        if (data.contains("error")) {
            log::error("Failed to get creator leaderboard: {}", data["error"].asString().unwrapOrDefault());
            callback(std::nullopt);
        } else {
            if (const Result<CreatorLeaderboardResponse> result = data.as<CreatorLeaderboardResponse>(); result.isOk()) {
                const CreatorLeaderboardResponse& response = result.unwrap();
                cache.cacheCreatorLeaderboard(query, response);
                callback(response);
            } else {
                log::error("Failed to parse creator leaderboard response: {}", result.unwrapErr());
                callback(std::nullopt);
            }
        }
    }, listener);
}