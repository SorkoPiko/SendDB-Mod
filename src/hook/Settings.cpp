#include <Geode/loader/SettingV3.hpp>
#include <layer/LeaderboardLayer.hpp>
#include <manager/SendDBIntegration.hpp>

using namespace geode::prelude;

$execute {
    listenForSettingChanges<int>("cacheTime", [](const int value) {
        SendDBIntegration::get()->getCache().setCacheDuration(value * 60);
    });

    listenForSettingChanges<int>("leaderboardPerPage", [](const int value) {
        LeaderboardLayer::query.limit = value;
        LeaderboardLayer::query.offset = 0;
    });
}