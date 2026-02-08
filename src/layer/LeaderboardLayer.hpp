#pragma once

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
    CCLabelBMFont* title = nullptr;
    CCLabelBMFont* pageLabel = nullptr;
    CCMenuItemSpriteExtra* refreshButton = nullptr;
    CCMenuItemSpriteExtra* filterButton = nullptr;
    CCMenuItemSpriteExtra* prevPageButton = nullptr;
    CCMenuItemSpriteExtra* nextPageButton = nullptr;
    CCMenuItemSpriteExtra* trendingButton = nullptr;
    CCLabelBMFont* pageText = nullptr;
    CCMenuItemSpriteExtra* pageButton = nullptr;
    FadeSpinner* loadingCircle = nullptr;
    std::vector<Ref<CCSprite>> shaderSprites;

    ShaderNode* trendingBackground = nullptr;

    EventListener<web::WebTask> leaderboardListener;
    std::vector<EventListener<web::WebTask>> sendCountListeners;
    int queryTotal = 0;
    std::vector<int> pageIDs;
    std::vector<int> currentQuery;
    static bool trending;
    bool loading = false;
    bool circleShown = false;

    std::unordered_map<int, int> ranks;
    std::unordered_map<int, double> trendingScores;
    static std::unordered_map<int, Ref<GJGameLevel>> cache;
    static std::set<int> failedCache;
    std::unordered_map<int, BatchLevel> batchCache;

    bool init() override;
    ~LeaderboardLayer() override;

    void updateShaderSprites() const;

    void onRefresh();

    void getSendCounts(const std::vector<int>& levelIDs);
    void updateSendCounts();

    void onLoaded(const std::vector<int>& levels, int total);
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
    void loadLevelsFailed(const char* key) override;
    void loadLevelsFailed(const char*, int) override;

    void setIDPopupClosed(SetIDPopup* popup, int value) override;

    void toggleLoadingUi(bool loadingState);

public:
    static LeaderboardQuery query;

    static LeaderboardLayer* create();
};