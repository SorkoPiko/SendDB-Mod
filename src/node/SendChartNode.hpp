#ifndef SENDDB_SENDCHARTNODE_HPP
#define SENDDB_SENDCHARTNODE_HPP

#include <Geode/Geode.hpp>
#include <model/APIResponse.hpp>

using namespace geode::prelude;

enum ChartStyle {
    LineChartStyle_Normal,
    LineChartStyle_Step
};

struct LineChartPoint {
    float x;
    float y;

    LineChartPoint(const float x, const float y) : x(x), y(y) {}
};

class SendChartNode : public CCDrawNode {
    std::vector<CCPoint> processedPoints;
    ccColor3B lineColor = {255, 255, 255};
    float lineWidth = 2.0f;
    CCSize chartSize;
    CCSize chartDimensions;
    ChartStyle chartStyle = LineChartStyle_Normal;
    std::optional<Level> levelData;

    bool init(const std::optional<Level>& level, const CCSize& size, const ccColor3B& color, float _lineWidth, ChartStyle style);

    CCPoint scalePoint(const LineChartPoint& point) const;

public:
    static SendChartNode* create(
        const std::optional<Level>& level,
        const CCSize& size,
        const ccColor3B& color = {255, 255, 255},
        float lineWidth = 2.0f,
        ChartStyle style = LineChartStyle_Normal
    );

    void draw() override;
};

#endif