#include <Geode/modify/GJScoreCell.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/Geode.hpp>
#include <manager/SendDBIntegration.hpp>
#include <model/APIResponse.hpp>

#include <UIBuilder.hpp>
#include <rock/RoundedRect.hpp>
#include <utils/Style.hpp>

class $modify(GJScoreCell) {
    void loadFromScore(GJUserScore* score) {
        GJScoreCell::loadFromScore(score);

        const std::optional<std::optional<LeaderboardCreator>> creatorOpt = SendDBIntegration::get()->getCache().getLeaderboardCreator(score->m_userID);
        if (creatorOpt.has_value() && creatorOpt->has_value()) {
            const LeaderboardCreator& creator = creatorOpt->value();

            CCNode* sendContainer = Build<CCNode>::create()
                    .pos({ 62.0f, 29.0f })
                    .id("send-container"_spr)
                    .parent(this);

            CCLabelBMFont* sendLabel = Build<CCLabelBMFont>::create(fmt::format("{} sends", creator.send_count).c_str(), "chatFont.fnt")
                    .scale(0.6f)
                    .anchorPoint({ 0.0f, 0.5f })
                    .id("label")
                    .parent(sendContainer);

            Build(rock::RoundedRect::create(
                infoBoxColor,
                3.0f,
                sendLabel->getScaledContentSize() + CCSize{ 2.0f, 0.0f }
            ))
                    .pos({ -1.0f, 0.0f })
                    .anchorPoint({ 0.0f, 0.5f })
                    .parent(sendContainer)
                    .id("bg")
                    .zOrder(-1);
        }
    }
};