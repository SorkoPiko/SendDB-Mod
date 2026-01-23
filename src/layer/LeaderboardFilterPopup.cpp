#include "LeaderboardFilterPopup.hpp"

#include <utils/PointUtils.hpp>
#include <utils/Style.hpp>

constexpr CCPoint popupSize = {130.0f, 120.0f};
constexpr CCPoint menuSize = {120.0f, 110.0f};

bool LeaderboardFilterPopup::init(const LeaderboardQuery& currentQuery, const std::function<void(const LeaderboardQuery&)>& queryCallback) {
    if (!FLAlertLayer::init(75)) return false;
    query = currentQuery;
    callback = queryCallback;

    const CCSize winSize = CCDirector::sharedDirector()->getWinSize();

    bg = Build<CCScale9Sprite>::create("GJ_square01.png")
            .contentSize(popupSize)
            .pos({winSize.width / 2, winSize.height / 2})
            .id("bg")
            .parent(m_mainLayer)
            .zOrder(-1);

    m_buttonMenu = Build<CCMenu>::create()
            .contentSize(menuSize)
            .pos({winSize.width / 2 - menuSize.x / 2, winSize.height / 2 - menuSize.y / 2})
            .id("menu")
            .zOrder(10)
            .parent(m_mainLayer);

    Build(CircleButtonSprite::createWithSpriteFrameName("geode.loader/close.png", 0.85f, CircleBaseColor::Gray))
        .scale(0.65f)
        .intoMenuItem([this](auto) {keyBackClicked();})
        .pos({-5.0f, menuSize.y + 5.0f})
        .id("close-button")
        .parent(m_buttonMenu);

    Build<CCLabelBMFont>::create("Filters", "bigFont.fnt")
            .anchorPoint({0.5f, 1.0f})
            .pos({menuSize.x / 2.0f, menuSize.y})
            .scale(0.85f)
            .id("title-label")
            .parent(m_buttonMenu);

    rateFilterRated = Build(GrayscaleSprite::create("GJ_hammerIcon_001.png"))
            .scale(1.0f/0.94f);

    rateFilterUnrated = Build(GrayscaleSprite::create("diffIcon_00_btn_001.png"))
            .scale(1.0f/1.2f);

    gamemodeFilterClassic = Build(GrayscaleSprite::create("GJ_starsIcon_001.png"))
            .scale(1.0f/0.94f);

    gamemodeFilterPlatformer = Build(GrayscaleSprite::create("GJ_moonsIcon_001.png"))
            .scale(1.0f/0.76f);

    rateFilterRatedButton = rateFilterRated.intoMenuItem([this](auto) {
        if (query.rateFilter != RateFilter::Rated) query.rateFilter = RateFilter::Rated;
        else query.rateFilter = std::nullopt;
        updateFilters();
    })
            .pos(menuSize / 2 + ccp(-30.0f, 3.0f))
            .parent(m_buttonMenu);

    rateFilterUnratedButton = rateFilterUnrated.intoMenuItem([this](auto) {
        if (query.rateFilter != RateFilter::Unrated) query.rateFilter = RateFilter::Unrated;
        else query.rateFilter = std::nullopt;
        updateFilters();
    })
            .pos(menuSize / 2 + ccp(-30.0f, -32.0f))
            .parent(m_buttonMenu);

    gamemodeFilterClassicButton = gamemodeFilterClassic.intoMenuItem([this](auto) {
        if (query.gamemodeFilter != GamemodeFilter::Classic) query.gamemodeFilter = GamemodeFilter::Classic;
        else query.gamemodeFilter = std::nullopt;
        updateFilters();
    })
            .pos(menuSize / 2 + ccp(30.0f, 3.0f))
            .parent(m_buttonMenu);

    gamemodeFilterPlatformerButton = gamemodeFilterPlatformer.intoMenuItem([this](auto) {
        if (query.gamemodeFilter != GamemodeFilter::Platformer) query.gamemodeFilter = GamemodeFilter::Platformer;
        else query.gamemodeFilter = std::nullopt;
        updateFilters();
    })
            .pos(menuSize / 2 + ccp(30.0f, -32.0f))
            .parent(m_buttonMenu);

    selectBg1 = Build(rock::RoundedRect::create(
        infoBoxColor,
        3.0f,
        {30.0f, 30.0f}
    ))
            .anchorPoint({0.5f, 0.5f})
            .visible(false)
            .zOrder(-1)
            .parent(m_buttonMenu);

    selectBg2 = Build(rock::RoundedRect::create(
        infoBoxColor,
        3.0f,
        {30.0f, 30.0f}
    ))
            .anchorPoint({0.5f, 0.5f})
            .visible(false)
            .zOrder(-1)
            .parent(m_buttonMenu);

    updateFilters();

    return true;
}

void LeaderboardFilterPopup::updateFilters() {
    rateFilterRated->setGrayscale(query.rateFilter != RateFilter::Rated);
    rateFilterUnrated->setGrayscale(query.rateFilter != RateFilter::Unrated);
    CCMenuItemSpriteExtra* selected = query.rateFilter.has_value() ? (query.rateFilter == RateFilter::Rated ? rateFilterRatedButton : rateFilterUnratedButton) : nullptr;
    selectBg1->setVisible(selected != nullptr);
    if (selected) selectBg1->setPosition(selected->getPosition());

    gamemodeFilterClassic->setGrayscale(query.gamemodeFilter != GamemodeFilter::Classic);
    gamemodeFilterPlatformer->setGrayscale(query.gamemodeFilter != GamemodeFilter::Platformer);
    selected = query.gamemodeFilter.has_value() ? (query.gamemodeFilter == GamemodeFilter::Classic ? gamemodeFilterClassicButton : gamemodeFilterPlatformerButton) : nullptr;
    selectBg2->setVisible(selected != nullptr);
    if (selected) selectBg2->setPosition(selected->getPosition());
}

bool LeaderboardFilterPopup::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!FLAlertLayer::ccTouchBegan(touch, event)) return false;

    const CCPoint touchLocation = touch->getLocation();
    if (!PointUtils::isPointInsideNode(bg, touchLocation)) keyBackClicked();

    return true;
}

void LeaderboardFilterPopup::keyBackClicked() {
    if (callback) callback(query);
    FLAlertLayer::keyBackClicked();
}

LeaderboardFilterPopup* LeaderboardFilterPopup::create(const LeaderboardQuery& currentQuery, const std::function<void(const LeaderboardQuery&)>& queryCallback) {
    if (const auto newLayer = new LeaderboardFilterPopup(); newLayer->init(currentQuery, queryCallback)) {
        newLayer->autorelease();
        return newLayer;
    } else {
        delete newLayer;
        return nullptr;
    }
}
