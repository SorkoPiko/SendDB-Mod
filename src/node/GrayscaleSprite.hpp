#pragma once

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>

using namespace geode::prelude;

class GrayscaleSprite : public CCNode {
    CCSprite* normalSprite = nullptr;
    CCSprite* grayscaleSprite = nullptr;

    bool init(const std::string& spriteName);
public:
    static GrayscaleSprite* create(const std::string& spriteName);

    void setGrayscale(bool grayscale);
};