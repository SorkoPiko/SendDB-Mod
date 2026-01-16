#include "SendInfoBox.hpp"

#include <rock/RoundedRect.hpp>
#include <utils/TimeUtils.hpp>

std::string plural(const int num) {
    static std::string s;
    s = num == 1 ? "" : "s";
    return s;
}

std::string ago(const int num, const std::string& unit) {
    static std::string s;
    s = std::to_string(num) + " " + unit + (num == 1 ? "" : "s") + " ago";
    return s;
}

std::string timestampAgo(long long timestamp) {
    const auto timeT = timestamp / 1000;
    tm timeInfo;
#ifdef _WIN32
    localtime_s(&timeInfo, &timeT);
#else
    localtime_r(&timeT, &timeInfo);
#endif

    const auto now = std::chrono::system_clock::now();
    const auto sendTime = std::chrono::system_clock::from_time_t(timeT);
    const auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - sendTime).count();

    std::string relativeTime;
    if (diff < 60) {
        relativeTime = ago(diff, "second");
    } else if (diff < 60 * 60) {
        relativeTime = ago(diff / 60, "minute");
    } else if (diff < 24 * 60 * 60) {
        relativeTime = ago(diff / (60 * 60), "hour");
    } else if (diff < 30 * 24 * 60 * 60) {
        relativeTime = ago(diff / (24 * 60 * 60), "day");
    } else if (diff < 365 * 24 * 60 * 60) {
        relativeTime = ago(diff / (30 * 24 * 60 * 60), "month");
    } else {
        relativeTime = ago(diff / (365 * 24 * 60 * 60), "year");
    }
    return relativeTime;
}

bool SendInfoBox::init() {
    if (!CCNode::init()) return false;
    scheduleUpdate();

    constexpr ccColor4B bgColor = {25, 25, 25, 220};

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
        ccc4FFromccc4B(bgColor),
        0.0f,
        {0.0f, 0.0f, 0.0f, 0.0f}
    );
    triangle->setPosition({30.0f, 0.0f});
    triangle->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
    addChild(triangle);

    const auto main = rock::RoundedRect::create(
        bgColor,
        3.0f,
        {60.0f, 30.0f}
    );
    main->setPosition({0.0f, 10.5f});
    main->setAnchorPoint({0.0f, 0.0f});
    addChild(main);

    infoLabel = CCLabelBMFont::create("Send #1", "bigFont.fnt");
    infoLabel->setScale(0.4f);
    infoLabel->setPosition({30.0f, 40.0f});
    infoLabel->setAnchorPoint({0.5f, 1.0f});
    addChild(infoLabel);

    timeLabel = CCLabelBMFont::create("11:56AM 01/02/2026", "chatFont.fnt");
    timeLabel->setScale(0.4f);
    timeLabel->setPosition({30.0f, 23.0f});
    timeLabel->setAnchorPoint({0.5f, 0.5f});
    addChild(timeLabel);

    timeLabel2 = CCLabelBMFont::create("2 days ago", "chatFont.fnt");
    timeLabel2->setScale(0.4f);
    timeLabel2->setPosition({30.0f, 16.0f});
    timeLabel2->setAnchorPoint({0.5f, 0.5f});
    addChild(timeLabel2);

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
    timeLabel2->setString(timestampAgo(timestamp).c_str());
}

void SendInfoBox::updateRate() const {
    const auto rate = rateData.value();
    timeLabel2->setString(timestampAgo(rate.timestamp).c_str());
}

void SendInfoBox::prepareSend() {
    if (!sendData.has_value()) return;

    const auto [timestamp] = sendData.value();

    infoLabel->setString(("Send #" + std::to_string(sendIndex)).c_str());
    timeLabel->setString(TimeUtils::timestampToDate(timestamp).c_str());

    updateSend();
}

void SendInfoBox::prepareRate() {
    if (!rateData.has_value()) return;

    infoLabel->setString("Rated");
    timeLabel->setString(TimeUtils::timestampToDate(rateData->timestamp).c_str());

    updateRate();
}