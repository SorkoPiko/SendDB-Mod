#include <Geode/Geode.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/ProfilePage.hpp>

#include <UIBuilder.hpp>
#include <layer/CreatorInfoPopup.hpp>

#include <manager/SendDBIntegration.hpp>

using namespace geode::prelude;

class $modify(SendDBProfilePage, ProfilePage) {
    struct Fields {
        TaskHolder<web::WebResponse> listener;
        std::optional<Creator> creatorInfo;

        CCLabelBMFont* sendsLabel;
    };

    void loadPageFromUserInfo(GJUserScore* score){
        ProfilePage::loadPageFromUserInfo(score);
        createStats();

        SendDBIntegration::get()->getCreator(score->m_userID, [this](const std::optional<Creator>& creatorData) {
            if (creatorData.has_value()) {
                m_fields->creatorInfo = creatorData;
                placeButton();
                updateStats();
            }
        }, m_fields->listener);
    }

    void placeButton() {
        const auto menu = m_mainLayer->getChildByID("main-menu");

        Build<CCSprite>::create("logo-circle.png"_spr)
                .scale(0.125f)
                .intoMenuItem([this](auto*) {
                    const auto popup = CreatorInfoPopup::create(m_score, m_fields->creatorInfo);
                    popup->show();
                })
                .parent(menu)
                .pos({16.0f, -219.0f})
                .id("chart-button"_spr);
    }

    void createStats() {
        if (m_fields->sendsLabel) return;

        const std::optional<std::optional<LeaderboardCreator>> creatorOpt = SendDBIntegration::get()->getCache().getLeaderboardCreator(m_score->m_userID);
        if (!creatorOpt.has_value() || !creatorOpt->has_value()) return;

        const LeaderboardCreator& creator = creatorOpt->value();

        CCNode* menu = m_mainLayer->getChildByID("stats-menu");

        m_fields->sendsLabel = Build<CCLabelBMFont>::create(fmt::format("{}", creator.send_count).c_str(), "bigFont.fnt")
                .scale(0.6f)
                .anchorPoint({ 0.0f, 0.0f })
                .id("sends-label"_spr)
                .zOrder(2)
                .limitLabelWidth(60.0f, 0.6f, 0.0f);

        CCNode* textNode = Build<CCNode>::create()
                .anchorPoint({ 0.0f, 0.0f })
                .contentSize(CCPoint{ 0.6f, 0.6f } * m_fields->sendsLabel->getContentSize())
                .id("sends-text-node"_spr)
                .parent(menu);

        textNode->addChild(m_fields->sendsLabel);
        textNode->setLayoutOptions(AxisLayoutOptions::create()
            ->setScaleLimits(0.0f, 1.0f)
        );

        CCSprite* sprite = Build<CCSprite>::create("logo-circle.png"_spr)
                .scale(0.05f)
                .anchorPoint({ 0.5f, 0.5f })
                .id("sends-icon"_spr)
                .zOrder(2)
                .parent(menu);

        sprite->setLayoutOptions(AxisLayoutOptions::create()
            ->setScaleLimits(0.0f, 1.0f)
            ->setRelativeScale(0.08f)
            ->setNextGap(7.0f)
        );

        menu->updateLayout();
    }

    void updateStats() {
        if (!m_fields->sendsLabel) {
            createStats();
            return;
        }

        if (!m_fields->creatorInfo.has_value()) return;

        m_fields->sendsLabel->setString(fmt::format("{}", m_fields->creatorInfo->send_count).c_str());
        m_mainLayer->getChildByID("stats-menu")->updateLayout();
    }
};