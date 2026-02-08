#pragma once

#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <model/APIResponse.hpp>
#include <rock/RoundedRect.hpp>

struct $modify(SendDBLevelCell, LevelCell) {
    struct Fields {
        CCNode* sendContainer = nullptr;
        CCLabelBMFont* sendLabel = nullptr;

        CCNode* rankContainer = nullptr;
        CCLabelBMFont* rankLabel = nullptr;
        CCNode* rankBg = nullptr;

        CCNode* trendingContainer = nullptr;
        CCLabelBMFont* trendingLabel = nullptr;
        CCNode* trendingBg = nullptr;

        std::optional<BatchLevel> levelInfo;
    };

    void setLevelInfo(const std::optional<BatchLevel>& info);
    void setRank(int rank);
    void setTrendingScore(double score);

    void createSendContainer();
    void createRankContainer();
    void createTrendingContainer();
};