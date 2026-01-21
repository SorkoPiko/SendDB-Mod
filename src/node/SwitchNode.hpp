#ifndef SENDDB_SWITCHNODE_HPP
#define SENDDB_SWITCHNODE_HPP

#include <Geode/cocos/base_nodes/CCNode.h>

using namespace geode::prelude;

class SwitchNode : public CCNode {
    std::vector<CCNode*> nodes;
    int currentIndex = -1;

    void updateVisibility() const;

public:
    static SwitchNode* create();

    int addNode(CCNode* node);
    void setActiveIndex(int index);
    int getActiveIndex() const;
};

#endif