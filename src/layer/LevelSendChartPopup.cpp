#include "LevelSendChartPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

#include <UIBuilder.hpp>

#include <node/SendChartNode.hpp>
#include <utils/PointUtils.hpp>

constexpr CCPoint popupSize = {400.0f, 250.0f};
constexpr CCPoint menuSize = {390.0f, 240.0f};

bool LevelSendChartPopup::init(const GJGameLevel* level, const int _levelID, const std::optional<Level>& _levelData) {
    if (!FLAlertLayer::init(75)) return false;

    levelID = _levelID;
    if (_levelData.has_value()) levelData = _levelData.value();
    levelName = level->m_levelName;
    creator = level->m_creatorName.empty() ? "Unknown" : level->m_creatorName;

    const CCSize winSize = CCDirector::sharedDirector()->getWinSize();

    Build<CCScale9Sprite>::create("GJ_square05.png")
            .contentSize(popupSize)
            .pos({winSize.width / 2, winSize.height / 2})
            .id("bg"_spr)
            .parent(m_mainLayer)
            .zOrder(-1);

    m_buttonMenu = Build<CCMenu>::create()
            .contentSize(menuSize)
            .pos({winSize.width / 2 - menuSize.x / 2, winSize.height / 2 - menuSize.y / 2})
            .id("menu"_spr)
            .zOrder(10)
            .parent(m_mainLayer);

    Build(CircleButtonSprite::createWithSpriteFrameName("geode.loader/close.png", 0.85f, CircleBaseColor::Gray))
            .intoMenuItem(this, menu_selector(LevelSendChartPopup::onClose))
            .pos({menuSize.x + 5.0f, menuSize.y + 5.0f})
            .scale(0.75f)
            .id("close-button"_spr)
            .parent(m_buttonMenu);

    chartNode = Build(SendChartNode::create(
        levelData,
        CCSize(260, 150),
        1.0f,
        chartStyleFromString(Mod::get()->getSettingValue<std::string>("graphStyle"))
    ))
            .anchorPoint({0.0f, 0.0f})
            .pos({20.0f, 20.0f})
            .id("send-chart"_spr)
            .parent(m_buttonMenu);

    return true;
}

bool LevelSendChartPopup::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!FLAlertLayer::ccTouchBegan(touch, event)) return false;

    const CCPoint touchLocation = touch->getLocation();
    if (!PointUtils::isPointInsideNode(m_mainLayer->getChildByID("bg"_spr), touchLocation)) {
        onClose(nullptr);
        return true;
    }

    if (PointUtils::isPointInsideNode(chartNode, touchLocation)) {
        chartNode->onClick(touchLocation);
    }

    return true;
}

void LevelSendChartPopup::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    FLAlertLayer::ccTouchEnded(touch, event);

    chartNode->onRelease(touch->getLocation());
}

LevelSendChartPopup* LevelSendChartPopup::create(const GJGameLevel* level, const int levelID, const std::optional<Level>& levelData) {
    if (const auto newLayer = new LevelSendChartPopup(); newLayer->init(level, levelID, levelData)) {
        newLayer->autorelease();
        return newLayer;
    } else {
        delete newLayer;
        return nullptr;
    }
}

void LevelSendChartPopup::show() {
    FLAlertLayer::show();
    handleTouchPriority(this);
}

void LevelSendChartPopup::onClose(CCObject* sender) {
    setKeypadEnabled(false);
    removeFromParentAndCleanup(true);
}