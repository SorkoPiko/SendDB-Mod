#include <Geode/modify/LeaderboardsLayer.hpp>
#include <Geode/Geode.hpp>
#include <UIBuilder.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <manager/SendDBIntegration.hpp>
#include <node/DiscordNode.hpp>

using namespace geode::prelude;

bool customData = false;

class $modify(LeaderboardsLayer) {
    struct Fields {
        TaskHolder<web::WebResponse> creatorListener;
        CCMenuItemSpriteExtra* button = nullptr;

        DiscordNode* discordButton = nullptr;
        CCMenuItemSpriteExtra* websiteButton = nullptr;
    };

    bool init(const LeaderboardType type, const LeaderboardStat stat) {
        if (!LeaderboardsLayer::init(type, stat)) return false;

        CCNode* menu = getChildByID("right-side-menu");

        m_fields->button = Build<ButtonSprite>::create(CCSprite::create("logo-small.png"_spr), 32, 0, 32.0f, 1.0f, true, customData ? "GJ_button_02.png" : "GJ_button_01.png", false)
                .with([](ButtonSprite* sprite) {
                    sprite->updateSpriteOffset({ 0.0f, -1.5f });
                })
                .scale(0.6f)
                .intoMenuItem([this] {
                    customData = !customData;

                    const auto button = static_cast<ButtonSprite*>(m_fields->button->getNormalImage());
                    button->m_BGSprite->removeMeAndCleanup();
                    button->m_BGSprite = Build<CCScale9Sprite>::create(customData ? "GJ_button_02.png" : "GJ_button_01.png", CCRect{ CCPointZero, { 40.0f, 40.0f } })
                            .contentSize({ 16.0f, 16.0f })
                            .parent(button);
                    button->updateSpriteBGSize();

                    m_fields->discordButton->setVisible(customData && m_type == LeaderboardType::Creator);
                    m_fields->websiteButton->setVisible(customData && m_type == LeaderboardType::Creator);
                    getChildByID("bottom-right-menu")->updateLayout();

                    if (customData) getCreatorLeaderboard();
                    else {
                        m_type = LeaderboardType::Default;
                        selectLeaderboard(LeaderboardType::Creator, m_stat);
                    }
                })
                .id("senddb-button"_spr)
                .visible(type == LeaderboardType::Creator)
                .parent(menu);

        menu->updateLayout();

        if (type == LeaderboardType::Creator && customData) {
            getCreatorLeaderboard();
        }

        CCNode* rightMenu = getChildByID("bottom-right-menu");

        m_fields->websiteButton = Build<CCSprite>::createSpriteName("geode.loader/homepage.png")
                .intoMenuItem([](auto*) {
                    web::openLinkInBrowser("https://senddb.dev/creators");
                })
                .id("website-button"_spr)
                .visible(customData && type == LeaderboardType::Creator)
                .zOrder(-3)
                .parent(rightMenu);

        m_fields->discordButton = Build<DiscordNode>::create()
                .id("discord-button"_spr)
                .visible(customData && type == LeaderboardType::Creator)
                .zOrder(-3)
                .parent(rightMenu);

        rightMenu->updateLayout();

        return true;
    }

    void selectLeaderboard(const LeaderboardType type, const LeaderboardStat stat) {
        LeaderboardsLayer::selectLeaderboard(type, stat);
        if (type == LeaderboardType::Creator && customData) {
            getCreatorLeaderboard();
        }
        if (m_fields->button) {
            m_fields->button->setVisible(type == LeaderboardType::Creator);
            getChildByID("right-side-menu")->updateLayout();
        }
    }

    void getCreatorLeaderboard() {
        setupLevelBrowser(nullptr);

        m_circle->show();
        SendDBIntegration::get()->getCreatorLeaderboard({ 50, 0 }, [this](const std::optional<CreatorLeaderboardResponse>& response) {
            m_circle->fadeAndRemove();
            if (!response.has_value()) return;

            GameLevelManager* glm = GameLevelManager::get();

            CCArray* scores = CCArray::create();
            for (const auto& entry : response->creators) {
                GJUserScore* score = glm->userInfoForAccountID(entry.accountID);
                if (!score) {
                    score = new GJUserScore();
                    score->autorelease();
                    score->m_userID = entry.playerID;
                    score->m_accountID = entry.accountID;
                    score->m_userName = entry.name;
                    score->m_scoreType = static_cast<int>(GJScoreType::Creator);
                }
                score->m_iconID = score->m_playerCube;
                score->m_playerRank = entry.rank;

                scores->addObject(score);
            }
            setupLevelBrowser(scores);
        }, m_fields->creatorListener);
    }
};