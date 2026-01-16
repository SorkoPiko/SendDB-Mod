#include <camila314.geode-uri/include/GeodeURI.hpp>
#include <Geode/loader/ModEvent.hpp>

$on_mod(Loaded) {
    handleURI("senddb", [](std::string const& path) {

    });
}