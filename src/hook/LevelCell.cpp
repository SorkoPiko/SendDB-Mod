#include "LevelCell.hpp"

#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>
#include <utils/Style.hpp>

void SendDBLevelCell::setLevelInfo(const std::optional<BatchLevel>& info) {
    auto& fields = *m_fields.self();

    fields.levelInfo = info;
    if (fields.levelInfo.has_value()) {
        if (!fields.sendContainer) createSendContainer();

        const int sendCount = fields.levelInfo->send_count;
        const bool accurate = fields.levelInfo->accurate;
        fields.sendLabel->setString(fmt::format("{}{} {}", sendCount, accurate ? "" : "?", sendCount == 1 ? "send" : "sends").c_str());
        fields.sendContainer->setVisible(true);
    } else {
        if (fields.sendContainer) fields.sendContainer->setVisible(false);
    }
}

void SendDBLevelCell::setRank(const int rank) {
    auto& fields = *m_fields.self();

    if (rank <= 0) {
        if (fields.rankContainer) fields.rankContainer->setVisible(false);
        return;
    }

    if (!fields.rankContainer) createRankContainer();

    fields.rankLabel->setString(fmt::format("#{}", rank).c_str());
    fields.rankContainer->setVisible(true);
    fields.rankBg->setContentSize(fields.rankLabel->getScaledContentSize() + CCSize(2.0f, 0.0f));
}

void SendDBLevelCell::setTrendingScore(double score) {
    auto& fields = *m_fields.self();

    if (score <= 0.0) {
        if (fields.trendingContainer) fields.trendingContainer->setVisible(false);
        return;
    }

    if (!fields.trendingContainer) createTrendingContainer();

    fields.trendingLabel->setString(fmt::format("Score: {:.2f}", score).c_str());
    fields.trendingLabel->limitLabelWidth(100.0f, 0.5f, 0.0f);
    fields.trendingContainer->setVisible(true);
    fields.trendingBg->setContentSize(fields.trendingLabel->getScaledContentSize() + CCSize(2.0f, 0.0f));
}

void SendDBLevelCell::createSendContainer() {
    auto& fields = *m_fields.self();

    float xPos;
    float yPos;
    CCPoint anchor;

    if (std::abs(m_height - 90.0f) < 10.0f) {
        xPos = 314.0f;
        yPos = 27.0f;
        anchor = ccp(0.5f, 1.0f);
    } else {
        xPos = 319.0f;
        yPos = 0.0f;
        anchor = ccp(0.5f, 0.0f);

        if (const auto globedLabel = getChildByID("dankmeme.globed2/player-count-label")) {
            globedLabel->setPositionY(37.0f);
        }
        if (const auto globedIcon = getChildByID("dankmeme.globed2/player-count-icon")) {
            globedIcon->setPositionY(37.0f);
        }
    }

    fields.sendContainer = Build<CCNode>::create()
            .pos({xPos, yPos})
            .parent(this)
            .visible(false)
            .id("send-container"_spr);

    fields.sendLabel = Build<CCLabelBMFont>::create("50? sends", "chatFont.fnt")
            .scale(0.6f)
            .anchorPoint(anchor)
            .id("label")
            .parent(fields.sendContainer);

    Build(rock::RoundedRect::create(
        infoBoxColor,
        3.0f,
        fields.sendLabel->getScaledContentSize()
    ))
            .anchorPoint(anchor)
            .parent(fields.sendContainer)
            .id("bg")
            .zOrder(-1);
}

void SendDBLevelCell::createRankContainer() {
    auto& fields = *m_fields.self();

    fields.rankContainer = Build<CCNode>::create()
            .anchorPoint({0.5f, 0.5f})
            .pos({m_compactView ? 287.0f : 277.0f, m_height / 2.0f - (m_compactView ? 15.0f : 0.0f)})
            .parent(this)
            .visible(false)
            .id("rank-container"_spr);

    fields.rankLabel = Build<CCLabelBMFont>::create("#1", "goldFont.fnt")
            .scale(0.7f)
            .anchorPoint({1.0f, 0.4f})
            .id("label")
            .parent(fields.rankContainer);

    fields.rankBg = Build(rock::RoundedRect::create(
        infoBoxColor,
        3.0f,
        fields.rankLabel->getScaledContentSize()
    ))
            .anchorPoint({1.0f, 0.5f})
            .parent(fields.rankContainer)
            .id("bg")
            .zOrder(-1);
}

void SendDBLevelCell::createTrendingContainer() {
    auto& fields = *m_fields.self();

    fields.trendingContainer = Build<CCNode>::create()
            .anchorPoint({0.5f, 0.5f})
            .pos({314.0f, m_compactView ? 45.0f : 12.0f})
            .parent(this)
            .visible(false)
            .id("trending-container"_spr);

    fields.trendingLabel = Build<CCLabelBMFont>::create("Score: 0.00", "chatFont.fnt")
            .scale(0.6f)
            .anchorPoint({0.5f, 0.5f})
            .id("label")
            .parent(fields.trendingContainer);

    fields.trendingBg = Build(rock::RoundedRect::create(
        infoBoxColor,
        3.0f,
        fields.trendingLabel->getScaledContentSize()
    ))
            .anchorPoint({0.5f, 0.5f})
            .parent(fields.trendingContainer)
            .id("bg")
            .zOrder(-1);
}