#include "LevelSendChartPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

#include <UIBuilder.hpp>
#include <node/RankingNode.hpp>

#include <node/SendChartNode.hpp>
#include <utils/PointUtils.hpp>

constexpr CCPoint popupSize = {400.0f, 250.0f};
constexpr CCPoint menuSize = {390.0f, 240.0f};

bool LevelSendChartPopup::init(const GJGameLevel* level, const int _levelID, const std::optional<Level>& _levelData, const std::optional<Creator>& _creatorData) {
    if (!FLAlertLayer::init(75)) return false;

    levelID = _levelID;
    levelData = _levelData;
    creatorData = _creatorData;
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

    std::vector creatorLevels = creatorData.has_value() ? creatorData->levels : std::vector<CreatorLevel>{};
    std::ranges::sort(creatorLevels, [](const CreatorLevel& a, const CreatorLevel& b) {
        return a.send_count > b.send_count;
    });
    std::vector<int> levelIDs;
    std::ranges::transform(creatorLevels, std::back_inserter(levelIDs), [](const CreatorLevel& c) {
        return c.levelID;
    });

    if (levelData.has_value()) {
        auto menu = Build<CCMenu>::create()
                .anchorPoint({0.0f, 1.0f})
                // .contentSize({260.0f, 300.0f})
                .pos({282.0f, 170.0f})
                .layout(ColumnLayout::create()
                    ->setAxisAlignment(AxisAlignment::End)
                    ->setCrossAxisLineAlignment(AxisAlignment::Start)
                    ->setAxisReverse(true)
                    ->setAutoGrowAxis(true)
                    ->setAutoScale(false)
                    ->setGap(23.0f)
                )
                .parent(m_buttonMenu);

        Build<CCLabelBMFont>::create("Rankings", "bigFont.fnt")
                .anchorPoint({0.5f, 0.5f})
                .posX(menu->getScaledContentWidth() / 2.0f)
                .scale(0.7f)
                .parent(menu);

        const Level levelInfo = levelData.value();
        SeedValueRSV stars = level->m_stars;

        Build<RankingNode>::create(levelInfo.rank, std::nullopt)
                .scale(0.6f)
                .anchorPoint({0.0f, 0.5f})
                .parent(menu);

        if (const auto it = std::ranges::find(levelIDs, levelID); it != levelIDs.end()) {
            const size_t index = it - levelIDs.begin();
            const int rank = static_cast<int>(index) + 1;
            Build<RankingNode>::create(rank, static_cast<int>(levelIDs.size()), RankingFilter::User)
                    .scale(0.6f)
                    .anchorPoint({0.0f, 0.5f})
                    .parent(menu);
        }

        const RankingFilter rateFilter = stars.value() > 0 ? RankingFilter::Rated : RankingFilter::Unrated;
        const RankingFilter gamemodeFilter = level->m_levelLength > 4 ? RankingFilter::Platformer : RankingFilter::Classic;

        Build<RankingNode>::create(levelInfo.rate_rank, std::nullopt, rateFilter)
                .scale(0.6f)
                .anchorPoint({0.0f, 0.5f})
                .parent(menu);

        Build<RankingNode>::create(levelInfo.gamemode_rank, std::nullopt, gamemodeFilter)
                .scale(0.6f)
                .anchorPoint({0.0f, 0.5f})
                .parent(menu);

        Build<RankingNode >::create(levelInfo.joined_rank, std::nullopt, rateFilter, gamemodeFilter)
                .scale(0.6f)
                .anchorPoint({0.0f, 0.5f})
                .parent(menu);

        menu->updateLayout();
    }

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

LevelSendChartPopup* LevelSendChartPopup::create(const GJGameLevel* level, const int levelID, const std::optional<Level>& levelData, const std::optional<Creator>& creatorData) {
    if (const auto newLayer = new LevelSendChartPopup(); newLayer->init(level, levelID, levelData, creatorData)) {
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