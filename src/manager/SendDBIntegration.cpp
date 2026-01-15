#include "SendDBIntegration.hpp"

#include <matjson/std.hpp>

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
    if (const std::optional<Level> cachedLevel = cache.getLevel(levelID)) {
        callback(cachedLevel);
        return;
    }

    sendGetRequest(SERVER_URL "/level/" + std::to_string(levelID), [this, callback, levelID](const matjson::Value& data) {
        if (data.contains("error")) {
            log::error("Failed to get level {}: {}", levelID, data["error"].asString().unwrapOrDefault());
            callback(std::nullopt);
        } else {
            if (const Result<Level> result = data.as<Level>(); result.isOk()) {
                const Level level = result.unwrap();
                cache.cacheLevel(level);
                callback(level);
            } else {
                log::error("Failed to parse level {}: {}", levelID, result.unwrapErr());
                callback(std::nullopt);
            }
        }
    }, listener);
}

void SendDBIntegration::getLevels(const std::vector<int>& levelIDs, const std::function<void(std::vector<Level>)>& callback, EventListener<web::WebTask>& listener) {
    std::vector<Level> cachedLevels;
    std::vector<int> uncachedIDs;

    for (const int levelID : levelIDs) {
        if (auto cachedLevel = cache.getLevel(levelID)) {
            cachedLevels.push_back(*cachedLevel);
        } else {
            uncachedIDs.push_back(levelID);
        }
    }

    if (uncachedIDs.empty()) {
        callback(cachedLevels);
        return;
    }

    matjson::Value body;
    body["level_ids"] = levelIDs;

    sendPostRequest(SERVER_URL "/level/batch", body, [this, callback, cachedLevels](const matjson::Value& data) {
        if (data.contains("error")) {
            log::error("Failed to get levels: {}", data["error"].asString().unwrapOrDefault());
            callback(cachedLevels);
        } else {
            if (const Result<BatchResponse> result = data.as<BatchResponse>(); result.isOk()) {
                std::vector<Level> fetchedLevels = result.unwrap().levels;
                for (const auto& level : fetchedLevels) {
                    cache.cacheLevel(level);
                }
                fetchedLevels.insert(fetchedLevels.end(), cachedLevels.begin(), cachedLevels.end());
                callback(fetchedLevels);
            } else {
                log::error("Failed to parse levels: {}", result.unwrapErr());
                callback(cachedLevels);
            }
        }
    }, listener);
}