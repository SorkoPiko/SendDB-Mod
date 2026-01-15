#include "SendChartPoint.hpp"

bool SendChartPoint::init(const ccColor3B& color, const std::optional<Send>& send) {
    if (!CCMenuItem::init()) return false;

    if (send.has_value()) sendData = send.value();

    sprite = CCSprite::createWithSpriteFrameName("d_circle_02_001.png");
    sprite->setColor(color);
    sprite->setPosition(sprite->getContentSize());
    addChild(sprite);

    setContentSize(sprite->getContentSize() * 2.0f);
    setAnchorPoint({0.5f, 0.5f});

    setEnabled(true);
    scheduleUpdate();

    return true;
}

void SendChartPoint::update(const float delta) {
    CCMenuItem::update(delta);

    if (lastSelected != m_bSelected) {
        if (m_bSelected) {
            sprite->setScale(2.0f);
            if (delegate) delegate->onSelectChartPoint(this);
        } else {
            sprite->setScale(1.0f);
            if (delegate) delegate->onDeselectChartPoint(this);
        }
    }

    lastSelected = m_bSelected;
}

SendChartPoint* SendChartPoint::create(const ccColor3B& color, const std::optional<Send>& send) {
    auto node = new SendChartPoint();
    if (node->init(color, send)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

void SendChartPoint::setDelegate(ChartPointCallback* delegate) {
    this->delegate = delegate;
}

const std::optional<Send>& SendChartPoint::getSendData() const {
    return sendData;
}
