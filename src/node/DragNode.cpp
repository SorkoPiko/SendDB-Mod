#include "DragNode.hpp"

#include <Geode/loader/Log.hpp>
#include <Geode/ui/Layout.hpp>
#include <utils/PointUtils.hpp>

bool DragNode::init(const int initTouchPriority, std::function<void()> onPressCallback) {
    if (!CCLayer::init()) return false;
    setAnchorPoint({0.5f, 0.5f});
    setTouchEnabled(true);
    ignoreAnchorPointForPosition(false);
    scheduleUpdate();

    touchPriority = initTouchPriority;
    onPress = std::move(onPressCallback);
    return true;
}

bool DragNode::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!m_bVisible) return false;
    if (PointUtils::isPointInsideNode(this, touch->getLocation())) {
        const auto parent = getParent();
        const auto localPos = parent->convertToNodeSpace(touch->getLocation());
        dragOffset = getPosition() - localPos;
        dragDistance = dragOffset.getLength();

        originalScale = CCSize{getScaleX(), getScaleY()};
        const float sdf = PointUtils::nodeSDF(this, touch->getLocation());
        scaleOnDrag = sdf > -10.0f;

        return true;
    }
    return false;
}

void DragNode::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    const auto parent = getParent();
    const auto localPos = parent->convertToNodeSpace(touch->getLocation());

    if (scaleOnDrag) {
        const float currentDistance = (getPosition() - localPos).getLength();
        const float scaleFactor = currentDistance / dragDistance;
        const auto newScale = originalScale * scaleFactor;
        setScale(newScale.width, newScale.height);
    } else {
        const auto newPos = localPos + dragOffset;
        setPosition(newPos);
    }
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
    CCTouchDispatcher::get()->addTargetedDelegate(this, touchPriority, true);
}

DragNode* DragNode::create(const int initTouchPriority, std::function<void()> onPressCallback) {
    auto ret = new DragNode();
    if (ret && ret->init(initTouchPriority, std::move(onPressCallback))) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
