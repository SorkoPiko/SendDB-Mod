#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include <UIBuilder.hpp>

#include <layer/LevelSendChartPopup.hpp>
#include <manager/SendDBIntegration.hpp>
#include <utils/MultiCallback.hpp>

using namespace geode::prelude;

class $modify(LevelInfoLayer) {
    struct Fields {
        TaskHolder<web::WebResponse> levelListener;
        std::optional<Level> levelInfo;
        TaskHolder<web::WebResponse> creatorListener;
        std::optional<Creator> creator;
        std::shared_ptr<MultiCallback> callback;
    };

    bool init(GJGameLevel* level, const bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        m_fields->callback = std::make_shared<MultiCallback>([this] {
            placeButton();
        }, 2);

        SendDBIntegration::get()->getLevel(level->m_levelID.value(), [this](const std::optional<Level>& levelData) {
            if (levelData.has_value()) {
                m_fields->levelInfo = levelData;
                (*m_fields->callback)(1);
            }
        }, m_fields->levelListener);

        SendDBIntegration::get()->getCreator(level->m_userID.value(), [this](const std::optional<Creator>& creatorData) {
            if (creatorData.has_value()) {
                m_fields->creator = creatorData;
                (*m_fields->callback)(2);
            }
        }, m_fields->creatorListener);

        return true;
    }

    void placeButton() {
        const auto menu = getChildByID("other-menu");

        Build<CCSprite>::create("logo-medium.png"_spr)
                .scale(0.245f)
                .intoMenuItem([this](auto*) {
                    const auto chart = LevelSendChartPopup::create(m_level, m_level->m_levelID, m_fields->levelInfo, m_fields->creator);
                    chart->show();
                })
                .parent(menu)
                .matchPos(menu->getChildByID("favorite-button"))
                .move({38.0f, 0.0f})
                .id("chart-button"_spr);
    }
};