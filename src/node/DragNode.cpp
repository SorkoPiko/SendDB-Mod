#include "DragNode.hpp"

#include <Geode/ui/Layout.hpp>
#include <utils/PointUtils.hpp>

bool DragNode::init(std::function<void()> onPressCallback) {
    if (!CCLayer::init()) return false;

    setTouchEnabled(true);
    ignoreAnchorPointForPosition(false);
    scheduleUpdate();

    onPress = std::move(onPressCallback);
    return true;
}

bool DragNode::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!m_bVisible) return false;
    if (PointUtils::isPointInsideNode(this, touch->getLocation())) {
        const auto parent = getParent();
        const auto localPos = parent->convertToNodeSpace(touch->getLocation());

        dragOffset = getPosition() - localPos;
        return true;
    }
    return false;
}

void DragNode::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    const auto parent = getParent();
    const auto localPos = parent->convertToNodeSpace(touch->getLocation());

    const auto newPos = localPos + dragOffset;
    setPosition(newPos);
}

void DragNode::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    constexpr float thresholdSq = 3.0f * 3.0f;
    dragOffset = CCPointZero;
    if ((touch->getLocation() - touch->getStartLocation()).getLengthSq() > thresholdSq) return;
    if (onPress) onPress();
}

void DragNode::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
    dragOffset = CCPointZero;
}

void DragNode::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -512, true);
}

DragNode* DragNode::create(std::function<void()> onPressCallback) {
    auto ret = new DragNode();
    if (ret && ret->init(std::move(onPressCallback))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
