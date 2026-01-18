#include <Geode/Geode.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/ProfilePage.hpp>

#include <UIBuilder.hpp>

#include <manager/SendDBIntegration.hpp>

using namespace geode::prelude;

class $modify(SendDBProfilePage, ProfilePage) {
    struct Fields {
        EventListener<web::WebTask> listener;
        std::optional<Creator> creatorInfo;
    };

    void loadPageFromUserInfo(GJUserScore* score){
        ProfilePage::loadPageFromUserInfo(score);

        SendDBIntegration::get()->getCreator(score->m_userID, [this](const std::optional<Creator>& creatorData) {
            if (creatorData.has_value()) {
                m_fields->creatorInfo = creatorData;
                placeButton();
            }
        }, m_fields->listener);
    }

    void placeButton() {
        const auto menu = m_mainLayer->getChildByID("main-menu");

        Build<CCSprite>::create("logo-circle.png"_spr)
                .scale(0.125f)
                .intoMenuItem(this, menu_selector(SendDBProfilePage::onChart))
                .parent(menu)
                .pos({16.0f, -189.0f})
                .id("chart-button"_spr);
    }

    void onChart(CCObject* sender) {}
};