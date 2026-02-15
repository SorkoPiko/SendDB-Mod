#pragma once

#include <cue/ListNode.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <model/APIResponse.hpp>
#include <node/SendChartNode.hpp>
#include <node/SentLevelNode.hpp>
#include <utils/Messages.hpp>

using namespace geode::prelude;

class CreatorInfoPopup : public FLAlertLayer, public LevelManagerDelegate {
    int playerID = 0;
    std::optional<Creator> creatorData;
    std::function<void()> closeCallback;
    GJCommentListLayer* commentList;

    CCScale9Sprite* bg = nullptr;
    cue::ListNode* levelList = nullptr;

    std::set<int> levelIDsToFetch;
    std::set<int> pendingLevelIDs;
    std::unordered_map<int, SentLevelNode*> levelNodes;
    static std::unordered_map<int, GJGameLevel*> cache;
    static std::set<int> failedLevelIDs;

    bool init(const GJUserScore* creator, const std::optional<Creator>& _creatorData, const GJCommentListLayer* commentList);
    ~CreatorInfoPopup() override;

    void updateLevelNodes();

    void downloadLevelInfo();

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;

    void loadLevelsFinished(CCArray* levels, const char* key) override;
    void loadLevelsFinished(CCArray* levels, const char*, int) override;
    void loadLevelsFailed(const char* key) override;
    void loadLevelsFailed(const char*, int) override;

    void keyDown(enumKeyCodes key, double timestamp) override;
    void onClose(CCObject*);

    void onEnter() override;

    void fixTouchPrio();

    static void infoPopup(const CreatorPopupInfo& info);

public:
    static CreatorInfoPopup* create(const GJUserScore* creator, const std::optional<Creator>& creatorData, const GJCommentListLayer* commentList);
    void show() override;

    void setCloseCallback(const std::function<void()>& callback) {
        closeCallback = callback;
    }
};