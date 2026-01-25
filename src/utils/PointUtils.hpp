#ifndef SENDDB_POINTUTILS_HPP
#define SENDDB_POINTUTILS_HPP

#include <Geode/Prelude.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>

using namespace geode::prelude;

class PointUtils {
public:
    static bool isPointInsideNode(CCNode* node, const CCPoint& touch);
    static float squaredDistanceFromNode(CCNode* node, const CCPoint& touch);
    static float nodeSDF(CCNode* node, const CCPoint& touch);
    static CCSize getNodeScreenSize(CCNode* node);
};

#endif