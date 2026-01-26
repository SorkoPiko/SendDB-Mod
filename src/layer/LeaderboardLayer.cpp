#include "LeaderboardLayer.hpp"

#include <ranges>

#include <UIBuilder.hpp>
#include <hook/LevelCell.hpp>
#include <manager/SendDBIntegration.hpp>
#include <node/DragNode.hpp>
#include <node/SwitchNode.hpp>
#include <utils/Messages.hpp>
#include <utils/TimeUtils.hpp>

#include "FadeSpinner.hpp"
#include "LeaderboardFilterPopup.hpp"

bool shadersEnabled = true;

bool LeaderboardLayer::trending = false;
LeaderboardQuery LeaderboardLayer::query = {
    Mod::get()->getSettingValue<int>("leaderboardPerPage"),
    0,
    std::nullopt,
    std::nullopt
};

std::unordered_map<int, Ref<GJGameLevel>> LeaderboardLayer::cache = {};
std::set<int> LeaderboardLayer::failedCache = {};

bool LeaderboardLayer::init() {
    if (!BaseLayer::init()) return false;
    shadersEnabled = Mod::get()->getSettingValue<bool>("shaders");

    if (shadersEnabled) {
        initShaderBackground("trending.fsh");
        if (backgroundShader) {
            trendingBackground = backgroundShader;
            trendingBackground->setID("trending-background");
            trendingBackground->setVisible(trending);
        }

        initShaderBackground("leaderboard.fsh");
        if (!backgroundShader) {
            shadersEnabled = false;
            initBackground();
        } else {
            backgroundShader->allocateSprites(2);
            backgroundShader->setVisible(!trending);
        }
    }
    else initBackground();

    const auto winSize = CCDirector::sharedDirector()->getWinSize();

    const bool expandedListView = Mod::get()->getSettingValue<bool>("expandedListView");
    const int blurPasses = Mod::get()->getSettingValue<int>("blurPasses");

    if (!expandedListView) {
        title = Build<CCLabelBMFont>::create(trending ? trendingLeaderboardTitle : leaderboardTitle, "bigFont.fnt")
                .limitLabelWidth(300.0f, 0.8f, 0.0f)
                .pos({winSize.width / 2.0f, winSize.height / 2 + 126.0f})
                .anchorPoint({0.5f, 0.0f})
                .zOrder(3)
                .id("title-label")
                .parent(this);
    }

    list = Build(cue::ListNode::create(
        {358.0f, expandedListView ? 320.0f : 220.0f},
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

            listBackground->setPassCurrentFrame(true);
            listBackground->setOnlyScissorFinalPass(true);
            listBackground->setPasses(blurPasses);
        } else shadersEnabled = false;
    }

    const auto menu = Build<CCMenu>::create()
            .pos(0.f, 0.f)
            .zOrder(2)
            .id("main-menu")
            .parent(this);

    pageLabel = Build<CCLabelBMFont>::create("0 to 0 of 0", "goldFont.fnt")
            .scale(0.44f)
            .pos({winSize.width - 3.0f, winSize.height - 4.0f})
            .anchorPoint({1.0f, 1.0f})
            .id("page-label")
            .parent(menu);

    auto chartSwitchSprite = SwitchNode::create();
    chartSwitchSprite->addNode(Build<CCSprite>::createSpriteName("GJ_starsIcon_001.png").scale(1.0f/0.94f));
    chartSwitchSprite->addNode(Build<CCSprite>::createSpriteName("GJ_sTrendingIcon_001.png").scale(1.0f/0.66f));
    chartSwitchSprite->setActiveIndex(trending);

    trendingButton = Build(CircleButtonSprite::create(chartSwitchSprite, CircleBaseColor::Gray))
            .intoMenuItem([this, chartSwitchSprite](auto*) {
                if (trending) {
                    trending = false;
                    chartSwitchSprite->setActiveIndex(0);
                } else {
                    trending = true;
                    chartSwitchSprite->setActiveIndex(1);
                }
                if (trendingBackground) trendingBackground->setVisible(trending);
                if (backgroundShader) backgroundShader->setVisible(!trending);
                if (title) {
                    title->setString(trending ? trendingLeaderboardTitle : leaderboardTitle);
                    title->limitLabelWidth(300.0f, 0.8f, 0.0f);
                }
                query.offset = 0;
                onRefresh();
            })
            .pos({32.0f, 32.0f})
            .id("chart-button")
            .parent(menu);

    chartSwitchSprite->setScale(1.0f); // ???

    Build<CCSprite>::createSpriteName("GJ_infoIcon_001.png")
            .intoMenuItem([](auto) {
                FLAlertLayer::create(
                    trending ? trendingLeaderboardTitle : leaderboardTitle,
                    trending ? trendingLeaderboardDesc :leaderboardDesc,
                    "OK"
                )->show();
            })
            .pos({winSize.width - 32.0f, 32.0f})
            .id("info-button")
            .parent(menu);

    filterButton = Build<CCSprite>::createSpriteName("GJ_plusBtn_001.png")
            .scale(0.7f)
            .intoMenuItem([this](auto) {
                LeaderboardFilterPopup::create(query, [this](auto res) {
                    query = res;
                    query.offset = 0;
                    updateShaderSprites();
                    onRefresh();
                })->show();
            })
            .pos(25.0f, winSize.height - 70.0f)
            .id("filter-button")
            .parent(menu);

    constexpr float pageBtnPadding = 20.0f;

    prevPageButton = Build<CCSprite>::createSpriteName("GJ_arrow_03_001.png")
            .intoMenuItem([this](auto) {
                onPrevPage();
            })
            .pos(pageBtnPadding, winSize.height / 2.0f)
            .id("prev-page-button")
            .parent(menu);

    nextPageButton = Build<CCSprite>::createSpriteName("GJ_arrow_03_001.png")
            .flipX(true)
            .intoMenuItem([this](auto) {
                onNextPage();
            })
            .pos(winSize.width - pageBtnPadding, winSize.height / 2.0f)
            .id("next-page-button")
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
            .id("page-button")
            .parent(menu);

    loadingCircle = Build<FadeSpinner>::create()
            .zOrder(-1)
            .pos(list->getContentSize() / 2.0f)
            .id("loading-circle")
            .parent(list);

    if (shadersEnabled && blurPasses > 0) {
        auto dragNode = Build<DragNode>::create(-512)
                .zOrder(100)
                .pos(32.0f, 92.0f)
                .contentSize({50.0f, 50.0f})
                .id("drag-node")
                .parent(menu);

        const auto shader = ShaderNode::create("generic.vsh", "glass.fsh");
        if (shader) {
            Build(shader)
                    .anchorPoint({0.5f, 0.5f})
                    .pos(dragNode->getContentSize() / 2.0f)
                    .contentSize(dragNode->getContentSize())
                    .id("drag-shader")
                    .parent(dragNode);

            shader->setPassCurrentFrame(true);
            shader->setPasses(blurPasses * 2 + 1);
        }
    }

    shaderSprites.push_back(CCSprite::create("classic.png"_spr));
    shaderSprites.push_back(CCSprite::create("platformer.png"_spr));
    shaderSprites.push_back(CCSprite::create("rated.png"_spr));
    shaderSprites.push_back(CCSprite::create("unrated.png"_spr));
    updateShaderSprites();

    onRefresh();

    return true;
}

LeaderboardLayer::~LeaderboardLayer() {
    GameLevelManager::get()->m_levelManagerDelegate = nullptr;
}

void LeaderboardLayer::updateShaderSprites() {
    if (!shadersEnabled || !backgroundShader) return;

    std::vector<Ref<CCSprite>> sprites;
    if (query.gamemodeFilter == GamemodeFilter::Classic) {
        sprites.push_back(shaderSprites[0]);
    } else if (query.gamemodeFilter == GamemodeFilter::Platformer) {
        sprites.push_back(shaderSprites[1]);
    }

    if (query.rateFilter == RateFilter::Rated) {
        sprites.push_back(shaderSprites[2]);
    } else if (query.rateFilter == RateFilter::Unrated) {
        sprites.push_back(shaderSprites[3]);
    }

    backgroundShader->setSprites(sprites);
}

void LeaderboardLayer::onRefresh() {
    if (loading) return;
    loading = true;

    toggleLoadingUi(true);
    pageLabel->setVisible(false);
    list->clear();

    if (trending) {
        SendDBIntegration::get()->getTrendingLeaderboard(query, [this](const std::optional<TrendingLeaderboardResponse>& response) {
                if (response.has_value()) {
                    auto responseData = response.value();
                    const auto levelIDs = ranges::map<std::vector<int>>(responseData.levels, [](const TrendingLeaderboardLevel& c) {
                        return c.levelID;
                    });

                    ranks.clear();
                    trendingScores.clear();
                    for (const auto& level : responseData.levels) {
                        ranks[level.levelID] = level.rank;
                        trendingScores[level.levelID] = level.trending_score;
                    }

                    getSendCounts(levelIDs);
                    onLoaded(levelIDs, responseData.total);
                }
            },
            leaderboardListener
        );
    } else {
        SendDBIntegration::get()->getLeaderboard(query, [this](const std::optional<LeaderboardResponse>& response) {
            if (response.has_value()) {
                auto responseData = response.value();
                const auto levelIDs = ranges::map<std::vector<int>>(responseData.levels, [](const LeaderboardLevel& c) {
                    return c.levelID;
                });

                ranks.clear();
                trendingScores.clear();
                for (const auto& level : responseData.levels) {
                    ranks[level.levelID] = level.rank;
                }

                getSendCounts(levelIDs);
                onLoaded(levelIDs, responseData.total);
            }
        }, leaderboardListener);
    }
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

void LeaderboardLayer::onLoaded(const std::vector<int>& levels, const int total) {
    pageLabel->setVisible(true);
    pageLabel->setString(fmt::format("{} to {} of {}", query.offset + 1, query.offset + query.limit, total).c_str());
    pageText->setString(fmt::format("{}", query.offset / query.limit + 1).c_str());
    pageText->limitLabelWidth(32.0f, 0.8f, 0.0f);
    queryTotal = total;
    pageIDs = levels;
    startLoadingForPage();
}

void LeaderboardLayer::setReady(float) {
    loading = false;
}

void LeaderboardLayer::startLoadingForPage() {
    loading = true;

    toggleLoadingUi(true);

    if (pageIDs.empty()) {
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

    for (const int id : pageIDs) {
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

        int levelID = level->m_levelID.value();

        cell->setRank(ranks.contains(levelID) ? ranks[levelID] : 0);
        cell->setTrendingScore(trendingScores.contains(levelID) ? trendingScores[levelID] : 0.0);

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

    for (const int id : pageIDs) {
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

void LeaderboardLayer::loadLevelsFailed(const char* key) {
    loadLevelsFailed(key, -1);
}

void LeaderboardLayer::loadLevelsFailed(const char*, int) {
    for (auto id : currentQuery) {
        failedCache.insert(id);
    }

    continueLoading();
}

void LeaderboardLayer::setIDPopupClosed(SetIDPopup* popup, const int value) {
    if (loading) return;

    query.offset = (value - 1) * query.limit;
    onRefresh();
}

void LeaderboardLayer::toggleLoadingUi(const bool loadingState) {
    filterButton->setVisible(!loadingState && !trending);
    prevPageButton->setVisible(!loadingState);
    nextPageButton->setVisible(!loadingState);
    pageButton->setVisible(!loadingState);
    trendingButton->setVisible(!loadingState);

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