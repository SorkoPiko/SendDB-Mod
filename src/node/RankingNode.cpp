#include "RankingNode.hpp"

#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>
#include <rock/RoundedRect.hpp>
#include <utils/Style.hpp>

Build<CCSprite> getRankingFilterInfo(const RankingFilter& filter) {
    switch (filter) {
        case RankingFilter::SendDB:
            return Build<CCSprite>::create("logo-small.png"_spr).scale(1.0f/1.07f);
        case RankingFilter::Rated:
            return Build<CCSprite>::createSpriteName("GJ_hammerIcon_001.png").scale(1.0f/0.94f);
        case RankingFilter::Unrated:
            return Build<CCSprite>::createSpriteName("diffIcon_00_btn_001.png").scale(1.0f/1.2f);
        case RankingFilter::Classic:
            return Build<CCSprite>::createSpriteName("GJ_starsIcon_001.png").scale(1.0f/0.94f);
        case RankingFilter::Platformer:
            return Build<CCSprite>::createSpriteName("GJ_moonsIcon_001.png").scale(1.0f/0.76f);
        case RankingFilter::User:
            return Build<CCSprite>::createSpriteName("GJ_profileButton_001.png").scale(1.0f/2.14f);
        case RankingFilter::Trending:
            return Build<CCSprite>::createSpriteName("GJ_sTrendingIcon_001.png").scale(1.0f/0.66f);
    }
    return Build<CCSprite>::create();
}

bool RankingNode::init(const int ranking, const std::string& descriptionText, const std::optional<int>& total, const std::optional<RankingFilter>& filter1, const std::optional<RankingFilter>& filter2) {
    if (!CCNode::init()) return false;

    setContentSize({140.0f, 43.0f});

    Build(rock::RoundedRect::create(
        infoBoxColor,
        3.0f,
        {108.0f, 30.0f}
    ))
            .posX(32.0f)
            .anchorPoint({0.0f, 0.0f})
            .zOrder(-1)
            .parent(this);

    if (filter1.has_value() && !filter2.has_value()) {
        filterSprite1 = getRankingFilterInfo(filter1.value())
                .pos({15.0f, 15.0f})
                .anchorPoint({0.5f, 0.5f})
                .parent(this);
    } else if (filter1.has_value() && filter2.has_value()) {
        filterSprite1 = getRankingFilterInfo(filter1.value())
                .pos({15.0f, 15.0f})
                .anchorPoint({0.5f, 0.5f})
                .parent(this);

        filterSprite2 = getRankingFilterInfo(filter2.value())
                .scaleBy(0.7f)
                .pos({22.0f, 8.0f})
                .anchorPoint({0.5f, 0.5f})
                .parent(this);
    }

    label = Build<CCLabelBMFont>::create("#0", "gjFont16.fnt")
            .scale(0.8f)
            .posX(35.0f)
            .anchorPoint({0.0f, 0.0f})
            .parent(this);

    label->setString(fmt::format("#{}", ranking).c_str());
    if (total.has_value()) {
        Build<CCLabelBMFont>::create(fmt::format("/{}", total.value()).c_str(), "gjFont16.fnt")
                .scale(0.8f)
                .anchorPoint({0.0f, 0.0f})
                .color(secondaryColor)
                .matchPos(label)
                .move({label->getScaledContentWidth(), 0.0f})
                .parent(this);
    }

    description = Build<CCLabelBMFont>::create(descriptionText.c_str(), "chatFont.fnt")
            .anchorPoint({0.0f, 0.0f})
            .pos({0.0f, 32.0f})
            .color(secondaryTextColor)
            .scale(0.6f)
            .parent(this);

    return true;
}

RankingNode* RankingNode::create(const int ranking, const std::string& description, const std::optional<int>& total, const std::optional<RankingFilter>& filter1, const std::optional<RankingFilter>& filter2) {
    auto node = new RankingNode();
    if (node->init(ranking, description, total, filter1, filter2)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}
