#include <Geode/modify/GJScoreCell.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/Geode.hpp>
#include <manager/SendDBIntegration.hpp>
#include <model/APIResponse.hpp>

#include <UIBuilder.hpp>
#include <layer/CreatorInfoPopup.hpp>

class $modify(GJScoreCell) {
    void loadFromScore(GJUserScore* score) {
        GJScoreCell::loadFromScore(score);

        const std::optional<std::optional<LeaderboardCreator>> creatorOpt = SendDBIntegration::get()->getCache().getLeaderboardCreator(score->m_userID);
        if (!creatorOpt.has_value() || !creatorOpt->has_value()) return;

        const LeaderboardCreator& creator = creatorOpt->value();

        CCNode* menu = m_mainLayer->getChildByID("stats-menu");

        CCLabelBMFont* label = Build<CCLabelBMFont>::create(fmt::format("{}", creator.send_count).c_str(), "bigFont.fnt")
                .scale(0.6f)
                .anchorPoint({ 0.0f, 0.5f })
                .id("sends-label"_spr)
                .zOrder(2)
                .parent(menu)
                .limitLabelWidth(60.0f, 0.5f, 0.0f);

        label->setLayoutOptions(AxisLayoutOptions::create()
            ->setScaleLimits(0.1f, 1.0f)
            ->setRelativeScale(0.45f)
        );

        CCNode* sprite;

        const std::optional<std::optional<Creator>> creatorInfoOpt = SendDBIntegration::get()->getCache().getCreator(creator.playerID);
        if (creatorInfoOpt.has_value() && creatorInfoOpt->has_value()) {
            const Creator& creatorInfo = creatorInfoOpt->value();
            sprite = Build<CCSprite>::create("logo-small.png"_spr)
                    .intoMenuItem([creatorInfo, this](auto*) {
                        const auto popup = CreatorInfoPopup::create(m_score, creatorInfo);
                        popup->show();
                    })
                    .anchorPoint({ 0.5f, 0.5f })
                    .id("sends-icon"_spr)
                    .zOrder(2)
                    .parent(menu);
        } else {
            sprite = Build<CCSprite>::create("logo-small.png"_spr)
                    .anchorPoint({ 0.5f, 0.5f })
                    .id("sends-icon"_spr)
                    .zOrder(2)
                    .parent(menu);
        }

        sprite->setLayoutOptions(AxisLayoutOptions::create()
            ->setScaleLimits(0.1f, 1.0f)
            ->setRelativeScale(0.7f)
            ->setNextGap(7.0f)
        );

        menu->updateLayout();
    }
};