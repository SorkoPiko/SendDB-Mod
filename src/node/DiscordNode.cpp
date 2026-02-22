#include "DiscordNode.hpp"

#include <Geode/Geode.hpp>
#include <UIBuilder.hpp>
#include <manager/AudioManager.hpp>

bool DiscordNode::init() {
    if (!CCNode::init()) return false;
    AudioManager::get().registerSource(this);

    setContentSize({ 35.0f, 35.0f });
    setAnchorPoint({ 0.5f, 0.5f });

    ParticleStruct particleInfo;
    particleInfo.TotalParticles = 30;
    particleInfo.Duration = -1.0f;
    particleInfo.Life = 1.5f;
    particleInfo.LifeVar = 0.5f;
    particleInfo.EmissionRate = particleInfo.TotalParticles / particleInfo.Life;
    particleInfo.AngleVar = 180;
    particleInfo.Speed = 50;
    particleInfo.SpeedVar = 5;
    particleInfo.PosVarX = 10;
    particleInfo.PosVarY = 10;
    particleInfo.RotatePerSecond = 40;
    particleInfo.RotatePerSecondVar = 10;
    particleInfo.StartSize = 5;
    particleInfo.StartSpin = 270;
    particleInfo.StartSizeVar = 2;
    particleInfo.StartSpinVar = 20;
    particleInfo.StartColorR = 1.0f;
    particleInfo.StartColorG = 1.0f;
    particleInfo.StartColorB = 1.0f;
    particleInfo.StartColorA = 1.0f;
    particleInfo.EndSize = 2;
    particleInfo.EndSpin = 270;
    particleInfo.EndSpinVar = 20;
    particleInfo.EndColorR = 1.0f;
    particleInfo.EndColorG = 1.0f;
    particleInfo.EndColorB = 1.0f;
    particleInfo.EndColorA = 0.0f;
    particleInfo.StartRadius = 25;
    particleInfo.EndRadius = 5;
    particleInfo.FadeInTime = 0.5f;
    particleInfo.EmitterMode = 2; // ??
    particleInfo.PositionType = kCCPositionTypeRelative;
    particleInfo.sFrame = "GJ_likesIcon_001.png";
    particleInfo.frictionSize = 5.0f;

    bool animations = !Mod::get()->getSettingValue<bool>("removeAnimations");

    particles = Build(GameToolbox::particleFromStruct(particleInfo, CCParticleSystemQuad::create(), false))
            .pos({ 17.5f, 17.5f })
            .id("particles")
            .zOrder(-1)
            .visible(animations)
            .parent(this);

    particles->setTotalParticles(20);

    CCMenu* menu = Build<CCMenu>::create()
            .pos({ 17.5f, 17.5f })
            .contentSize({ 0.0f, 0.0f })
            .id("menu")
            .parent(this);

    sprite = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");

    button = Build(sprite).intoMenuItem([](auto*) {
                web::openLinkInBrowser(Mod::get()->getMetadata().getLinks().getCommunityURL().value_or("https://discord.senddb.dev"));
            })
            .id("button")
            .parent(menu);

    if (animations) {
        sprite->runAction(CCRepeatForever::create(
            CCSequence::create(
                CCEaseInOut::create(CCRotateTo::create(1.5f, 2), 2.0f),
                CCEaseInOut::create(CCRotateTo::create(1.5f, -2), 2.0f),
                nullptr
            )
        ));
    }

    return true;
}

DiscordNode::~DiscordNode() {
    AudioManager::get().unregisterSource(this);
}

DiscordNode* DiscordNode::create() {
    auto ret = new DiscordNode();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
