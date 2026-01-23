#include <Geode/Geode.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>

#include <UIBuilder.hpp>
#include <layer/LeaderboardLayer.hpp>

using namespace geode::prelude;

class $modify(SendDBLevelSearchLayer, LevelSearchLayer) {
    bool init(const int searchType) {
        if (!LevelSearchLayer::init(searchType)) return false;

        if (const auto menu = getChildByID("other-filter-menu")) {
            Build<CCSprite>::create("logo-circle.png"_spr)
                    .scale(0.154f)
                    .intoMenuItem([](auto*) {
                        const auto scene = CCScene::create();
                        AppDelegate::get()->m_runningScene = scene;
                        const auto layer = LeaderboardLayer::create();
                        scene->addChild(layer);
                        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5, scene));
                    })
                    .parent(menu)
                    .id("leaderboard-button"_spr);

            menu->updateLayout();
        }

        return true;
    }
};