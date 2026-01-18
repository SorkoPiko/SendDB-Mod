#ifndef SENDDB_RANKINGNODE_HPP
#define SENDDB_RANKINGNODE_HPP

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>

#include <optional>

using namespace geode::prelude;

enum class RankingFilter {
    Rated,
    Unrated,
    Classic,
    Platformer,
    User
};

class RankingNode : public CCNode {
    CCSprite* filterSprite1 = nullptr;
    CCSprite* filterSprite2 = nullptr;
    CCLabelBMFont* label = nullptr;

    bool init(int ranking, const std::optional<int>& total, const std::optional<RankingFilter>& filter1, const std::optional<RankingFilter>& filter2);

public:
    static RankingNode* create(
        int ranking,
        const std::optional<int>& total = std::nullopt,
        const std::optional<RankingFilter>& filter1 = std::nullopt,
        const std::optional<RankingFilter>& filter2 = std::nullopt
    );
};

#endif