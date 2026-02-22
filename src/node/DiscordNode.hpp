#pragma once

#include <Geode/Prelude.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/particle_nodes/CCParticleSystemQuad.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

class DiscordNode : public CCNode {
    CCMenuItemSpriteExtra* button = nullptr;
    CCSprite* sprite = nullptr;
    CCParticleSystemQuad* particles = nullptr;

    bool init() override;
    ~DiscordNode() override;

public:
    static DiscordNode* create();
};