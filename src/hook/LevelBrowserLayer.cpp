#include <Geode/binding/BoomListView.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/TableView.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <manager/SendDBIntegration.hpp>

#include "LevelCell.hpp"

using namespace geode::prelude;

class $modify(SendDBLevelBrowserLayer, LevelBrowserLayer) {
    struct Fields {
        EventListener<web::WebTask> listener;
        std::unordered_map<int, BatchLevel> levels;

        std::vector<EventListener<web::WebTask>> listeners;
    };

    void setupLevelBrowser(CCArray* levels) {
        LevelBrowserLayer::setupLevelBrowser(levels);
        if (!m_list->m_listView) return;

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

    void refreshSendLabels() {
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
};