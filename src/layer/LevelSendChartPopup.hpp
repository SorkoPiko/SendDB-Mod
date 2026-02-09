#pragma once

#include <Geode/binding/FLAlertLayer.hpp>
#include <model/APIResponse.hpp>
#include <node/SendChartNode.hpp>
#include <utils/Messages.hpp>
#include <utils/SendUtils.hpp>

using namespace geode::prelude;

class LevelSendChartPopup final : public FLAlertLayer {
    int levelID = 0;
    std::optional<Level> levelData;
    std::optional<Creator> creatorData;

    std::vector<int> sendTimestamps;
    PeakTrendingScore peakTrendingScore = {};

    CCScale9Sprite* bg = nullptr;
    SendChartNode* chartNode = nullptr;
    CCLabelBMFont* trendingScoreLabel = nullptr;
    CCLabelBMFont* peakTrendingScoreLabel = nullptr;

    bool init(const GJGameLevel* level, int _levelID, const std::optional<Level>& _levelData, const std::optional<Creator>& _creatorData);

    void update(float delta) override;

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;

    void onClose(CCObject*);

    static void infoPopup(const LevelSendPopupInfo& info);

public:
    static LevelSendChartPopup* create(const GJGameLevel* level, int levelID, const std::optional<Level>& levelData, const std::optional<Creator>& creatorData);
    void show() override;
};