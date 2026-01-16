#ifndef SENDDB_SENDCHARTPOINT_HPP
#define SENDDB_SENDCHARTPOINT_HPP

#include <Geode/cocos/menu_nodes/CCMenuItem.h>
#include <model/APIResponse.hpp>

using namespace geode::prelude;


class SendChartPoint : public CCNode {
    std::optional<Send> sendData;
    std::optional<Rate> rateData;
    int sendIndex = 0;
    bool hovering = false;

    CCSprite* sprite = nullptr;

    bool init(const ccColor3B& color, const std::optional<Send>& send, const std::optional<Rate>& rate, int index);

public:
    static SendChartPoint* create(const ccColor3B& color, const std::optional<Send>& send, int index);
    static SendChartPoint* create(const std::optional<Rate>& rate);

    void onHover(bool isHovering);

    const std::optional<Send>& getSendData() const;
    const std::optional<Rate>& getRateData() const;
    int getSendIndex() const;
};

#endif