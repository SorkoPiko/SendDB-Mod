#include "LevelCell.hpp"

#include <UIBuilder.hpp>

void SendDBLevelCell::setLevelInfo(const std::optional<BatchLevel>& info) {
    auto& fields = *m_fields.self();

    fields.levelInfo = info;
    if (fields.levelInfo.has_value()) {
        if (!fields.sendLabel) createSendLabel();

        const int sendCount = fields.levelInfo->send_count;
        const bool accurate = fields.levelInfo->accurate;
        fields.sendLabel->setString((std::to_string(sendCount) + (accurate ? "" : "?") + (sendCount == 1 ? " send" : " sends")).c_str());
        fields.sendLabel->setVisible(true);
    } else {
        if (fields.sendLabel) fields.sendLabel->setVisible(false);
    }
}

void SendDBLevelCell::createSendLabel() {
    auto& fields = *m_fields.self();

    float xPos = 0.0f;
    float yPos = 0.0f;
    CCPoint anchor;

    if (std::abs(m_height - 90.0f) < 10.0f) {
        xPos = 314.0f;
        yPos = 30.0f;
        anchor = ccp(0.5f, 1.0f);
    } else {
        xPos = m_width - 6.0f;
        yPos = 0.0f;
        anchor = ccp(1.0f, 0.0f);

        if (const auto globedLabel = getChildByID("dankmeme.globed2/player-count-label")) {
            globedLabel->setPositionY(37.0f);
        }
        if (const auto globedIcon = getChildByID("dankmeme.globed2/player-count-icon")) {
            globedIcon->setPositionY(37.0f);
        }
    }

    fields.sendLabel = Build<CCLabelBMFont>::create("0 sends", "bigFont.fnt")
            .scale(0.4f)
            .pos({xPos, yPos})
            .anchorPoint(anchor)
            .id("send-label"_spr)
            .visible(false)
            .parent(this);
}