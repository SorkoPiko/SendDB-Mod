#include "LevelCell.hpp"

#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>
#include <rock/RoundedRect.hpp>
#include <utils/Style.hpp>

void SendDBLevelCell::setLevelInfo(const std::optional<BatchLevel>& info) {
    auto& fields = *m_fields.self();

    fields.levelInfo = info;
    if (fields.levelInfo.has_value()) {
        if (!fields.sendContainer) createSendContainer();

        const int sendCount = fields.levelInfo->send_count;
        const bool accurate = fields.levelInfo->accurate;
        fields.sendLabel->setString((std::to_string(sendCount) + (accurate ? "" : "?") + (sendCount == 1 ? " send" : " sends")).c_str());
        fields.sendContainer->setVisible(true);
    } else {
        if (fields.sendContainer) fields.sendContainer->setVisible(false);
    }
}

void SendDBLevelCell::createSendContainer() {
    auto& fields = *m_fields.self();

    float xPos;
    float yPos;
    CCPoint anchor;

    if (std::abs(m_height - 90.0f) < 10.0f) {
        xPos = 314.0f;
        yPos = 30.0f;
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