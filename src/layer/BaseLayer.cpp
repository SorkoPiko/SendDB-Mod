#include "BaseLayer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/cocos/CCDirector.h>

#include <UIBuilder.hpp>
#include <node/ShaderNode.hpp>

void BaseLayer::initShaderBackground(const std::string& fragPath) {
    auto shader = ShaderNode::createFromPath("", fragPath);
    if (!shader) {
        log::error("Failed to create background shader: {}", shader.unwrapErr());
        return;
    }

    Build(shader.unwrap())
        .zOrder(-10)
        .id("background-shader")
        .parent(this);
}

void BaseLayer::keyBackClicked() {
    CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
}

bool BaseLayer::init() {
    if (!CCLayer::init()) return false;

    setKeyboardEnabled(true);
    setKeypadEnabled(true);

    const auto winSize = CCDirector::get()->getWinSize();

    auto backButton = Build<CCSprite>::createSpriteName("GJ_arrow_01_001.png")
            .intoMenuItem([this](auto) {
                keyBackClicked();
            })
            .id("back-button");

    Build<CCMenu>::create()
        .contentSize(backButton->getScaledContentSize())
        .anchorPoint(0.0f, 1.0f)
        .pos(25.f, winSize.height - 25.f)
        .id("back-menu")
        .child(backButton)
        .parent(this);

    return true;
}
