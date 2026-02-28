#include <Geode/loader/ModEvent.hpp>
#include <node/ShaderNode.hpp>
#include <utils/TimeUtils.hpp>
#include <camila314.geode-uri/include/GeodeURI.hpp>

$on_mod(Loaded) {
    ShaderNode::firstTime = TimeUtils::getCurrentTimestamp();
    handleURI("senddb", [](std::string const& path) {
        // soon 👀
    }).leak();
}