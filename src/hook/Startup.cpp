#include <Geode/loader/ModEvent.hpp>
#include <node/ShaderNode.hpp>
#include <utils/TimeUtils.hpp>
#include <camila314.geode-uri/include/GeodeURI.hpp>
#include <layer/LeaderboardLayer.hpp>
#include <manager/SendDBIntegration.hpp>

$on_mod(Loaded) {
    ShaderNode::firstTime = TimeUtils::getCurrentTimestamp();
    handleURI("senddb", [](std::string const& path) {
        // soon 👀
    }).leak();

    listenForSettingChanges<int>("cacheTime", [](const int value) {
        SendDBIntegration::get()->getCache().setCacheDuration(value * 60);
    });

    listenForSettingChanges<int>("leaderboardPerPage", [](const int value) {
        LeaderboardLayer::query.limit = value;
        LeaderboardLayer::query.offset = 0;
    });
}