#ifndef SENDDB_SENDCHARTNODE_HPP
#define SENDDB_SENDCHARTNODE_HPP

#include <Geode/Geode.hpp>
#include <model/APIResponse.hpp>

#include "SendChartPoint.hpp"
#include "SendInfoBox.hpp"

using namespace geode::prelude;

enum ChartStyle {
    LineChartStyle_Normal,
    LineChartStyle_Step
};

struct LineChartPoint {
    float x;
    float y;
    bool rated;

    LineChartPoint(const float x, const float y, const bool rated) : x(x), y(y), rated(rated) {}

    [[nodiscard]] CCPoint toCCPoint() const {
        return ccp(x, y);
    }
};

class SendChartNode : public CCNode {
    std::vector<LineChartPoint> processedPoints;
    std::vector<SendChartPoint*> points;
    float lineWidth = 2.0f;
    CCSize chartSize;
    CCSize chartDimensions;
    ChartStyle chartStyle = LineChartStyle_Normal;
    long long startTimestamp = 0;
    std::optional<Level> levelData;

    SendChartPoint* hoveredPoint = nullptr;

    CCNode* labelsNode = nullptr;
    CCDrawNode* gridNode = nullptr;
    CCDrawNode* graphLineNode = nullptr;
    CCMenu* hoverMenu = nullptr;
    CCDrawNode* selectNode = nullptr;
    SendInfoBox* sendInfoBox = nullptr;
    CCLabelBMFont* positionLabel = nullptr;

    bool init(const std::optional<Level>& level, const CCSize& size, float _lineWidth, ChartStyle style);

    void update(float delta) override;

    void drawLabelsAndGrid() const;
    LineChartPoint scalePoint(const LineChartPoint& point) const;

    float getTimestampFromX(float x) const;

public:
    static SendChartNode* create(
        const std::optional<Level>& level,
        const CCSize& size,
        float lineWidth = 2.0f,
        ChartStyle style = LineChartStyle_Normal
    );

    void draw() override;

    void onClick(const CCPoint& position);
};

#endif