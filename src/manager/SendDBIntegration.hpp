#ifndef SENDDB_SENDDBINTEGRATION_HPP
#define SENDDB_SENDDBINTEGRATION_HPP

//#define SERVER_URL "https://api.senddb.dev/api/v1"
#define SERVER_URL "http://127.0.0.1:8080/api/v1"
// TODO: change back to production server url before release

#include <Geode/Prelude.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/web.hpp>

#include <model/APIResponse.hpp>
#include "APICache.hpp"

using namespace geode::prelude;

class SendDBIntegration {
    static SendDBIntegration* instance;

    EventListener<web::WebTask> downloadListener;
    std::unique_ptr<web::WebRequest> currentRequest;
    APICache cache{Mod::get()->getSettingValue<int>("cacheTime") * 60};

    void sendPostRequest(const std::string& url, const matjson::Value& body, const std::function<void(const matjson::Value&)>& callback, EventListener<web::WebTask>& listener);
    void sendGetRequest(const std::string& url, const std::function<void(const matjson::Value&)>& callback, EventListener<web::WebTask>& listener);

public:
    static SendDBIntegration* get() {
        if (!instance) instance = new SendDBIntegration();
        return instance;
    }

    void setCacheDuration(const int duration) {
        cache.setCacheDuration(duration);
    }

    void getLevel(int levelID, const std::function<void(std::optional<Level>)>& callback, EventListener<web::WebTask>& listener);
    std::vector<EventListener<web::WebTask>> getLevels(const std::vector<int>& levelIDs, const std::function<void(std::vector<BatchLevel>)>& callback, std::shared_ptr<void> lifetimeTracker);
};

#endif