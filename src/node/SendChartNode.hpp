#ifndef SENDDB_SENDCHARTNODE_HPP
#define SENDDB_SENDCHARTNODE_HPP

#include <Geode/Geode.hpp>
#include <model/APIResponse.hpp>
#include <utils/LayoutUtils.hpp>

#include <UIBuilder.hpp>

#include "SendChartPoint.hpp"
#include "SendInfoBox.hpp"

using namespace geode::prelude;

enum class ChartType {
    Sends,
    Trending
};

enum class ChartStyle {
    Line,
    Step
};

enum class AxisType {
    Numeric,
    Time
};

struct AxisRenderConfig {
    bool isVertical;
    AxisType type;
    float start;
    float end;
    float viewportOrigin;
    float viewportSize;
    float chartSize;
    float chartDimension;
};

class SendChartNode : public CCNode {
    std::vector<LineChartPoint> sendChartPoints;
    std::vector<SendChartPoint*> sendPoints;
    std::vector<LineChartPoint> trendingChartPoints;
    std::vector<SendChartPoint*> trendingPoints;
    float lineWidth = 2.0f;
    CCSize chartSize;
    CCSize sendChartDimensions;
    CCSize trendingChartDimensions;
    CCRect viewport;
    ChartType chartType = ChartType::Sends;
    ChartStyle chartStyle = ChartStyle::Line;
    long long startTimestamp = 0;
    std::optional<Level> levelData;

    bool hovering = false;
    SendChartPoint* hoveredPoint = nullptr;
    SendChartPoint* selectedPoint = nullptr;

    std::optional<CCPoint> touchPoint;

    CCNode* labelsNode = nullptr;
    CCDrawNode* gridNode = nullptr;
    CCDrawNode* graphLineNode = nullptr;
    CCMenu* hoverMenu = nullptr;
    CCDrawNode* selectNode = nullptr;
    SendInfoBox* sendInfoBox = nullptr;
    CCLabelBMFont* positionLabel = nullptr;

    bool init(const std::optional<Level>& level, const CCSize& size, float _lineWidth, ChartType type, ChartStyle style);

    void update(float delta) override;

    void handleZoom(const CCPoint& start, const CCPoint& end);

    void drawGraph() const;
    void drawSendGraph() const;
    void drawTrendingGraph() const;

    void drawLabelsAndGrid() const;

    void drawAxis(const AxisRenderConfig& config, const ChartAxisLayout& layout, int maxValue, int startValue) const;
    void drawAxisTick(const AxisRenderConfig& config, float scaledPos, float absoluteValue, bool isLabel) const;
    void drawGridLine(const AxisRenderConfig& config, float scaledPos) const;
    void createAxisLabel(const AxisRenderConfig& config, float scaledPos, float absoluteValue, Build<CCSprite>& tickSprite) const;
    static std::string getLabelText(const AxisRenderConfig& config, float absoluteValue) ;

    [[nodiscard]] LineChartPoint scalePoint(const LineChartPoint& point) const;
    [[nodiscard]] CCPoint scalePoint(const CCPoint& point) const;
    [[nodiscard]] CCPoint screenToChartPoint(const CCPoint& screenPoint) const;
    [[nodiscard]] CCPoint applyViewportScaling(const CCPoint& point) const;

    [[nodiscard]] long long getTimestampFromX(float x) const;

    CCSize getChartDimensions() const;
    std::vector<SendChartPoint*>* getPoints();

public:
    static SendChartNode* create(
        const std::optional<Level>& level,
        const CCSize& size,
        float lineWidth = 2.0f,
        ChartType type = ChartType::Sends,
        ChartStyle style = ChartStyle::Line
    );

    void onClick(const CCPoint& position);
    void onRelease(const CCPoint& position);

    ChartType getType() const;
    void setType(ChartType type);
};

static ChartStyle chartStyleFromString(const std::string& str) {
    if (str == "Line") {
        return ChartStyle::Line;
    } else if (str == "Step") {
        return ChartStyle::Step;
    } else {
        log::warn("Unknown chart style '{}', defaulting to Line", str);
        return ChartStyle::Line;
    }
}

#endif