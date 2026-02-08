#pragma once

#include <Geode/Prelude.hpp>

using namespace geode::prelude;

class FadeSpinner : public CCSprite {
    bool init() override {
        if (!initWithFile("loadingCircle.png")) return false;

        setBlendFunc({GL_SRC_ALPHA, GL_ONE });
        setVisible(false);
        setOpacity(0);
        spin();

        return true;
    }

    void spin() {
        runAction(CCRepeatForever::create(CCRotateBy::create(1.f, 360.f)));
    }

public:
    static FadeSpinner* create() {
        auto node = new FadeSpinner();
        if (node->init()) {
            node->autorelease();
            return node;
        }
        CC_SAFE_DELETE(node);
        return nullptr;
    }

    void fadeIn(const float duration) {
        stopActionByTag(0);

        setVisible(true);
        setOpacity(0);
        const auto action = CCFadeIn::create(duration);
        action->setTag(0);
        runAction(action);
    }

    void fadeOut(const float duration) {
        stopActionByTag(0);

        const auto action = CCSequence::create(
            CCFadeOut::create(duration),
            CCCallFunc::create(this, callfunc_selector(FadeSpinner::deactivate)),
            nullptr
        );
        action->setTag(0);
        runAction(action);
    }

    void deactivate() {
        setVisible(false);
    }
};