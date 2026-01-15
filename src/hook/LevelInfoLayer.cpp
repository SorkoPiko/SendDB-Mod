#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
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
        const auto menu = CCMenu::create();
        menu->setPosition({50.0f, 50.0f});
        menu->setContentSize({25.0f, 25.0f});
        menu->setScale(0.5f);
        menu->setID("chart-menu"_spr);
        addChild(menu);

        const auto button = CCMenuItemSpriteExtra::create(CCSprite::create("logo-circle.png"_spr), this, menu_selector(SendDBLevelInfoLayer::onChart));
        button->setPosition({0.0f, 0.0f});
        button->setID("chart-button"_spr);
        menu->addChild(button);
    }

    void onChart(CCObject* sender) {
        const auto chart = LevelSendChartPopup::create(m_level, m_level->m_levelID, m_fields->levelInfo);
        chart->show();
    }
};