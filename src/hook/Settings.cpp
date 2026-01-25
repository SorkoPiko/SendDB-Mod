#include <Geode/loader/SettingV3.hpp>
#include <layer/LeaderboardLayer.hpp>
#include <manager/SendDBIntegration.hpp>

using namespace geode::prelude;

$execute {
    listenForSettingChangesV3("cacheTime", [](const int value) {
        SendDBIntegration::get()->setCacheDuration(value * 60);
    });

    listenForSettingChangesV3("leaderboardPerPage", [](const int value) {
        LeaderboardLayer::query.limit = value;
        LeaderboardLayer::query.offset = 0;
    });
}