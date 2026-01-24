#ifndef SENDDB_LEADERBOARDLAYER_HPP
#define SENDDB_LEADERBOARDLAYER_HPP

#include <cue/ListNode.hpp>
#include <Geode/Prelude.hpp>

#include <Geode/binding/LevelManagerDelegate.hpp>
#include <model/APIRequest.hpp>
#include <model/APIResponse.hpp>
#include <node/ShaderNode.hpp>

#include "BaseLayer.hpp"
#include "FadeSpinner.hpp"

using namespace geode::prelude;

class LeaderboardLayer : public BaseLayer, LevelManagerDelegate, SetIDPopupDelegate {
    cue::ListNode* list = nullptr;
    ShaderNode* listBackground = nullptr;
    CCLabelBMFont* pageLabel = nullptr;
    CCMenuItemSpriteExtra* refreshButton = nullptr;
    CCMenuItemSpriteExtra* filterButton = nullptr;
    CCMenuItemSpriteExtra* prevPageButton = nullptr;
    CCMenuItemSpriteExtra* nextPageButton = nullptr;
    CCLabelBMFont* pageText = nullptr;
    CCMenuItemSpriteExtra* pageButton = nullptr;
    FadeSpinner* loadingCircle = nullptr;

    EventListener<web::WebTask> leaderboardListener;
    std::vector<EventListener<web::WebTask>> sendCountListeners;
    LeaderboardQuery query = {Mod::get()->getSettingValue<int>("leaderboardPerPage"), 0, std::nullopt, std::nullopt};
    int queryTotal = 0;
    std::vector<LeaderboardLevel> pageLevels = {};
    std::vector<int> currentQuery;
    bool loading = false;
    bool circleShown = false;

    std::unordered_map<int, Ref<GJGameLevel>> cache;
    std::unordered_map<int, BatchLevel> batchCache;
    std::set<int> failedCache;

    bool init() override;
    ~LeaderboardLayer() override;

    void onRefresh();

    void getSendCounts(const std::vector<int>& levelIDs);
    void updateSendCounts();

    void onLoaded(const std::vector<LeaderboardLevel>& levels, int total);
    void setReady(float);

    void startLoadingForPage();
    void continueLoading();
    void finishLoading();
    bool loadNextBatch();

    void keyDown(enumKeyCodes key) override;

    void onNextPage();
    void onPrevPage();

    void loadLevelsFinished(CCArray* levels, const char* key) override;
    void loadLevelsFinished(CCArray* levels, const char*, int) override;

    void setIDPopupClosed(SetIDPopup* popup, int value) override;

    void toggleLoadingUi(bool loadingState);

public:
    static LeaderboardLayer* create();

    void setQuery(LeaderboardQuery query);
};

#endif