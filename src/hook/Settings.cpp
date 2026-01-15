#include <Geode/loader/SettingV3.hpp>
#include <manager/SendDBIntegration.hpp>

using namespace geode::prelude;

$execute {
    listenForSettingChangesV3("cacheTime", [](const int value) {
        SendDBIntegration::get()->setCacheDuration(value * 60);
    });
}