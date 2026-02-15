#pragma once

#ifdef NDEBUG
#define SERVER_URL "https://api.senddb.dev/api/v1"
#else
#define SERVER_URL "http://127.0.0.1:8080/api/v1"
#endif

#include <Geode/Prelude.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/web.hpp>

#include <model/APIRequest.hpp>
#include <model/APIResponse.hpp>
#include "APICache.hpp"

using namespace geode::prelude;

class SendDBIntegration {
    static SendDBIntegration* instance;

    TaskHolder<web::WebResponse> downloadListener;
    std::unique_ptr<web::WebRequest> currentRequest;
    APICache cache{Mod::get()->getSettingValue<int>("cacheTime") * 60};

    void sendPostRequest(const std::string& url, const matjson::Value& body, const std::function<void(const matjson::Value&)>& callback, TaskHolder<web::WebResponse>& listener);
    void sendGetRequest(const std::string& url, const std::function<void(const matjson::Value&)>& callback, TaskHolder<web::WebResponse>& listener);

public:
    static SendDBIntegration* get() {
        if (!instance) instance = new SendDBIntegration();
        return instance;
    }

    APICache& getCache() {
        return cache;
    }

    void getLevel(int levelID, const std::function<void(std::optional<Level>)>& callback, TaskHolder<web::WebResponse>& listener);
    std::vector<TaskHolder<web::WebResponse>> getLevels(const std::vector<int>& levelIDs, const std::function<void(std::vector<BatchLevel>)>& callback);
    void getCreator(int creatorID, const std::function<void(std::optional<Creator>)>& callback, TaskHolder<web::WebResponse>& listener);

    void getLeaderboard(const LeaderboardQuery& query, const std::function<void(std::optional<LeaderboardResponse>)>& callback, TaskHolder<web::WebResponse>& listener);
    void getTrendingLeaderboard(const TrendingLeaderboardQuery& query, const std::function<void(std::optional<TrendingLeaderboardResponse>)>& callback, TaskHolder<web::WebResponse>& listener);
    void getCreatorLeaderboard(const CreatorLeaderboardQuery& query, const std::function<void(std::optional<CreatorLeaderboardResponse>)>& callback, TaskHolder<web::WebResponse>& listener);
};