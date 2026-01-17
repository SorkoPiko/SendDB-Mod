#include "LevelSendChartPopup.hpp"

#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
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

    const auto bg = CCScale9Sprite::create("GJ_square05.png", {0.0f, 0.0f, 80.0f, 80.0f});
    bg->setContentSize(popupSize);
    bg->setPosition({winSize.width / 2, winSize.height / 2});
    bg->setID("bg"_spr);
    m_mainLayer->addChild(bg, -1);

    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setID("menu"_spr);
    m_buttonMenu->setContentSize(menuSize);
    m_buttonMenu->setPosition({winSize.width / 2 - menuSize.x / 2, winSize.height / 2 - menuSize.y / 2});
    m_mainLayer->addChild(m_buttonMenu, 10);

    const auto closeButtonSprite = CircleButtonSprite::createWithSpriteFrameName("geode.loader/close.png", 0.85f, CircleBaseColor::Gray);
    const auto closeBtn = CCMenuItemSpriteExtra::create(
        closeButtonSprite,
        this,
        menu_selector(LevelSendChartPopup::onClose)
    );
    closeBtn->setPosition({menuSize.x + 5.0f, menuSize.y + 5.0f});
    closeBtn->setScale(0.75f);
    closeBtn->setID("close-button"_spr);
    m_buttonMenu->addChild(closeBtn);

    chartNode = SendChartNode::create(
        levelData,
        CCSize(260, 150),
        1.0f,
        chartStyleFromString(Mod::get()->getSettingValue<std::string>("graphStyle"))
    );
    chartNode->setAnchorPoint({0.0f, 0.0f});
    chartNode->setPosition({20.0f, 20.0f});
    chartNode->setID("send-chart"_spr);
    m_buttonMenu->addChild(chartNode);

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