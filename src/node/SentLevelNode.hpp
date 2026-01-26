#ifndef SENDDB_SENTLEVELNODE_HPP
#define SENDDB_SENTLEVELNODE_HPP

#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/menu_nodes/CCMenu.h>
#include <Geode/ui/LoadingSpinner.hpp>

using namespace geode::prelude;

class SentLevelNode : public CCNode {
    int levelID = 0;

    CCLabelBMFont* title = nullptr;
    LoadingSpinner* spinner = nullptr;
    CCSprite* failedIcon = nullptr;
    CCMenuItemSpriteExtra* playButton = nullptr;
    CCMenu* menu = nullptr;

    bool init(const CCSize& size, int _levelID, int sendCount, bool accurate);

public:
    static SentLevelNode* create(const CCSize& size, int levelID, int sendCount, bool accurate);

    void update(GJGameLevel* level);
};

#endif