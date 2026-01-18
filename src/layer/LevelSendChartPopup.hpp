#ifndef SENDDB_LEVELSENDCHARTPOPUP_HPP
#define SENDDB_LEVELSENDCHARTPOPUP_HPP

#include <Geode/binding/FLAlertLayer.hpp>
#include <model/APIResponse.hpp>
#include <node/SendChartNode.hpp>

using namespace geode::prelude;

class LevelSendChartPopup final : public FLAlertLayer {
    int levelID = 0;
    std::string levelName, creator;
    std::optional<Level> levelData;

    SendChartNode* chartNode = nullptr;

    bool init(const GJGameLevel* level, int _levelID, const std::optional<Level>& _levelData);

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;

    void onClose(CCObject* sender);

public:
    static LevelSendChartPopup* create(const GJGameLevel* level, int levelID, const std::optional<Level>& levelData);
    void show() override;
};

#endif