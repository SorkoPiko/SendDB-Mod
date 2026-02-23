#include "CreatorInfoPopup.hpp"

#include <node/DiscordNode.hpp>
#include <node/RankingNode.hpp>
#include <utils/FormatUtils.hpp>
#include <utils/PointUtils.hpp>
#include <utils/Style.hpp>
#include <utils/TimeUtils.hpp>

constexpr CCPoint popupSize = {350.0f, 200.0f};
constexpr CCPoint menuSize = {340.0f, 190.0f};

std::unordered_map<int, Ref<GJGameLevel>> CreatorInfoPopup::cache = {};
std::set<int> CreatorInfoPopup::failedLevelIDs = {};

bool CreatorInfoPopup::init(const GJUserScore* creator, const std::optional<Creator>& _creatorData) {
    if (!FLAlertLayer::init(75)) return false;

    playerID = creator->m_userID;
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
            .intoMenuItem(this, menu_selector(CreatorInfoPopup::onClose))
            .pos({-5.0f, menuSize.y + 5.0f})
            .id("close-button")
            .parent(m_buttonMenu);

    Build<CCSprite>::createSpriteName("GJ_infoIcon_001.png")
            .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::Info);})
            .pos({menuSize.x + 5.0f, menuSize.y + 5.0f})
            .id("info-button")
            .parent(m_buttonMenu);

    Build<DiscordNode>::create()
            .anchorPoint({0.5f, 0.5f})
            .pos({menuSize.x - 3.0f, 3.0f})
            .id("discord-button")
            .parent(m_buttonMenu);

    Build<CCLabelBMFont>::create(creator->m_userName.c_str(), "goldFont.fnt")
            .anchorPoint({0.5f, 1.0f})
            .pos({menuSize.x / 2.0f, menuSize.y + 2.0f})
            .scale(0.9f)
            .id("title-label")
            .parent(m_buttonMenu);

    if (creatorData.has_value()) {
        const Creator creatorInfo = creatorData.value();

        auto menu = Build<CCMenu>::create()
                .anchorPoint({0.0f, 1.0f})
                .pos({247.0f, 110.0f})
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

        Build<RankingNode>::create(creatorInfo.rank, "All", std::nullopt, RankingFilter::SendDB)
                .scale(0.6f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::RankingAll);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .id("all-ranking-node")
                .parent(menu);

        if (creatorInfo.trending_score > 0.0 && creatorInfo.trending_rank > 0) {
            Build<RankingNode>::create(creatorInfo.trending_rank, "Trending", std::nullopt, RankingFilter::Trending)
                    .scale(0.6f)
                    .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::RankingTrending);})
                    .animationEnabled(false)
                    .anchorPoint({0.5f, 0.5f})
                    .id("trending-ranking-node")
                    .parent(menu);
        }

        menu->updateLayout();

        auto sendInfoNode = Build<CCMenu>::create()
                .ignoreAnchorPointForPos(false)
                .scale(0.95f)
                .anchorPoint({0.0f, 0.0f})
                .pos({13.0f, menuSize.y - 53.0f})
                .contentSize({160.0f, 25.0f})
                .id("send-info-node")
                .parent(m_buttonMenu);

        Build<CCSprite>::createSpriteName("GJ_starsIcon_001.png")
                .scale(0.9f/0.94f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::SendCategory);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .pos({10.0f, 10.0f})
                .id("send-icon")
                .parent(sendInfoNode);

        auto sendCountLabel = Build<CCLabelBMFont>::create(fmt::format("Total Sends: {}", creatorInfo.send_count).c_str(), "bigFont.fnt")
                .limitLabelWidth(135.0f, 0.45f, 0.25f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::SendTotalSends);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .pos({25.0f, sendInfoNode->getContentHeight() - 10.0f})
                .id("send-count-label")
                .parent(sendInfoNode);

        auto recentSendsLabel = Build<CCLabelBMFont>::create(fmt::format("Recent Sends: {}", creatorInfo.recent_sends).c_str(), "bigFont.fnt")
                .scale(0.3f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::SendRecentSends);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .id("recent-sends-label")
                .parent(sendInfoNode)
                .matchPos(sendCountLabel)
                .move({0.0f, -13.0f})
                .color(secondaryTextColor);

        const float sendAverage = creatorInfo.send_count / static_cast<float>(creatorInfo.levels.size());
        auto averageSendsLabel = Build<CCLabelBMFont>::create(fmt::format("Average Sends: {:.2f}", sendAverage).c_str(), "bigFont.fnt")
                .scale(0.3f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::SendAverageSends);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .id("creator-average-label")
                .parent(sendInfoNode)
                .matchPos(recentSendsLabel)
                .move({0.0f, -9.0f})
                .color(secondaryTextColor);

        Build<CCLabelBMFont>::create(fmt::format("Sent Levels: {}", creatorInfo.levels.size()).c_str(), "bigFont.fnt")
                .scale(0.3f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::SendLevelCount);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .id("creator-average-label")
                .parent(sendInfoNode)
                .matchPos(averageSendsLabel)
                .move({0.0f, -9.0f})
                .color(secondaryTextColor);

        auto trendingInfoNode = Build<CCMenu>::create()
                .ignoreAnchorPointForPos(false)
                .scale(0.95f)
                .anchorPoint({1.0f, 0.0f})
                .pos({menuSize.x - 13.0f, menuSize.y - 53.0f})
                .contentSize({160.0f, 25.0f})
                .id("trending-info-node")
                .parent(m_buttonMenu);

        Build<CCSprite>::createSpriteName("GJ_sTrendingIcon_001.png")
                .scale(0.9f/0.66f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::TrendingCategory);})
                .animationEnabled(false)
                .anchorPoint({0.5f, 0.5f})
                .pos({10.0f, 10.0f})
                .id("trending-icon")
                .parent(trendingInfoNode);

        auto trendingScoreLabel = Build<CCLabelBMFont>::create(fmt::format("Trending Score: {:.2f}", creatorInfo.trending_score).c_str(), "bigFont.fnt")
                .limitLabelWidth(135.0f, 0.4f, 0.2f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::TrendingTotalScore);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .pos({25.0f, trendingInfoNode->getContentHeight() - 10.0f})
                .id("trending-score-label")
                .parent(trendingInfoNode);

        auto trendingAverageLabel = Build<CCLabelBMFont>::create(fmt::format("Average Score: {}", FormatUtils::formatFloat(creatorInfo.trending_score / creatorInfo.levels.size(), 2)).c_str(), "bigFont.fnt")
                .scale(0.3f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::TrendingAverageScore);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .id("trending-average-label")
                .parent(trendingInfoNode)
                .matchPos(trendingScoreLabel)
                .move({0.0f, -11.0f})
                .color(secondaryTextColor);

        Build<CCLabelBMFont>::create(fmt::format("Trending Levels: {}", creatorInfo.trending_level_count).c_str(), "bigFont.fnt")
                .scale(0.3f)
                .intoMenuItem([](auto*) {infoPopup(CreatorPopupInfo::TrendingLevelCount);})
                .animationEnabled(false)
                .anchorPoint({0.0f, 0.5f})
                .id("trending-levels-label")
                .parent(trendingInfoNode)
                .matchPos(trendingAverageLabel)
                .move({0.0f, -9.0f})
                .color(secondaryTextColor);

        levelList = Build(cue::ListNode::create(
            {230.0f, 100.0f},
            gridBackgroundColor,
            cue::ListBorderStyle::None
        ))
                .anchorPoint({0.0f, 0.0f})
                .pos({10.0f, 10.0f})
                .id("level-list")
                .parent(m_buttonMenu);

        levelList->setCellColor(ccColor4B{0, 0, 0, 0});
        fixTouchPrio();

        std::vector<CreatorLevel> sortedLevels = creatorInfo.levels;
        std::sort(sortedLevels.begin(), sortedLevels.end(), [](const CreatorLevel& a, const CreatorLevel& b) {
            return a.send_count > b.send_count;
        });

        for (const auto& level : sortedLevels) {
            SentLevelNode* node = SentLevelNode::create(
                {230.0f, 30.0f},
                level.levelID,
                level.send_count,
                true
            );
            levelList->addCell(node);
            levelNodes[level.levelID] = node;
        }

        updateLevelNodes();
    }

    return true;
}

CreatorInfoPopup::~CreatorInfoPopup() {
    const auto glm = GameLevelManager::get();
    if (glm->m_levelManagerDelegate == this) {
        glm->m_levelManagerDelegate = nullptr;
    }
}

void CreatorInfoPopup::updateLevelNodes() {
    for (const auto& level : creatorData->levels) {
        SentLevelNode* node = levelNodes.contains(level.levelID) ? levelNodes[level.levelID] : nullptr;
        if (cache.contains(level.levelID)) {
            if (node) node->update(cache[level.levelID]);
        } else {
            const auto saved = GameLevelManager::get()->getSavedLevel(level.levelID);
            if (saved) {
                cache[level.levelID] = saved;
                if (node) node->update(saved);
            } else levelIDsToFetch.insert(level.levelID);
        }
    }

    for (const int levelID : failedLevelIDs) {
        levelIDsToFetch.erase(levelID);
        if (levelNodes.contains(levelID)) {
            levelNodes[levelID]->update(nullptr);
        }
    }

    if (!levelIDsToFetch.empty()) downloadLevelInfo();
}

void CreatorInfoPopup::downloadLevelInfo() {
    if (!creatorData.has_value()) return;

    const auto glm = GameLevelManager::get();

    pendingLevelIDs.clear();
    for (const int levelID : levelIDsToFetch) {
        if (failedLevelIDs.contains(levelID)) continue;
        const auto saved = glm->getSavedLevel(levelID);
        if (!saved) {
            if (pendingLevelIDs.size() < 100) pendingLevelIDs.insert(levelID);
        }
    }

    if (pendingLevelIDs.empty()) return;

    glm->m_levelManagerDelegate = this;

    const auto searchObject = GJSearchObject::create(SearchType::Type26, fmt::format("{}", fmt::join(pendingLevelIDs, ",")));
    const auto cachedOnlineLevels = glm->getStoredOnlineLevels(searchObject->getKey());
    if (cachedOnlineLevels && cachedOnlineLevels->count() > 0) {
        loadLevelsFinished(cachedOnlineLevels, searchObject->getKey());
        return;
    }

    glm->getOnlineLevels(searchObject);
}

bool CreatorInfoPopup::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!FLAlertLayer::ccTouchBegan(touch, event)) return false;

    const CCPoint touchLocation = touch->getLocation();
    if (!PointUtils::isPointInsideNode(bg, touchLocation)) onClose(nullptr);

    return true;
}

void CreatorInfoPopup::loadLevelsFinished(CCArray* levels, const char* key) {
    loadLevelsFinished(levels, key, -1);
}

void CreatorInfoPopup::loadLevelsFinished(CCArray* levels, const char*, int) {
    for (GJGameLevel* level : CCArrayExt<GJGameLevel*>(levels)) {
        if (!level || level->m_levelName.empty()) continue;
        cache[level->m_levelID.value()] = level;
    }

    for (const int levelID : pendingLevelIDs) {
        levelIDsToFetch.erase(levelID);
        if (cache.contains(levelID)) {
            const auto levelData = cache[levelID];
            if (levelNodes.contains(levelID)) {
                levelNodes[levelID]->update(levelData);
            }
        } else {
            failedLevelIDs.insert(levelID);
            if (levelNodes.contains(levelID)) {
                levelNodes[levelID]->update(nullptr);
            }
        }
    }

    pendingLevelIDs.clear();

    if (!levelIDsToFetch.empty()) downloadLevelInfo();
}

void CreatorInfoPopup::loadLevelsFailed(const char* key) {
    loadLevelsFailed(key, -1);
}

void CreatorInfoPopup::loadLevelsFailed(const char*, int) {
    for (const int levelID : pendingLevelIDs) {
        levelIDsToFetch.erase(levelID);
        failedLevelIDs.insert(levelID);
        if (levelNodes.contains(levelID)) {
            levelNodes[levelID]->update(nullptr);
        }
    }
    pendingLevelIDs.clear();

    downloadLevelInfo();
}

void CreatorInfoPopup::keyDown(const enumKeyCodes key, const double timestamp) {
    FLAlertLayer::keyDown(key, timestamp);

    if (key == KEY_Escape) {
        onClose(nullptr);
    }
}

void CreatorInfoPopup::onClose(CCObject*) {
    if (closeCallback) closeCallback();
    setKeypadEnabled(false);
    removeFromParentAndCleanup(true);
}

void CreatorInfoPopup::onEnter() {
    FLAlertLayer::onEnter();

    fixTouchPrio();
}

void fixTouchPrioNode(CCNode* node, const int prio) {
    if (const auto delegate = typeinfo_cast<CCTouchDelegate*>(node)) {
        if (const auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
            CCTouchDispatcher::get()->setPriority(prio, handler->getDelegate());
        }
    }
}

void CreatorInfoPopup::fixTouchPrio() {
    if (!levelList) return;

    fixTouchPrioNode(levelList->getScrollLayer(), -511);

    for (const auto cell : levelList->iterChecked<SentLevelNode>()) {
        fixTouchPrioNode(cell->getMenu(), -512);
    }
}

void CreatorInfoPopup::infoPopup(const CreatorPopupInfo& info) {
    FLAlertLayer::create(
        Messages::getCreatorPopupInfoTitle(info).c_str(),
        Messages::getCreatorPopupInfoContent(info),
        "OK"
    )->show();
}

CreatorInfoPopup* CreatorInfoPopup::create(const GJUserScore* creator, const std::optional<Creator>& creatorData) {
    if (const auto newLayer = new CreatorInfoPopup(); newLayer->init(creator, creatorData)) {
        newLayer->autorelease();
        return newLayer;
    } else {
        delete newLayer;
        return nullptr;
    }
}

void CreatorInfoPopup::show() {
    FLAlertLayer::show();
    handleTouchPriority(this);
}