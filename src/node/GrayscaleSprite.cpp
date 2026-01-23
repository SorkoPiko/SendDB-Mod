#include "GrayscaleSprite.hpp"

#include <Geode/binding/CCSpriteGrayscale.hpp>

bool GrayscaleSprite::init(const std::string& spriteName) {
    if (!CCNode::init()) return false;

    normalSprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
    grayscaleSprite = CCSpriteGrayscale::createWithSpriteFrameName(spriteName.c_str());
    grayscaleSprite->setVisible(false);

    normalSprite->setAnchorPoint({0.0f, 0.0f});
    grayscaleSprite->setAnchorPoint({0.0f, 0.0f});

    setContentSize(normalSprite->getContentSize());

    addChild(normalSprite);
    addChild(grayscaleSprite);

    return true;
}

GrayscaleSprite* GrayscaleSprite::create(const std::string& spriteName) {
    auto node = new GrayscaleSprite();
    if (node->init(spriteName)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

void GrayscaleSprite::setGrayscale(bool grayscale) {
    normalSprite->setVisible(!grayscale);
    grayscaleSprite->setVisible(grayscale);
}