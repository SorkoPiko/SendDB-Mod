#ifndef SENDDB_SENDCHARTPOINT_HPP
#define SENDDB_SENDCHARTPOINT_HPP

#include <Geode/cocos/menu_nodes/CCMenuItem.h>
#include <model/APIResponse.hpp>

using namespace geode::prelude;

class ChartPointCallback;

class SendChartPoint : public CCMenuItem {
    CCSprite* sprite = nullptr;
    std::optional<Send> sendData;
    ChartPointCallback* delegate = nullptr;

    bool lastSelected = false;

    bool init(const ccColor3B& color, const std::optional<Send>& send);
    void update(float delta) override;

public:
    static SendChartPoint* create(const ccColor3B& color, const std::optional<Send>& send);

    void setDelegate(ChartPointCallback* delegate);
    const std::optional<Send>& getSendData() const;
};

class ChartPointCallback {
public:
    virtual ~ChartPointCallback() = default;

    virtual void onSelectChartPoint(SendChartPoint* point) {}
    virtual void onDeselectChartPoint(SendChartPoint* point) {}
};

#endif