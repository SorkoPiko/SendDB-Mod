#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include <UIBuilder.hpp>

#include <layer/LevelSendChartPopup.hpp>
#include <manager/SendDBIntegration.hpp>

using namespace geode::prelude;

class $modify(SendDBLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        EventListener<web::WebTask> listener;
        std::optional<Level> levelInfo;
    };

    bool init(GJGameLevel* level, const bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        SendDBIntegration::get()->getLevel(level->m_levelID.value(), [this](const std::optional<Level>& levelData) {
            if (levelData.has_value()) {
                m_fields->levelInfo = levelData;
                placeButton();
            }
        }, m_fields->listener);

        return true;
    }

    void placeButton() {
        const auto menu = getChildByID("other-menu");

        Build<CCSprite>::create("logo-circle.png"_spr)
                .scale(0.125f)
                .intoMenuItem(this, menu_selector(SendDBLevelInfoLayer::onChart))
                .parent(menu)
                .matchPos(menu->getChildByID("favorite-button"))
                .move({38.0f, 0.0f})
                .id("chart-button"_spr);
    }

    void onChart(CCObject* sender) {
        const auto chart = LevelSendChartPopup::create(m_level, m_level->m_levelID, m_fields->levelInfo);
        chart->show();
    }
};