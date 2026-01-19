#include "SendInfoBox.hpp"

#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>
#include <rock/RoundedRect.hpp>
#include <utils/Style.hpp>

#include <utils/TimeUtils.hpp>

bool SendInfoBox::init() {
    if (!CCNode::init()) return false;
    scheduleUpdate();

    setContentSize({60.0f, 40.0f});

    CCPoint triangleShape[3] = {
        {-10.0f, 10.0f},
        {10.0f, 10.0f},
        {0.0f, 0.0f},
    };

    const auto triangle = CCDrawNode::create();
    triangle->drawPolygon(
        triangleShape,
        3,
        ccc4FFromccc4B(infoBoxColor),
        0.0f,
        {0.0f, 0.0f, 0.0f, 0.0f}
    );
    triangle->setPosition({30.0f, 0.0f});
    triangle->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
    addChild(triangle);

    Build(rock::RoundedRect::create(
        infoBoxColor,
        3.0f,
        {60.0f, 30.0f}
    ))
            .pos({0.0f, 10.5f})
            .anchorPoint({0.0f, 0.0f})
            .parent(this);

    infoLabel = Build<CCLabelBMFont>::create("Send #1", "bigFont.fnt")
            .scale(0.4f)
            .pos({30.0f, 40.0f})
            .anchorPoint({0.5f, 1.0f})
            .parent(this);

    timeLabel = Build<CCLabelBMFont>::create("11:56AM 01/02/2026", "chatFont.fnt")
            .scale(0.4f)
            .pos({30.0f, 23.0f})
            .anchorPoint({0.5f, 0.5f})
            .parent(this);

    timeLabel2 = Build<CCLabelBMFont>::create("2 days ago", "chatFont.fnt")
            .scale(0.4f)
            .pos({30.0f, 16.0f})
            .anchorPoint({0.5f, 0.5f})
            .parent(this);

    setVisible(false);

    return true;
}

SendInfoBox* SendInfoBox::create() {
    auto node = new SendInfoBox();
    if (node->init()) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

void SendInfoBox::setSendData(const std::optional<Send>& send, const int index) {
    sendIndex = index;
    rateData.reset();
    if (send.has_value()) {
        sendData = send.value();
        setVisible(true);
        prepareSend();
    } else clearData();
}

void SendInfoBox::setRateData(const std::optional<Rate>& rate) {
    sendData.reset();
    if (rate.has_value()) {
        rateData = rate.value();
        setVisible(true);
        prepareRate();
    } else clearData();
}

void SendInfoBox::clearData() {
    sendData.reset();
    rateData.reset();
    setVisible(false);
}

void SendInfoBox::update(float delta) {
    if (sendData.has_value()) updateSend();
    if (rateData.has_value()) updateRate();
}

void SendInfoBox::updateSend() const {
    const auto [timestamp] = sendData.value();
    timeLabel2->setString(TimeUtils::timestampAgo(timestamp).c_str());
}

void SendInfoBox::updateRate() const {
    const auto rate = rateData.value();
    timeLabel2->setString(TimeUtils::timestampAgo(rate.timestamp).c_str());
}

void SendInfoBox::prepareSend() {
    if (!sendData.has_value()) return;

    const auto [timestamp] = sendData.value();

    infoLabel->setString(("Send #" + std::to_string(sendIndex)).c_str());
    timeLabel->setString(TimeUtils::timestampToDateTime(timestamp).c_str());

    updateSend();
}

void SendInfoBox::prepareRate() {
    if (!rateData.has_value()) return;

    infoLabel->setString(fmt::format("Rated{}", rateData->accurate ? "" : "(?)").c_str());
    timeLabel->setString(TimeUtils::timestampToDateTime(rateData->timestamp).c_str());

    updateRate();
}