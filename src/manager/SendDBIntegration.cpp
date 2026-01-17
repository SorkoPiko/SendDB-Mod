#include "SendDBIntegration.hpp"

SendDBIntegration* SendDBIntegration::instance = nullptr;

void SendDBIntegration::sendPostRequest(const std::string& url, const matjson::Value& body, const std::function<void(const matjson::Value&)>& callback, EventListener<web::WebTask>& listener) {
    listener.bind([callback](web::WebTask::Event* e) {
        if (const auto res = e->getValue()) {
            if (!res->ok()) {
                matjson::Value ret;
                matjson::Value defaultVal;
                defaultVal["message"] = "Malformed response";
                if (matjson::Value unwrapped = res->json().unwrapOr(defaultVal); unwrapped.contains("message")) {
                    ret["error"] = unwrapped["message"];
                } else {
                    ret["error"] = defaultVal["message"];
                }
                callback(ret);
                return;
            }
            const auto data = res->json().unwrapOr(matjson::Value{});
            callback(data);
        } else if (e->isCancelled()) {
            matjson::Value ret;
            ret["error"] = "Request was cancelled.";
            callback(ret);
        }
    });

    currentRequest = std::make_unique<web::WebRequest>();
    currentRequest->bodyJSON(body);
    listener.setFilter(currentRequest->post(url));
}

void SendDBIntegration::sendGetRequest(const std::string& url, const std::function<void(const matjson::Value&)>& callback, EventListener<web::WebTask>& listener) {
    listener.bind([callback](web::WebTask::Event* e) {
        if (const auto res = e->getValue()) {
            if (!res->ok()) {
                matjson::Value ret;
                matjson::Value defaultVal;
                defaultVal["message"] = "Malformed response";
                if (matjson::Value unwrapped = res->json().unwrapOr(defaultVal); unwrapped.contains("message")) {
                    ret["error"] = unwrapped["message"];
                } else {
                    ret["error"] = defaultVal["message"];
                }
                callback(ret);
                return;
            }
            const auto data = res->json().unwrapOr(matjson::Value{});
            callback(data);
        } else if (e->isCancelled()) {
            matjson::Value ret;
            ret["error"] = "Request was cancelled.";
            callback(ret);
        }
    });

    currentRequest = std::make_unique<web::WebRequest>();
    listener.setFilter(currentRequest->get(url));
}

void SendDBIntegration::getLevel(const int levelID, const std::function<void(std::optional<Level>)>& callback, EventListener<web::WebTask>& listener) {
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
                const Level level = result.unwrap();
                cache.cacheLevel(level.levelID, level);
                callback(level);
            } else {
                log::error("Failed to parse level {}: {}", levelID, result.unwrapErr());
                callback(std::nullopt);
            }
        }
    }, listener);
}

std::vector<EventListener<web::WebTask>> SendDBIntegration::getLevels(const std::vector<int>& levelIDs, const std::function<void(std::vector<BatchLevel>)>& callback, std::shared_ptr<void> lifetimeTracker) {
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

    std::vector<EventListener<web::WebTask>> chunkListeners;
    chunkListeners.reserve(totalChunks);

    auto processChunk = [this, allFetchedLevels, callback, completedChunks, totalChunks](const std::vector<int>& chunkIDs, EventListener<web::WebTask>& chunkListener) {
        matjson::Value body;
        body["level_ids"] = chunkIDs;

        sendPostRequest(SERVER_URL "/level/batch", body, [this, allFetchedLevels, callback, completedChunks, totalChunks, chunkIDs](const matjson::Value& data) {
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