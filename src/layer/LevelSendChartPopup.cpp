#include "LevelSendChartPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

#include <UIBuilder.hpp>
#include <node/RankingNode.hpp>

#include <node/SendChartNode.hpp>
#include <node/SwitchNode.hpp>
#include <rock/RoundedRect.hpp>
#include <utils/FormatUtils.hpp>
#include <utils/Messages.hpp>
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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glLineWidth(1);

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
            .scale(0.75f)
            .intoMenuItem(this, menu_selector(LevelSendChartPopup::onClose))
            .pos({-5.0f, menuSize.y + 5.0f})
            .id("close-button")
            .parent(m_buttonMenu);

    Build<CCSprite>::createSpriteName("GJ_infoIcon_001.png")
            .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::Info);})
            .pos({menuSize.x + 5.0f, menuSize.y + 5.0f})
            .id("info-button")
            .parent(m_buttonMenu);

    Build<CCLabelBMFont>::create(level->m_levelName.c_str(), "goldFont.fnt")
            .anchorPoint({0.5f, 1.0f})
            .pos({menuSize.x / 2.0f, menuSize.y + 2.0f})
            .scale(0.85f)
            .id("title-label")
            .parent(m_buttonMenu);

    Build<CCLabelBMFont>::create(fmt::format("by {}", level->m_creatorName.empty() ? "Unknown" : level->m_creatorName).c_str(), "goldFont.fnt")
            .anchorPoint({0.0f, 1.0f})
            .pos({3.0f, menuSize.y - 18.0f})
            .scale(0.5f)
            .visible(false)
            .id("creator-label")
            .parent(m_buttonMenu);

    chartNode = Build(SendChartNode::create(
        levelData,
        CCSize(260, 150),
        1.0f,
        ChartType::Sends,
        chartStyleFromString(Mod::get()->getSettingValue<std::string>("graphStyle"))
    ))
            .anchorPoint({0.0f, 0.0f})
            .pos({30.0f, 20.0f})
            .id("send-chart")
            .parent(m_buttonMenu)
            .zOrder(1);

    auto chartSwitchSprite = SwitchNode::create();
    chartSwitchSprite->addNode(Build<CCSprite>::createSpriteName("GJ_starsIcon_001.png").scale(1.0f/0.94f));
    chartSwitchSprite->addNode(Build<CCSprite>::createSpriteName("GJ_sTrendingIcon_001.png").scale(1.0f/0.66f));
    chartSwitchSprite->setActiveIndex(0);

    Build(CircleButtonSprite::create(chartSwitchSprite, CircleBaseColor::Gray))
            .scale(0.75f)
            .intoMenuItem([this, chartSwitchSprite](auto*) {
                switch (chartNode->getType()) {
                    case ChartType::Sends:
                        chartNode->setType(ChartType::Trending);
                        chartSwitchSprite->setActiveIndex(1);
                        break;
                    case ChartType::Trending:
                        chartNode->setType(ChartType::Sends);
                        chartSwitchSprite->setActiveIndex(0);
                        break;
                }
            })
            .pos({-5.0f, -5.0f})
            .id("chart-button")
            .parent(m_buttonMenu);

    chartSwitchSprite->setScale(1.0f); // ???

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
                .pos({297.0f, 176.0f})
                .layout(ColumnLayout::create()
                    ->setAxisAlignment(AxisAlignment::End)
                    ->setCrossAxisLineAlignment(AxisAlignment::Start)
                    ->setAxisReverse(true)
                    ->setAutoGrowAxis(true)
                    ->setAutoScale(false)
                    ->setGap(3.0f)
                )
                .id("ranking-menu")
                .parent(m_buttonMenu);

        Build<CCLabelBMFont>::create("Rankings", "bigFont.fnt")
                .anchorPoint({0.0f, 0.5f})
                .pos({290.0f, 170.0f})
                .scale(0.6f)
                .visible(false)
                .id("ranking-label")
                .parent(m_buttonMenu);

        Build<RankingNode>::create(levelInfo.rank, "All", std::nullopt, RankingFilter::SendDB)
                .scale(0.6f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::RankingAll);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .id("all-ranking-node")
                .parent(menu);

        if (const auto it = std::ranges::find(levelIDs, levelID); it != levelIDs.end()) {
            const size_t index = it - levelIDs.begin();
            const int rank = static_cast<int>(index) + 1;
            Build<RankingNode>::create(rank, "Creator", static_cast<int>(levelIDs.size()), RankingFilter::User)
                    .scale(0.6f)
                    .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::RankingCreator);})
                    .animationEnabled(false)
                    .anchorPoint({0.5f, 0.5f})
                    .id("creator-ranking-node")
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
                    .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::RankingTrending);})
                    .animationEnabled(false)
                    .anchorPoint({0.5f, 0.5f})
                    .id("trending-ranking-node")
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
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::RankingRate);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .id("rate-ranking-node")
                .parent(menu);

        Build<RankingNode>::create(levelInfo.gamemode_rank, gamemodeText, std::nullopt, gamemodeFilter)
                .scale(0.6f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::RankingGamemode);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .id("gamemode-ranking-node")
                .parent(menu);

        Build<RankingNode>::create(levelInfo.joined_rank, fmt::format("{} {}", rateText, gamemodeText), std::nullopt, rateFilter, gamemodeFilter)
                .scale(0.6f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::RankingJoined);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .id("joined-ranking-node")
                .parent(menu);

        menu->updateLayout();


        auto sendInfoNode = Build<CCMenu>::create()
                .ignoreAnchorPointForPos(false)
                .scale(0.95f)
                .anchorPoint({0.0f, 0.0f})
                .pos({13.0f, menuSize.y - 53.0f})
                .contentSize({180.0f, 25.0f})
                .id("send-info-node")
                .parent(m_buttonMenu);

        Build<CCSprite>::createSpriteName("GJ_starsIcon_001.png")
                .scale(0.9f/0.94f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::SendCategory);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .pos({10.0f, 10.0f})
                .id("send-icon")
                .parent(sendInfoNode);

        auto sendCountLabel = Build<CCLabelBMFont>::create(fmt::format("Send Count: {}", levelInfo.sends.size()).c_str(), "bigFont.fnt")
                .limitLabelWidth(110.0f, 0.45f, 0.25f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::SendCount);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .pos({25.0f, sendInfoNode->getContentHeight() - 10.0f})
                .id("send-count-label")
                .parent(sendInfoNode);

        const float creatorSendAverage = creatorInfo.send_count / static_cast<float>(creatorInfo.levels.size());
        const float sendDifference = levelInfo.sends.size() - creatorSendAverage;

        if (creatorInfo.levels.size() > 1) {
            auto differenceSprite = changeIcon(sendDifference)
                    .scaleBy(0.6f)
                    .anchorPoint({0.0f, 0.5f})
                    .id("difference-sprite")
                    .parent(sendInfoNode)
                    .matchPos(sendCountLabel)
                    .move({sendCountLabel->getScaledContentWidth() + 3.0f, 0.0f})
                    .color(changeColor(sendDifference, differenceGreen, {255, 255, 255}, differenceRed));

            auto differenceLabel = Build<CCLabelBMFont>::create(FormatUtils::formatFloat(sendDifference, 2, "+").c_str(), "chatFont.fnt")
                    .scale(0.6f)
                    .anchorPoint({0.0f, 0.5f})
                    .id("difference-label")
                    .parent(sendInfoNode)
                    .matchPos(differenceSprite)
                    .move({differenceSprite->getScaledContentWidth() - 0.5f, 0.0f})
                    .color(secondaryTextColor);

            Build(rock::RoundedRect::create(
                infoBoxColor,
                3.0f,
                {
                    differenceLabel->getScaledContentWidth() + differenceLabel->getPositionX() - differenceSprite->getPositionX() + 4.0f,
                    differenceSprite->getScaledContentHeight() + 3.0f
                }
            ))
                    .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::SendDifference);})
                    .animationEnabled(false)
                    .anchorPoint({0.0f, 0.5f})
                    .id("difference-bg")
                    .parent(sendInfoNode)
                    .matchPos(differenceSprite)
                    .move({-2.0f, 0.0f})
                    .zOrder(-1);
        }

        Build<CCLabelBMFont>::create(fmt::format("Creator Average: {}", FormatUtils::formatFloat(creatorSendAverage, 2)).c_str(), "bigFont.fnt")
                .scale(0.3f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::SendCreatorAverage);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .id("creator-average-label")
                .parent(sendInfoNode)
                .matchPos(sendCountLabel)
                .move({0.0f, -12.0f})
                .color(secondaryTextColor);


        auto trendingInfoNode = Build<CCMenu>::create()
                .ignoreAnchorPointForPos(false)
                .scale(0.95f)
                .anchorPoint({1.0f, 0.0f})
                .pos({menuSize.x - 13.0f, menuSize.y - 53.0f})
                .contentSize({190.0f, 30.0f})
                .id("trending-info-node")
                .parent(m_buttonMenu);

        Build<CCSprite>::createSpriteName("GJ_sTrendingIcon_001.png")
                .scale(0.9f/0.66f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::TrendingCategory);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .pos({10.0f, 10.0f})
                .id("trending-icon")
                .parent(trendingInfoNode);

        if (levelInfo.rate.has_value()) {
            Build<CCSprite>::createSpriteName("edit_delBtnSmall_001.png")
                    .scale(0.4f/0.52f)
                    .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::TrendingNotEligible);})
                    .animationEnabled(false)
                    .anchorPoint({0.5f, 0.5f})
                    .pos({17.0f, 3.0f})
                    .id("trending-unavailable-icon")
                    .parent(trendingInfoNode);
        }

        auto trendingScoreLabelButton = Build<CCLabelBMFont>::create(fmt::format("Trending Score: {:.2f}", levelInfo.trending_score).c_str(), "bigFont.fnt")
                .limitLabelWidth(165.0f, 0.4f, 0.2f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::TrendingScore);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .pos({25.0f, trendingInfoNode->getContentHeight() - 10.0f})
                .id("trending-score-label")
                .parent(trendingInfoNode);
        trendingScoreLabel = typeinfo_cast<CCLabelBMFont*>(trendingScoreLabelButton->getNormalImage());
        trendingScoreLabel->setPositionX(0.0f);
        trendingScoreLabel->setAnchorPoint({0.0f, 0.5f});

        sendTimestamps = geode::utils::ranges::map<std::vector<int>>(levelInfo.sends, [](const Send& s) {
            return s.timestamp / 1000;
        });

        peakTrendingScore = SendUtils::calculatePeakTrendingScore(sendTimestamps);

        auto peakTrendingScoreLabelButton = Build<CCLabelBMFont>::create(fmt::format(
            "Peak Score: {:.2f} ({})",
            peakTrendingScore.score,
            TimeUtils::timestampToDate(peakTrendingScore.timestamp * 1000LL)
        ).c_str(), "bigFont.fnt")
                .limitLabelWidth(165.0f, 0.3f, 0.15f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::TrendingPeakScore);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .id("peak-trending-score-label")
                .parent(trendingInfoNode)
                .matchPos(trendingScoreLabelButton)
                .move({0.0f, -10.0f})
                .color(secondaryTextColor)
                .animationEnabled(false);
        peakTrendingScoreLabel = typeinfo_cast<CCLabelBMFont*>(peakTrendingScoreLabelButton->getNormalImage());
        peakTrendingScoreLabel->setPositionX(0.0f);
        peakTrendingScoreLabel->setAnchorPoint({0.0f, 0.5f});

        auto creatorScoreLabel = Build<CCLabelBMFont>::create(fmt::format("Creator Score: {:.2f}", creatorInfo.trending_score).c_str(), "bigFont.fnt")
                .limitLabelWidth(125.0f, 0.3f, 0.15f)
                .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::TrendingCreatorScore);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .id("creator-score-label")
                .parent(trendingInfoNode)
                .matchPos(peakTrendingScoreLabelButton)
                .move({0.0f, -10.0f})
                .color(secondaryTextColor);

        const float expectedAverage = creatorInfo.trending_score / creatorInfo.trending_level_count;
        const float scoreDifference = levelInfo.trending_score - expectedAverage;

        if (creatorInfo.trending_level_count > 1 && levelInfo.trending_score > 0.0 && !levelInfo.rate.has_value()) {
            auto percentageSprite = changeIcon(scoreDifference)
                    .scaleBy(0.5f)
                    .anchorPoint({0.0f, 0.5f})
                    .id("score-contribution-sprite")
                    .parent(trendingInfoNode)
                    .matchPos(creatorScoreLabel)
                    .move({creatorScoreLabel->getScaledContentWidth() + 4.0f, 0.0f})
                    .color(changeColor(scoreDifference, differenceRed, {255, 255, 255}, differenceGreen));

            auto percentageLabel = Build<CCLabelBMFont>::create(fmt::format("{}%", FormatUtils::formatFloat(levelInfo.trending_score / creatorInfo.trending_score * 100.0f, 1)).c_str(), "chatFont.fnt")
                    .scale(0.5f)
                    .anchorPoint({0.0f, 0.5f})
                    .id("score-contribution-label")
                    .parent(trendingInfoNode)
                    .matchPos(percentageSprite)
                    .move({percentageSprite->getScaledContentWidth(), 0.0f})
                    .color(secondaryTextColor);

            Build(rock::RoundedRect::create(
                infoBoxColor,
                3.0f,
                {
                    percentageLabel->getScaledContentWidth() + percentageLabel->getPositionX() - percentageSprite->getPositionX() + 4.0f,
                    percentageSprite->getScaledContentHeight() + 3.0f
                }
            ))
                    .intoMenuItem([](auto*) {infoPopup(LevelSendPopupInfo::TrendingContribution);})
                    .animationEnabled(false)
                    .anchorPoint({0.0f, 0.5f})
                    .id("score-contribution-bg")
                    .parent(trendingInfoNode)
                    .matchPos(percentageSprite)
                    .move({-2.0f, 0.0f})
                    .zOrder(-1);
        }
    }

    return true;
}

void LevelSendChartPopup::update(const float delta) {
    FLAlertLayer::update(delta);

    const bool rated = levelData.value_or(Level{}).rate.has_value();
    double trendingScore = SendUtils::calculateTrendingScore(TimeUtils::getCurrentTimestamp(), sendTimestamps);
    std::string trendingStr = trendingScore > 0.0 && rated ? fmt::format("{:.2f}", trendingScore) : "N/A";
    trendingScoreLabel->setString(fmt::format("Trending Score: {}", trendingStr).c_str());

    peakTrendingScoreLabel->setString(fmt::format(
        "Peak Score: {} ({})",
        FormatUtils::formatFloat(peakTrendingScore.score, 2),
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

void LevelSendChartPopup::onClose(CCObject*) {
    setKeypadEnabled(false);
    removeFromParentAndCleanup(true);
}

void LevelSendChartPopup::infoPopup(const LevelSendPopupInfo& info) {
    FLAlertLayer::create(
        Messages::getSendPopupInfoTitle(info).c_str(),
        Messages::getSendPopupInfoContent(info).c_str(),
        "OK"
    )->show();
}