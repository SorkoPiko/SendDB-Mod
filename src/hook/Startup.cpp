#include <Geode/loader/ModEvent.hpp>
#include <node/ShaderNode.hpp>
#include <utils/TimeUtils.hpp>

$on_mod(Loaded) {
    ShaderNode::firstTime = TimeUtils::getCurrentTimestamp();
}