#include "SwitchNode.hpp"

void SwitchNode::updateVisibility() const {
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i]->setVisible(i == currentIndex);
    }
}

SwitchNode* SwitchNode::create() {
    auto ret = new SwitchNode();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

int SwitchNode::addNode(CCNode* node) {
    nodes.push_back(node);
    this->addChild(node);
    updateVisibility();
    return static_cast<int>(nodes.size() - 1);
}

void SwitchNode::setActiveIndex(const int index) {
    if (index < -1 || index >= static_cast<int>(nodes.size())) {
        return;
    }
    currentIndex = index;
    updateVisibility();
}

int SwitchNode::getActiveIndex() const {
    return currentIndex;
}