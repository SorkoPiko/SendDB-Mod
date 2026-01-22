#include "LeaderboardBrowserLayer.hpp"

#include <Geode/binding/AppDelegate.hpp>
#include <Geode/binding/BoomListView.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LoadingCircle.hpp>
#include <Geode/binding/TableView.hpp>
#include <Geode/utils/ranges.hpp>

#include "LevelCell.hpp"

std::string commaSeparatedList(const std::vector<int>& list) {
    std::string result;
    for (size_t i = 0; i < list.size(); ++i) {
        result += std::to_string(list[i]);
        if (i < list.size() - 1) result += ",";
    }
    return result;
}

bool LeaderboardBrowserLayer::init(GJSearchObject* object) {
    if (object->m_searchQuery == "SendDB") {
        object = GJSearchObject::create(SearchType::Type19, "");
        setFilter({10, 0, std::nullopt, std::nullopt});
    }
    return LevelBrowserLayer::init(object);
}

void LeaderboardBrowserLayer::setFilter(const LeaderboardQuery& filter) {
    const auto f = m_fields.self();
    f->enabled = true;
    f->currentFilter = filter;
    m_searchObject = GJSearchObject::create(SearchType::Type19, "0");
}

void LeaderboardBrowserLayer::loadPage(GJSearchObject* object) {
    const auto f = m_fields.self();
    if (!f->enabled) return LevelBrowserLayer::loadPage(object);

    if (f->ignore) return LevelBrowserLayer::loadPage(object);

    if (f->spoofLoad) {
        f->spoofLoad = false;
        f->ignore = true;
        LevelBrowserLayer::loadPage(object);
        f->ignore = false;
    } else {
        m_searchObject = object;
        customLoad();
    }
}

void LeaderboardBrowserLayer::setupLevelBrowser(CCArray* levels) {
    LevelBrowserLayer::setupLevelBrowser(levels);
    if (!m_list->m_listView) return;

    levels = updateResultArray(levels);

    std::vector<int> levelIDs;
    for (auto* levelObj : CCArrayExt<CCObject*>(levels)) {
        if (const auto level = typeinfo_cast<GJGameLevel*>(levelObj)) {
            levelIDs.push_back(level->m_levelID.value());
        }
    }

    m_fields->listeners = SendDBIntegration::get()->getLevels(levelIDs, [this](const std::vector<BatchLevel>& batchLevels) {
        m_fields->levels.clear();
        for (const auto& batchLevel : batchLevels) {
            m_fields->levels[batchLevel.levelID] = batchLevel;
        }
        refreshSendLabels();
    });
}

void LeaderboardBrowserLayer::setupPageInfo(const std::string info, const char* key) {
    const auto f = m_fields.self();
    if (!f->enabled) return LevelBrowserLayer::setupPageInfo(info, key);

    LevelBrowserLayer::setupPageInfo(fmt::format("{}:{}:{}", f->currentResponse.total, f->currentFilter.offset, f->currentFilter.limit), key);
}

void LeaderboardBrowserLayer::updatePageLabel() {
    const auto f = m_fields.self();
    if (!f->enabled) return LevelBrowserLayer::updatePageLabel();

    m_searchObject->m_page = f->currentFilter.offset / f->currentFilter.limit;
    LevelBrowserLayer::updatePageLabel();
}

bool LeaderboardBrowserLayer::isCorrect(const char* key) {
    const auto f = m_fields.self();
    if (!f->enabled) return LevelBrowserLayer::isCorrect(key);

    log::info("evil check: {}", key);
    log::info("expected: {}", m_searchObject->getKey());
    int originalPage = m_searchObject->m_page;
    m_searchObject->m_page = 0;
    bool result = LevelBrowserLayer::isCorrect(key);
    m_searchObject->m_page = originalPage;
    log::info("evil check result: {}", result);
    return result;
}

void LeaderboardBrowserLayer::customLoad() {
    const auto f = m_fields.self();

    if (m_allObjectsToggler) {
        m_allSelected = false;
        m_allObjectsToggler->toggle(false);
    }
    m_circle->setVisible(true);
    if (m_refreshBtn) m_refreshBtn->setVisible(false);
    if (m_cancelSearchBtn) m_cancelSearchBtn->setVisible(false);

    f->currentFilter.offset = std::max(0, m_searchObject->m_page) * f->currentFilter.limit;
    SendDBIntegration::get()->getLeaderboard(f->currentFilter, [this](const std::optional<LeaderboardResponse>& response) {
        if (response.has_value()) {
            const auto f = m_fields.self();

            f->currentResponse = response.value();
            const auto levelIDs = geode::utils::ranges::map<std::vector<int>>(f->currentResponse.levels, [](const LeaderboardLevel& c) {
                return c.levelID;
            });

            GJSearchObject* searchObject = GJSearchObject::create(SearchType::Type19, commaSeparatedList(levelIDs));
            f->spoofLoad = true;
            loadPage(searchObject);
            f->spoofLoad = false;
        }
    }, f->leaderboardListener);
}

void LeaderboardBrowserLayer::refreshSendLabels() {
    if (!m_list->m_listView) return;

    const auto cells = m_list->m_listView->m_tableView->m_contentLayer->getChildrenExt();
    for (const auto child : cells) {
        if (!typeinfo_cast<LevelCell*>(child)) continue;
        if (const auto cell = static_cast<SendDBLevelCell*>(child)) {
            int id = cell->m_level->m_levelID;
            auto it = m_fields->levels.find(id);

            if (it != m_fields->levels.end()) {
                cell->setLevelInfo(it->second);
            } else {
                cell->setLevelInfo(std::nullopt);
            }
        }
    }
}

LeaderboardBrowserLayer* LeaderboardBrowserLayer::create(const LeaderboardQuery& filter) {
    const auto newLayer = new LeaderboardBrowserLayer();
    newLayer->setFilter(filter);
    if (newLayer->init(GJSearchObject::create(SearchType::Type19, ""))) {
        newLayer->autorelease();
        return newLayer;
    }
    delete newLayer;
    return nullptr;
}