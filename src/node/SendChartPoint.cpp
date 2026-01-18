#include "SendChartPoint.hpp"

bool SendChartPoint::init(const ccColor3B& color, const std::optional<Send>& send, const std::optional<Rate>& rate, const int index) {
    if (!CCNode::init()) return false;

    if (send.has_value()) sendData = send.value();
    sendIndex = index;

    if (rate.has_value()) {
        rateData = rate.value();
        sprite = CCSprite::createWithSpriteFrameName("rankIcon_top10_001.png");
    } else {
        sprite = CCSprite::createWithSpriteFrameName("d_circle_02_001.png");
        sprite->setColor(color);
    }
    sprite->setPosition(sprite->getContentSize());
    addChild(sprite);

    setContentSize(sprite->getContentSize() * 2.0f);
    setAnchorPoint({0.5f, 0.5f});

    return true;
}

void SendChartPoint::onHover(const bool isHovering) {
    hovering = isHovering;
    if (isHovering) {
        sprite->setScale(2.0f);
    } else {
        sprite->setScale(1.0f);
    }
}

SendChartPoint* SendChartPoint::create(const ccColor3B& color, const std::optional<Send>& send, const int index) {
    auto node = new SendChartPoint();
    if (node->init(color, send, std::nullopt, index)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

SendChartPoint* SendChartPoint::create(const std::optional<Rate>& rate) {
    auto node = new SendChartPoint();
    if (node->init({0, 0, 0}, std::nullopt, rate, 0)) {
        if (rate.has_value()) node->rateData = rate.value();
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

const std::optional<Send>& SendChartPoint::getSendData() const {
    return sendData;
}

const std::optional<Rate>& SendChartPoint::getRateData() const {
    return rateData;
}

int SendChartPoint::getSendIndex() const {
    return sendIndex;
}

void SendChartPoint::setPoint(const LineChartPoint& point) {
    this->point = point;
}

std::optional<LineChartPoint> SendChartPoint::getPoint() const {
    return point;
}