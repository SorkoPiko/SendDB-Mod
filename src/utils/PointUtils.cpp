#include "PointUtils.hpp"

#include <Geode/cocos/support/CCPointExtension.h>

bool PointUtils::isPointInsideNode(CCNode* node, const CCPoint& touch) {
    if (!node) return false;

    const CCPoint local = node->convertToNodeSpace(touch);
    const auto rect = CCRect(0, 0, node->getContentWidth(), node->getContentHeight());

    return rect.containsPoint(local);
}

float PointUtils::squaredDistanceFromNode(CCNode* node, const CCPoint& touch) {
    if (!node) return false;

    const CCPoint local = node->convertToNodeSpace(touch);
    const auto rect = CCRect(0, 0, node->getContentWidth(), node->getContentHeight());
    const CCPoint center = ccp(rect.getMidX(), rect.getMidY());

    return ccpDistanceSQ(local, center);
}