#ifndef SENDDB_LEVELCELL_HPP
#define SENDDB_LEVELCELL_HPP

#include <Geode/Geode.hpp>
#include <Geode/modify/Modify.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <model/APIResponse.hpp>

struct $modify(SendDBLevelCell, LevelCell) {
    struct Fields {
        CCLabelBMFont* sendLabel = nullptr;
        std::optional<BatchLevel> levelInfo;
    };

    void setLevelInfo(const std::optional<BatchLevel>& info);
    void createSendLabel();
};

#endif