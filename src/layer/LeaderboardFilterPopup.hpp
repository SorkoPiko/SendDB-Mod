#ifndef SENDDB_LEADERBOARDFILTERPOPUP_HPP
#define SENDDB_LEADERBOARDFILTERPOPUP_HPP

#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>

#include <model/APIRequest.hpp>
#include <node/GrayscaleSprite.hpp>
#include <rock/RoundedRect.hpp>

using namespace geode::prelude;

class LeaderboardFilterPopup : public FLAlertLayer {
    LeaderboardQuery originalQuery = {};
    LeaderboardQuery query = {};
    std::function<void(const LeaderboardQuery&)> callback;

    CCScale9Sprite* bg = nullptr;

    Build<GrayscaleSprite> rateFilterRated = nullptr;
    Build<GrayscaleSprite> rateFilterUnrated = nullptr;
    Build<GrayscaleSprite> gamemodeFilterClassic = nullptr;
    Build<GrayscaleSprite> gamemodeFilterPlatformer = nullptr;

    CCMenuItemSpriteExtra* rateFilterRatedButton = nullptr;
    CCMenuItemSpriteExtra* rateFilterUnratedButton = nullptr;
    CCMenuItemSpriteExtra* gamemodeFilterClassicButton = nullptr;
    CCMenuItemSpriteExtra* gamemodeFilterPlatformerButton = nullptr;

    Build<rock::RoundedRect> selectBg1 = nullptr;
    Build<rock::RoundedRect> selectBg2 = nullptr;

    bool init(
        const LeaderboardQuery& currentQuery,
        const std::function<void(const LeaderboardQuery&)>& queryCallback
    );

    void updateFilters();

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void keyBackClicked() override;

public:
    static LeaderboardFilterPopup* create(
        const LeaderboardQuery& currentQuery,
        const std::function<void(const LeaderboardQuery&)>& queryCallback
    );
};

#endif