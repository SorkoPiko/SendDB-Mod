#pragma once

#include <Geode/cocos/cocoa/CCGeometry.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>
#include <Geode/cocos/support/CCPointExtension.h>
#include <model/APIResponse.hpp>

using namespace geode::prelude;

struct LineChartPoint {
    float x;
    float y;
    bool rated;

    LineChartPoint(const float x, const float y, const bool rated) : x(x), y(y), rated(rated) {}

    [[nodiscard]] CCPoint toCCPoint() const {
        return ccp(x, y);
    }
};

class SendChartPoint : public CCNode {
    std::optional<Send> sendData;
    std::optional<Rate> rateData;
    std::optional<LineChartPoint> point;
    int sendIndex = 0;
    bool hovering = false;

    CCSprite* sprite = nullptr;

    bool init(const ccColor3B& color, const std::optional<Send>& send, const std::optional<Rate>& rate, int index);

public:
    static SendChartPoint* create(const ccColor3B& color, const std::optional<Send>& send, int index);
    static SendChartPoint* create(const std::optional<Rate>& rate);

    void onHover(bool isHovering);

    [[nodiscard]] const std::optional<Send>& getSendData() const;
    [[nodiscard]] const std::optional<Rate>& getRateData() const;
    [[nodiscard]] int getSendIndex() const;

    void setPoint(const LineChartPoint& point);
    [[nodiscard]] std::optional<LineChartPoint> getPoint() const;
};