#include "LeaderboardLayer.hpp"

#include <ranges>

#include <UIBuilder.hpp>
#include <hook/LevelCell.hpp>
#include <manager/SendDBIntegration.hpp>
#include <utils/Messages.hpp>
#include <utils/TimeUtils.hpp>

#include "FadeSpinner.hpp"
#include "LeaderboardFilterPopup.hpp"

bool shadersEnabled = true;

bool LeaderboardLayer::init() {
    if (!BaseLayer::init()) return false;
    shadersEnabled = Mod::get()->getSettingValue<bool>("shaders");

    if (shadersEnabled) initShaderBackground("leaderboard.fsh");
    else initBackground();

    const auto winSize = CCDirector::sharedDirector()->getWinSize();

    const bool expandedListView = Mod::get()->getSettingValue<bool>("expandedListView");

    list = Build(cue::ListNode::create(
        {358.0f, expandedListView? 320.0f : 220.0f},
        shadersEnabled || expandedListView ? ccColor4B{0, 0, 0, 0} : cue::Brown,
        expandedListView ? cue::ListBorderStyle::None : cue::ListBorderStyle::SlimLevels
    ))
            .zOrder(2)
            .anchorPoint({0.5f, 0.5f})
            .pos(winSize / 2.0f)
            .id("level-list")
            .parent(this);


    if (shadersEnabled) {
        list->setCellColor(ccColor4B{0, 0, 0, 80});

        const auto shader = ShaderNode::create("generic.vsh", "kawase.fsh");
        if (shader) {
            listBackground = Build(shader)
                    .zOrder(-10)
                    .anchorPoint({0.5f, 0.5f})
                    .pos(list->getContentSize() / 2.0f)
                    .contentSize(list->getContentSize() + CCSize(10.0f, 10.0f))
                    .id("list-background")
                    .parent(list);

            listBackground->setPasses(Mod::get()->getSettingValue<int>("blurPasses"));
        } else shadersEnabled = false;
    }

    const auto menu = Build<CCMenu>::create()
            .pos(0.f, 0.f)
            .zOrder(2)
            .parent(this)
            .collect();

    pageLabel = Build<CCLabelBMFont>::create("0 to 0 of 0", "goldFont.fnt")
        .scale(0.44f)
        .pos({winSize.width - 3.0f, winSize.height - 4.0f})
        .anchorPoint({1.0f, 1.0f})
        .parent(menu);

    Build<CCSprite>::createSpriteName("GJ_infoIcon_001.png")
        .intoMenuItem([](auto) {
            FLAlertLayer::create(
                leaderboardTitle,
                leaderboardDesc,
                "OK"
            )->show();
        })
        .pos(32.0f, 32.0f)
        .parent(menu);

    filterButton = Build<CCSprite>::createSpriteName("GJ_plusBtn_001.png")
        .scale(0.7f)
        .intoMenuItem([this](auto) {
            LeaderboardFilterPopup::create(query, [this](auto res) {
                query = res;
                query.offset = 0;
                onRefresh();
            })->show();
        })
        .pos(25.0f, winSize.height - 70.0f)
        .parent(menu);

    constexpr float pageBtnPadding = 20.0f;

    prevPageButton = Build<CCSprite>::createSpriteName("GJ_arrow_03_001.png")
            .intoMenuItem([this](auto) {
                onPrevPage();
            })
            .pos(pageBtnPadding, winSize.height / 2.0f)
            .parent(menu);

    nextPageButton = Build<CCSprite>::createSpriteName("GJ_arrow_03_001.png")
            .flipX(true)
            .intoMenuItem([this](auto) {
                onNextPage();
            })
            .pos(winSize.width - pageBtnPadding, winSize.height / 2.0f)
            .parent(menu);

    auto pageSprite = Build<CCSprite>::create("GJ_button_02.png");
    pageText = Build<CCLabelBMFont>::create("0", "bigFont.fnt")
            .pos(pageSprite->getContentSize() / 2.0f + ccp(0.0f, 1.0f))
            .limitLabelWidth(32.0f, 0.8f, 0.0f)
            .id("page-text")
            .parent(pageSprite);

    pageButton = pageSprite.intoMenuItem([this](auto) {
        const auto popup = SetIDPopup::create(
            query.offset / query.limit + 1,
            1,
            (queryTotal + query.limit - 1) / query.limit,
            "Go to page",
            "Go",
            false,
            1,
            60.0f,
            false,
            false
        );

        popup->setTag(3);
        popup->m_delegate = this;
        popup->show();
    })
            .pos({winSize.width - 23.0f, winSize.height / 2.0f + 122.0f})
            .parent(menu);

    loadingCircle = Build<FadeSpinner>::create()
            .zOrder(-1)
            .pos(list->getContentSize() / 2.0f)
            .parent(list);

    onRefresh();

    return true;
}

LeaderboardLayer::~LeaderboardLayer() {
    GameLevelManager::get()->m_levelManagerDelegate = nullptr;
}

void LeaderboardLayer::onRefresh() {
    if (loading) return;
    loading = true;

    toggleLoadingUi(true);
    pageLabel->setVisible(false);
    list->clear();

    SendDBIntegration::get()->getLeaderboard(query, [this](const std::optional<LeaderboardResponse>& response) {
        if (response.has_value()) {
            auto responseData = response.value();
            const auto levelIDs = ranges::map<std::vector<int>>(responseData.levels, [](const LeaderboardLevel& c) {
                return c.levelID;
            });
            getSendCounts(levelIDs);
            onLoaded(responseData.levels, responseData.total);
        }
    }, leaderboardListener);
}

void LeaderboardLayer::getSendCounts(const std::vector<int>& levelIDs) {
    sendCountListeners = SendDBIntegration::get()->getLevels(levelIDs, [this](const std::optional<std::vector<BatchLevel>>& levels) {
        if (levels.has_value()) {
            batchCache.clear();
            for (const auto& level : levels.value()) {
                batchCache[level.levelID] = level;
            }
            updateSendCounts();
        }
    });
}

void LeaderboardLayer::updateSendCounts() {
    for (const auto levelObj : CCArrayExt<CCObject*>(list->getCells())) {
        if (const auto genericCell = typeinfo_cast<cue::ListCell*>(levelObj)) {
            const auto cell = static_cast<SendDBLevelCell*>(genericCell->getInner());
            if (const auto levelID = cell->m_level->m_levelID.value(); batchCache.contains(levelID)) {
                cell->setLevelInfo(batchCache[levelID]);
            }
        }
    }
}

void LeaderboardLayer::onLoaded(const std::vector<LeaderboardLevel>& levels, const int total) {
    pageLabel->setVisible(true);
    pageLabel->setString(fmt::format("{} to {} of {}", query.offset + 1, query.offset + query.limit, total).c_str());
    pageText->setString(fmt::format("{}", query.offset / query.limit + 1).c_str());
    pageText->limitLabelWidth(32.0f, 0.8f, 0.0f);
    queryTotal = total;
    pageLevels = levels;
    startLoadingForPage();
}

void LeaderboardLayer::setReady(float) {
    loading = false;
}

void LeaderboardLayer::startLoadingForPage() {
    loading = true;

    toggleLoadingUi(true);

    if (pageLevels.empty()) {
        finishLoading();
        return;
    }

    continueLoading();
}

void LeaderboardLayer::continueLoading() {
    if (!loadNextBatch()) finishLoading();
}

void LeaderboardLayer::finishLoading() {
    std::vector<Ref<GJGameLevel>> page;

    int unloadedLevels = 0;

    for (const auto level : pageLevels) {
        const auto id = level.levelID;
        if (cache.contains(id)) page.push_back(cache[id]);
        else if (!failedCache.contains(id)) unloadedLevels++;
    }

    list->clear();

    const long long globalStartTime = TimeUtils::getCurrentTimestamp();
    for (auto level : page) {
        const auto cell = static_cast<SendDBLevelCell*>(new LevelCell("", 356.f, 90.f));
        cell->autorelease();

        cell->loadFromLevel(level);

        cell->setContentSize({356.f, 90.f});
        list->addCell(cell);
    }

    const long long globalEndTime = TimeUtils::getCurrentTimestamp();
    log::debug("total load time for page: {} ms ({} unloaded levels)", globalEndTime - globalStartTime, unloadedLevels);

    list->updateLayout();
    updateSendCounts();

    toggleLoadingUi(false);

    prevPageButton->setVisible(query.offset > 0);
    nextPageButton->setVisible(query.offset + query.limit < queryTotal);

    scheduleOnce(schedule_selector(LeaderboardLayer::setReady), 0.0f);
}

bool LeaderboardLayer::loadNextBatch() {
    currentQuery.clear();

    for (const auto level : pageLevels) {
        const auto id = level.levelID;
        if (cache.contains(id) || failedCache.contains(id)) continue;

        currentQuery.push_back(id);

        if (currentQuery.size() == 100) break;
    }

    if (currentQuery.empty()) return false;

    const auto glm = GameLevelManager::get();
    glm->m_levelManagerDelegate = this;

    const auto searchObject = GJSearchObject::create(SearchType::Type26, fmt::format("{}", fmt::join(currentQuery, ",")));
    const auto cachedOnlineLevels = glm->getStoredOnlineLevels(searchObject->getKey());
    if (cachedOnlineLevels && cachedOnlineLevels->count() > 0) {
        loadLevelsFinished(cachedOnlineLevels, searchObject->getKey());
        return true;
    }

    glm->getOnlineLevels(searchObject);

    return true;
}

void LeaderboardLayer::keyDown(const enumKeyCodes key) {
    BaseLayer::keyDown(key);

    if (loading) return;

    switch (key) {
        case CONTROLLER_Left:
        case KEY_Left:
            if (prevPageButton->isVisible()) onPrevPage();
            break;

        case CONTROLLER_Right:
        case KEY_Right:
            if (nextPageButton->isVisible()) onNextPage();
            break;

        default:
            break;
    }
}

void LeaderboardLayer::onNextPage() {
    if (loading) return;

    query.offset += query.limit;
    onRefresh();
}

void LeaderboardLayer::onPrevPage() {
    if (loading) return;

    query.offset = std::max(0, query.offset - query.limit);
    onRefresh();
}

void LeaderboardLayer::loadLevelsFinished(CCArray* levels, const char* key) {
    loadLevelsFinished(levels, key, -1);
}

void LeaderboardLayer::loadLevelsFinished(CCArray* levels, const char*, int) {
    for (GJGameLevel* level : CCArrayExt<GJGameLevel*>(levels)) {
        cache[level->m_levelID.value()] = level;
    }

    for (auto id : currentQuery) {
        if (!cache.contains(id)) {
            failedCache.insert(id);
        }
    }

    continueLoading();
}

void LeaderboardLayer::setIDPopupClosed(SetIDPopup* popup, const int value) {
    if (loading) return;

    query.offset = (value - 1) * query.limit;
    onRefresh();
}

void LeaderboardLayer::toggleLoadingUi(const bool loadingState) {
    filterButton->setVisible(!loadingState);
    prevPageButton->setVisible(!loadingState);
    nextPageButton->setVisible(!loadingState);
    pageButton->setVisible(!loadingState);

    const bool prevShown = circleShown;
    circleShown = loadingState;

    if (prevShown != loadingState && loadingCircle) {
        if (loadingState) loadingCircle->fadeIn(0.25f);
        else loadingCircle->fadeOut(0.0f);
    }
}

LeaderboardLayer* LeaderboardLayer::create() {
    const auto ret = new LeaderboardLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}