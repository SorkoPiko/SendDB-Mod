#include "LevelSendChartPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

#include <UIBuilder.hpp>
#include <node/RankingNode.hpp>

#include <node/SendChartNode.hpp>
#include <rock/RoundedRect.hpp>
#include <utils/PointUtils.hpp>
#include <utils/Style.hpp>
#include <utils/TimeUtils.hpp>

constexpr CCPoint popupSize = {400.0f, 250.0f};
constexpr CCPoint menuSize = {390.0f, 240.0f};

Build<CCSprite> changeIcon(const float value) {
    if (value > 0.0f) {
        return Build<CCSprite>::createSpriteName("edit_upBtn_001.png").scale(1.0f/0.84f);
    } else if (value < 0.0f) {
        return Build<CCSprite>::createSpriteName("edit_downBtn_001.png").scale(1.0f/0.84f);
    } else {
        return Build<CCSprite>::createSpriteName("edit_delCBtn_001.png").scale(1.0f/1.16f);
    }
}

ccColor3B changeColor(const float value, const ccColor3B less, const ccColor3B equal, const ccColor3B more) {
    if (value > 0.0f) {
        return more;
    } else if (value < 0.0f) {
        return less;
    } else {
        return equal;
    }
}

bool LevelSendChartPopup::init(const GJGameLevel* level, const int _levelID, const std::optional<Level>& _levelData, const std::optional<Creator>& _creatorData) {
    if (!FLAlertLayer::init(75)) return false;
    scheduleUpdate();

    levelID = _levelID;
    levelData = _levelData;
    creatorData = _creatorData;

    const CCSize winSize = CCDirector::sharedDirector()->getWinSize();

    bg = Build<CCScale9Sprite>::create("GJ_square05.png")
            .contentSize(popupSize)
            .pos({winSize.width / 2, winSize.height / 2})
            .id("bg")
            .parent(m_mainLayer)
            .zOrder(-1);

    m_buttonMenu = Build<CCMenu>::create()
            .contentSize(menuSize)
            .pos({winSize.width / 2 - menuSize.x / 2, winSize.height / 2 - menuSize.y / 2})
            .id("menu")
            .zOrder(10)
            .parent(m_mainLayer);

    Build(CircleButtonSprite::createWithSpriteFrameName("geode.loader/close.png", 0.85f, CircleBaseColor::Gray))
            .intoMenuItem(this, menu_selector(LevelSendChartPopup::onClose))
            .pos({menuSize.x + 5.0f, menuSize.y + 5.0f})
            .scale(0.75f)
            .id("close-button")
            .parent(m_buttonMenu);

    Build<CCLabelBMFont>::create(level->m_levelName.c_str(), "bigFont.fnt")
            .anchorPoint({0.0f, 1.0f})
            .pos({3.0f, menuSize.y + 2.0f})
            .scale(0.7f)
            .id("title-label")
            .parent(m_buttonMenu);

    Build<CCLabelBMFont>::create(fmt::format("by {}", level->m_creatorName.empty() ? "Unknown" : level->m_creatorName).c_str(), "chatFont.fnt")
            .anchorPoint({0.0f, 1.0f})
            .pos({3.0f, menuSize.y - 20.0f})
            .scale(0.5f)
            .id("creator-label")
            .parent(m_buttonMenu);

    chartNode = Build(SendChartNode::create(
        levelData,
        CCSize(260, 150),
        1.0f,
        chartStyleFromString(Mod::get()->getSettingValue<std::string>("graphStyle"))
    ))
            .anchorPoint({0.0f, 0.0f})
            .pos({20.0f, 20.0f})
            .id("send-chart")
            .parent(m_buttonMenu)
            .zOrder(1);

    std::vector creatorLevels = creatorData.has_value() ? creatorData->levels : std::vector<CreatorLevel>{};
    std::ranges::sort(creatorLevels, [](const CreatorLevel& a, const CreatorLevel& b) {
        return a.send_count > b.send_count;
    });
    auto levelIDs = geode::utils::ranges::map<std::vector<int>>(creatorLevels, [](const CreatorLevel& c) {
        return c.levelID;
    });

    if (levelData.has_value() && creatorData.has_value()) {
        const Level levelInfo = levelData.value();
        const Creator creatorInfo = creatorData.value();

        auto menu = Build<CCMenu>::create()
                .anchorPoint({0.0f, 1.0f})
                .pos({284.0f, 160.0f})
                .layout(ColumnLayout::create()
                    ->setAxisAlignment(AxisAlignment::End)
                    ->setCrossAxisLineAlignment(AxisAlignment::Start)
                    ->setAxisReverse(true)
                    ->setAutoGrowAxis(true)
                    ->setAutoScale(false)
                    ->setGap(2.0f)
                )
                .id("ranking-menu")
                .parent(m_buttonMenu);

        Build<CCLabelBMFont>::create("Rankings", "bigFont.fnt")
                .anchorPoint({0.0f, 0.5f})
                .pos({284.0f, 170.0f})
                .scale(0.6f)
                .parent(m_buttonMenu);

        Build<RankingNode>::create(levelInfo.rank, "All", std::nullopt, RankingFilter::SendDB)
                .scale(0.6f)
                .anchorPoint({0.5f, 0.5f})
                .parent(menu);

        if (const auto it = std::ranges::find(levelIDs, levelID); it != levelIDs.end()) {
            const size_t index = it - levelIDs.begin();
            const int rank = static_cast<int>(index) + 1;
            Build<RankingNode>::create(rank, "Creator", static_cast<int>(levelIDs.size()), RankingFilter::User)
                    .scale(0.6f)
                    .anchorPoint({0.5f, 0.5f})
                    .parent(menu);
        }

        std::string rateText;
        RankingFilter rateFilter;
        if (levelInfo.rate.has_value()) {
            rateText = "Rated";
            rateFilter = RankingFilter::Rated;
        } else {
            rateText = "Unrated";
            rateFilter = RankingFilter::Unrated;

            Build<RankingNode>::create(levelInfo.trending_rank, "Trending", std::nullopt, RankingFilter::Trending)
                    .scale(0.6f)
                    .anchorPoint({0.5f, 0.5f})
                    .parent(menu);
        }

        std::string gamemodeText;
        RankingFilter gamemodeFilter;
        if (levelInfo.platformer) {
            gamemodeText = "Platformers";
            gamemodeFilter = RankingFilter::Platformer;
        } else {
            gamemodeText = "Classics";
            gamemodeFilter = RankingFilter::Classic;
        }

        Build<RankingNode>::create(levelInfo.rate_rank, rateText, std::nullopt, rateFilter)
                .scale(0.6f)
                .anchorPoint({0.5f, 0.5f})
                .parent(menu);

        Build<RankingNode>::create(levelInfo.gamemode_rank, gamemodeText, std::nullopt, gamemodeFilter)
                .scale(0.6f)
                .anchorPoint({0.5f, 0.5f})
                .parent(menu);

        Build<RankingNode>::create(levelInfo.joined_rank, fmt::format("{} {}", rateText, gamemodeText), std::nullopt, rateFilter, gamemodeFilter)
                .scale(0.6f)
                .anchorPoint({0.5f, 0.5f})
                .parent(menu);

        menu->updateLayout();


        auto sendInfoNode = Build<CCNode>::create()
                .anchorPoint({0.0f, 1.0f})
                .pos({10.0f, menuSize.y - 35.0f})
                .contentSize({180.0f, 25.0f})
                .parent(m_buttonMenu);

        Build<CCSprite>::createSpriteName("GJ_starsIcon_001.png")
                .scale(0.9f/0.94f)
                .anchorPoint({0.5f, 0.5f})
                .pos({10.0f, 10.0f})
                .parent(sendInfoNode);

        auto sendCountLabel = Build<CCLabelBMFont>::create(fmt::format("Send Count: {}", levelInfo.sends.size()).c_str(), "bigFont.fnt")
                .limitLabelWidth(110.0f, 0.45f, 0.25f)
                .anchorPoint({0.0f, 0.5f})
                .pos({25.0f, sendInfoNode->getContentHeight() - 10.0f})
                .parent(sendInfoNode);

        const float creatorSendAverage = creatorInfo.send_count / static_cast<float>(creatorInfo.levels.size());
        const float sendDifference = levelInfo.sends.size() - creatorSendAverage;

        if (sendDifference != 0.0f) {
            auto differenceSprite = changeIcon(sendDifference)
                    .scaleBy(0.6f)
                    .anchorPoint({0.0f, 0.5f})
                    .parent(sendInfoNode)
                    .matchPos(sendCountLabel)
                    .move({sendCountLabel->getScaledContentWidth() + 3.0f, 0.0f})
                    .color(changeColor(sendDifference, differenceGreen, {255, 255, 255}, differenceRed));

            auto differenceLabel = Build<CCLabelBMFont>::create(fmt::format("{:+.2f}", sendDifference).c_str(), "chatFont.fnt")
                    .scale(0.6f)
                    .anchorPoint({0.0f, 0.5f})
                    .parent(sendInfoNode)
                    .matchPos(differenceSprite)
                    .move({differenceSprite->getScaledContentWidth() - 0.5f, 0.0f})
                    .color(secondaryTextColor);

            Build(rock::RoundedRect::create(
                infoBoxColor,
                3.0f,
                {
                    differenceLabel->getScaledContentWidth() + differenceLabel->getPositionX() - differenceSprite->getPositionX() + 2.0f,
                    differenceSprite->getScaledContentHeight() + 2.0f
                }
            ))
                    .anchorPoint({0.0f, 0.5f})
                    .parent(sendInfoNode)
                    .matchPos(differenceSprite)
                    .move({-1.0f, 0.0f})
                    .zOrder(-1);
        }

        Build<CCLabelBMFont>::create(fmt::format("Creator Average: {:.2f}", creatorSendAverage).c_str(), "bigFont.fnt")
                .scale(0.3f)
                .anchorPoint({0.0f, 0.5f})
                .parent(sendInfoNode)
                .matchPos(sendCountLabel)
                .move({0.0f, -10.0f})
                .color(secondaryTextColor);


        auto trendingInfoNode = Build<CCNode>::create()
                .anchorPoint({1.0f, 1.0f})
                .pos({menuSize.x - 10.0f, menuSize.y - 30.0f})
                .contentSize({190.0f, 30.0f})
                .parent(m_buttonMenu);

        Build<CCSprite>::createSpriteName("GJ_sTrendingIcon_001.png")
                .scale(0.9f/0.66f)
                .anchorPoint({0.5f, 0.5f})
                .pos({10.0f, 10.0f})
                .parent(trendingInfoNode);

        if (levelInfo.rate.has_value()) {
            Build<CCSprite>::createSpriteName("edit_delBtnSmall_001.png")
                    .scale(0.4f/0.52f)
                    .anchorPoint({0.5f, 0.5f})
                    .pos({17.0f, 3.0f})
                    .parent(trendingInfoNode);
        }

        trendingScoreLabel = Build<CCLabelBMFont>::create(fmt::format("Trending Score: {:.2f}", levelInfo.trending_score).c_str(), "bigFont.fnt")
                .limitLabelWidth(165.0f, 0.4f, 0.2f)
                .anchorPoint({0.0f, 0.5f})
                .pos({25.0f, trendingInfoNode->getContentHeight() - 10.0f})
                .parent(trendingInfoNode);

        sendTimestamps = geode::utils::ranges::map<std::vector<int>>(levelInfo.sends, [](const Send& s) {
            return s.timestamp / 1000;
        });

        peakTrendingScore = SendUtils::calculatePeakTrendingScore(sendTimestamps);

        peakTrendingScoreLabel = Build<CCLabelBMFont>::create(fmt::format(
            "Peak Score: {:.2f} ({})",
            peakTrendingScore.score,
            TimeUtils::timestampToDate(peakTrendingScore.timestamp * 1000LL)
        ).c_str(), "bigFont.fnt")
                .limitLabelWidth(165.0f, 0.3f, 0.15f)
                .anchorPoint({0.0f, 0.5f})
                .parent(trendingInfoNode)
                .matchPos(trendingScoreLabel)
                .move({0.0f, -9.0f})
                .color(secondaryTextColor);

        auto creatorScoreLabel = Build<CCLabelBMFont>::create(fmt::format("Creator Score: {:.2f}", creatorInfo.trending_score).c_str(), "bigFont.fnt")
                .limitLabelWidth(125.0f, 0.3f, 0.15f)
                .anchorPoint({0.0f, 0.5f})
                .parent(trendingInfoNode)
                .matchPos(peakTrendingScoreLabel)
                .move({0.0f, -8.0f})
                .color(secondaryTextColor);

        const float expectedAverage = creatorInfo.trending_score / creatorInfo.trending_level_count;
        const float scoreDifference = levelInfo.trending_score - expectedAverage;

        if (creatorInfo.trending_level_count > 1 && levelInfo.trending_score > 0.0 && !levelInfo.rate.has_value()) {
            auto percentageSprite = changeIcon(scoreDifference)
                    .scaleBy(0.5f)
                    .anchorPoint({0.0f, 0.5f})
                    .parent(trendingInfoNode)
                    .matchPos(creatorScoreLabel)
                    .move({creatorScoreLabel->getScaledContentWidth() + 3.0f, 0.0f})
                    .color(changeColor(scoreDifference, differenceRed, {255, 255, 255}, differenceGreen));

            auto percentageLabel = Build<CCLabelBMFont>::create(fmt::format("{:.1f}%", levelInfo.trending_score / creatorInfo.trending_score * 100.0f).c_str(), "chatFont.fnt")
                    .scale(0.5f)
                    .anchorPoint({0.0f, 0.5f})
                    .parent(trendingInfoNode)
                    .matchPos(percentageSprite)
                    .move({percentageSprite->getScaledContentWidth(), 0.0f})
                    .color(secondaryTextColor);

            Build(rock::RoundedRect::create(
                infoBoxColor,
                3.0f,
                {
                    percentageLabel->getScaledContentWidth() + percentageLabel->getPositionX() - percentageSprite->getPositionX() + 2.0f,
                    percentageSprite->getScaledContentHeight() + 1.0f
                }
            ))
                    .anchorPoint({0.0f, 0.5f})
                    .parent(trendingInfoNode)
                    .matchPos(percentageSprite)
                    .move({-1.0f, 0.0f})
                    .zOrder(-1);
        }
    }

    return true;
}

void LevelSendChartPopup::update(const float delta) {
    FLAlertLayer::update(delta);

    double trendingScore = SendUtils::calculateTrendingScore(TimeUtils::getCurrentTimestamp(), sendTimestamps);
    std::string trendingStr = trendingScore > 0.0 ? fmt::format("{:.2f}", trendingScore) : "N/A";
    trendingScoreLabel->setString(fmt::format("Trending Score: {}", trendingStr).c_str());

    peakTrendingScoreLabel->setString(fmt::format(
        "Peak Score: {:.2f} ({})",
        peakTrendingScore.score,
        TimeUtils::timestampToDate(peakTrendingScore.timestamp * 1000LL)
    ).c_str());
}

bool LevelSendChartPopup::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!FLAlertLayer::ccTouchBegan(touch, event)) return false;

    const CCPoint touchLocation = touch->getLocation();
    if (!PointUtils::isPointInsideNode(bg, touchLocation)) {
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