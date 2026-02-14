#include "SendChartPoint.hpp"

bool SendChartPoint::init(const ccColor3B& color, const std::optional<Send>& send, const std::optional<Rate>& rate, const int index, const double score) {
    if (!CCNode::init()) return false;

    if (send.has_value()) sendData = send.value();
    sendIndex = index;
    trendingScore = score;

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
        setZOrder(1);
    } else {
        sprite->setScale(1.0f);
        setZOrder(0);
    }
}

SendChartPoint* SendChartPoint::create(const ccColor3B& color, const std::optional<Send>& send, const int index, const double score) {
    auto node = new SendChartPoint();
    if (node->init(color, send, std::nullopt, index, score)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

SendChartPoint* SendChartPoint::create(const std::optional<Rate>& rate, const double score) {
    auto node = new SendChartPoint();
    if (node->init({0, 0, 0}, std::nullopt, rate, 0, score)) {
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

double SendChartPoint::getTrendingScore() const {
    return trendingScore;
}

void SendChartPoint::setPoint(const LineChartPoint& point) {
    this->point = point;
}

std::optional<LineChartPoint> SendChartPoint::getPoint() const {
    return point;
}