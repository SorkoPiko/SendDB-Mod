#include "SentLevelNode.hpp"

#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>
#include <rock/RoundedRect.hpp>

bool SentLevelNode::init(const CCSize& size, const int _levelID, const int sendCount, const bool accurate) {
    if (!CCNode::init()) return false;
    setContentSize(size);

    levelID = _levelID;

    title = Build<CCLabelBMFont>::create(
        std::to_string(levelID).c_str(),
        "bigFont.fnt"
    )
            .scale(0.5f)
            .anchorPoint({0.0f, 1.0f})
            .pos({3.0f, size.height})
            .id("name-label")
            .parent(this);

    Build<CCLabelBMFont>::create(
        (std::to_string(sendCount) + (accurate ? "" : "?") + (sendCount == 1 ? " send" : " sends")).c_str(),
        "chatFont.fnt"
    )
            .scale(0.5f)
            .anchorPoint({0.0f, 1.0f})
            .pos({3.0f, size.height - title->getScaledContentHeight() - 2.0f})
            .id("send-label")
            .parent(this);

    menu = Build<CCMenu>::create()
            .pos({0.0f, 0.0f})
            .anchorPoint({0.0f, 0.0f})
            .contentSize(size)
            .id("menu")
            .parent(this);

    spinner = Build<LoadingSpinner>::create(20.0f)
            .anchorPoint({0.5f, 0.5f})
            .pos({size.width - 20.0f, size.height / 2.0f})
            .id("loading-spinner")
            .parent(menu);

    failedIcon = Build<CCSprite>::createSpriteName("edit_delBtnSmall_001.png")
            .scale(0.7f/0.52f)
            .anchorPoint({0.5f, 0.5f})
            .pos({size.width - 20.0f, size.height / 2.0f})
            .visible(false)
            .id("failed-icon")
            .parent(menu);

    const auto glm = GameLevelManager::get();
    if (const auto levelData = glm->getSavedLevel(levelID)) update(levelData);

    return true;
}

void SentLevelNode::update(GJGameLevel* level) {
    spinner->setVisible(false);

    if (!level) {
        failedIcon->setVisible(true);
        return;
    }
    failedIcon->setVisible(false);
    title->setString(level->m_levelName.c_str());

    if (playButton) playButton->removeFromParentAndCleanup(true);
    playButton = Build<CCSprite>::createSpriteName("GJ_playBtn2_001.png")
            .scale(0.25f)
            .intoMenuItem([level](auto*) {
                const auto levelInfo = CCScene::get()->getChildByType<LevelInfoLayer>(0);
                if (levelInfo && levelInfo->m_level && levelInfo->m_level->m_levelID.value() == level->m_levelID) {
                    return;
                }

                const auto scene = LevelInfoLayer::scene(level, false);
                CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5, scene));
            })
            .pos({getContentSize().width - 20.0f, getContentSize().height / 2.0f})
            .id("play-button")
            .parent(menu);
}

SentLevelNode* SentLevelNode::create(const CCSize& size, const int levelID, const int sendCount, const bool accurate) {
    auto node = new SentLevelNode();
    if (node->init(size, levelID, sendCount, accurate)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}
