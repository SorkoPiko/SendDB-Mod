#include "BaseLayer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/cocos/CCDirector.h>

#include <UIBuilder.hpp>

void BaseLayer::initShaderBackground(const std::string& fragPath) {
    const auto shader = ShaderNode::create("generic.vsh", fragPath);
    if (!shader) return initBackground();

    backgroundShader = Build(shader)
        .zOrder(-10)
        .contentSize(getContentSize())
        .id("background-shader")
        .parent(this);
}

void BaseLayer::initBackground() {
    constexpr ccColor3B bgColor = {0, 102, 255};

    const auto winSize = CCDirector::get()->getWinSize();
    const auto bg = CCSprite::create("GJ_gradientBG.png");
    const auto bgSize = bg->getTextureRect().size;

    Build(bg)
            .anchorPoint(0.f, 0.f)
            .scaleX((winSize.width + 10.f) / bgSize.width)
            .scaleY((winSize.height + 10.f) / bgSize.height)
            .pos(-5.f, -5.f)
            .color(bgColor)
            .zOrder(-10)
            .id("background")
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
