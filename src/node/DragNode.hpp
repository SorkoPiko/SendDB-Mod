#ifndef SENDDB_DRAGNODE_HPP
#define SENDDB_DRAGNODE_HPP

#include <Geode/Prelude.hpp>
#include <Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h>

using namespace geode::prelude;

class DragNode : public CCLayer {
    int touchPriority = 0;
    std::function<void()> onPress;
    CCPoint dragOffset = CCPoint{0, 0};
    float dragDistance = 0.0f;
    CCSize originalScale = {};
    bool scaleOnDrag = false;

    bool init(int initTouchPriority, std::function<void()> onPressCallback);

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override;

    void registerWithTouchDispatcher() override;

public:
    static DragNode* create(int initTouchPriority = 0, std::function<void()> onPressCallback = nullptr);
};

#endif