#include "PointUtils.hpp"

#include <Geode/cocos/support/CCPointExtension.h>

bool PointUtils::isPointInsideNode(CCNode* node, const CCPoint& touch) {
    if (!node) return false;

    const CCPoint local = node->convertToNodeSpace(touch);
    const auto rect = CCRect(0, 0, node->getContentWidth(), node->getContentHeight());

    return rect.containsPoint(local);
}

float PointUtils::squaredDistanceFromNode(CCNode* node, const CCPoint& touch) {
    if (!node) return std::numeric_limits<float>::max();

    const auto nodeCenter = ccp(
        node->getContentWidth() / 2.0f,
        node->getContentHeight() / 2.0f
    );

    const CCPoint worldCenter = node->convertToWorldSpace(nodeCenter);

    return ccpDistanceSQ(touch, worldCenter);
}

float PointUtils::nodeSDF(CCNode* node, const CCPoint& touch) {
    if (!node) return std::numeric_limits<float>::max();

    const auto nodeCenter = ccp(
        node->getContentWidth() / 2.0f,
        node->getContentHeight() / 2.0f
    );
    const CCPoint worldCenter = node->convertToWorldSpace(nodeCenter);

    const float halfWidth = node->getScaledContentWidth() / 2.0f;
    const float halfHeight = node->getScaledContentHeight() / 2.0f;

    const auto offset = ccp(
        touch.x - worldCenter.x,
        touch.y - worldCenter.y
    );

    const auto distance = ccp(
        std::abs(offset.x) - halfWidth,
        std::abs(offset.y) - halfHeight
    );

    const float outsideDist = ccpLength(ccp(std::max(distance.x, 0.0f), std::max(distance.y, 0.0f)));
    const float insideDist = std::min(std::max(distance.x, distance.y), 0.0f);

    return outsideDist + insideDist;
}

CCSize PointUtils::getNodeScreenSize(CCNode* node) {
    if (!node) return CCSizeZero;

    const auto min = node->convertToWorldSpace(CCPointZero);
    const auto max = node->convertToWorldSpace(ccp(node->getContentWidth(), node->getContentHeight()));

    return CCSize(
        std::abs(max.x - min.x),
        std::abs(max.y - min.y)
    );
}