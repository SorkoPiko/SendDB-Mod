#ifndef SENDDB_LEADERBOARDBROWSERLAYER_HPP
#define SENDDB_LEADERBOARDBROWSERLAYER_HPP

#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <manager/SendDBIntegration.hpp>

#include <Geode/utils/web.hpp>

struct $modify(LeaderboardBrowserLayer, LevelBrowserLayer) {
    struct Fields {
        bool enabled = false;

        bool spoofLoad = false;
        bool ignore = false;
        EventListener<web::WebTask> leaderboardListener;

        LeaderboardQuery currentFilter;
        LeaderboardResponse currentResponse;

        std::vector<EventListener<web::WebTask>> listeners;
        std::unordered_map<int, BatchLevel> levels;
    };

    bool init(GJSearchObject* object);

    void setFilter(const LeaderboardQuery& filter);

    void loadPage(GJSearchObject* object);
    void setupLevelBrowser(CCArray* levels);
    void setupPageInfo(std::string info, const char* key);
    void updatePageLabel();
    bool isCorrect(const char* key);

    void customLoad();

    void refreshSendLabels();

    static LeaderboardBrowserLayer* create(const LeaderboardQuery& filter);
};

#endif