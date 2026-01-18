#include "RankingNode.hpp"

#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>
#include <rock/RoundedRect.hpp>

struct RankingFilterInfo {
    std::string frame;
    float scale;
};

RankingFilterInfo getRankingFilterInfo(const RankingFilter& filter) {
    switch (filter) {
        case RankingFilter::Rated:
            return {"GJ_hammerIcon_001.png", 1.0f/0.94f};
        case RankingFilter::Unrated:
            return {"diffIcon_00_btn_001.png", 1.0f/1.2f};
        case RankingFilter::Classic:
            return {"GJ_starsIcon_001.png", 1.0f/0.94f};
        case RankingFilter::Platformer:
            return {"GJ_moonsIcon_001.png", 1.0f/0.76f};
        case RankingFilter::User:
            return {"GJ_profileButton_001.png", 1.0f/2.14f};
    }
    return {"", 1.0f};
}

bool RankingNode::init(const int ranking, const std::optional<int>& total, const std::optional<RankingFilter>& filter1, const std::optional<RankingFilter>& filter2) {
    if (!CCNode::init()) return false;

    constexpr ccColor3B totalColor = {150, 150, 150};
    constexpr ccColor4B bgColor = {25, 25, 25, 220};

    Build(rock::RoundedRect::create(
        bgColor,
        3.0f,
        {110.0f, 30.0f}
    ))
            .posX(60.0f)
            .anchorPoint({0.0f, 0.0f})
            .zOrder(-1)
            .parent(this);

    if (filter1.has_value()) {
        const auto info = getRankingFilterInfo(filter1.value());
        Build<CCSprite>::createSpriteName(info.frame.c_str())
                .scale(info.scale)
                .pos({45.0f, 15.0f})
                .anchorPoint({0.5f, 0.5f})
                .parent(this);
    }

    if (filter2.has_value()) {
        const auto info = getRankingFilterInfo(filter2.value());
        Build<CCSprite>::createSpriteName(info.frame.c_str())
                .scale(info.scale)
                .pos({15.0f, 15.0f})
                .anchorPoint({0.5f, 0.5f})
                .parent(this);
    }

    label = Build<CCLabelBMFont>::create("#0", "bigFont.fnt")
            .posX(65.0f)
            .anchorPoint({0.0f, 0.0f})
            .parent(this);

    label->setString(fmt::format("#{}", ranking).c_str());
    if (total.has_value()) {
        Build<CCLabelBMFont>::create(("/" + std::to_string(total.value())).c_str(), "bigFont.fnt")
                .anchorPoint({0.0f, 0.0f})
                .color(totalColor)
                .matchPos(label)
                .move({label->getScaledContentWidth(), 0.0f})
                .parent(this);
    }

    return true;
}

RankingNode* RankingNode::create(const int ranking, const std::optional<int>& total, const std::optional<RankingFilter>& filter1, const std::optional<RankingFilter>& filter2) {
    auto node = new RankingNode();
    if (node->init(ranking, total, filter1, filter2)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}
