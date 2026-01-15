#include "SendChartNode.hpp"

#include "SendChartPoint.hpp"

bool SendChartNode::init(const std::optional<Level>& level, const CCSize& size, const ccColor3B& color, const float _lineWidth, const ChartStyle style) {
    if (!CCDrawNode::init()) return false;

    setContentSize(size);
    chartSize = size;
    lineColor = color;
    lineWidth = _lineWidth;
    chartStyle = style;
    if (level.has_value()) {
        auto levelValue = level.value();
        if (!levelValue.sends.empty()) {
            std::ranges::sort(levelValue.sends, [](const Send& a, const Send& b) {
                return a.timestamp < b.timestamp;
            });

            const float firstTimestamp = levelValue.sends.front().timestamp;
            const float lastTimestamp = levelValue.sends.back().timestamp;
            const float timeRange = lastTimestamp - firstTimestamp;

            chartDimensions = ccp(timeRange > 0 ? timeRange : 1.0f, ceil(levelValue.sends.size() / 5.0) * 5);

            int i = 0;
            for (const auto& send : levelValue.sends) {
                i++;
                const auto point = LineChartPoint(send.timestamp - firstTimestamp, i);
                const auto scaledPoint = scalePoint(point);

                processedPoints.push_back(scaledPoint);
                const auto chartPoint = SendChartPoint::create({50, 50, 50}, send);
                chartPoint->setPosition(scaledPoint);
                chartPoint->setScale(0.15f);
                addChild(chartPoint);
            }
        }

        levelData = levelValue;
    }

    return true;
}

CCPoint SendChartNode::scalePoint(const LineChartPoint& point) const {
    return ccp(
        point.x / chartDimensions.width * chartSize.width,
        point.y / chartDimensions.height * chartSize.height
    );
}

SendChartNode* SendChartNode::create(const std::optional<Level>& level, const CCSize& size, const ccColor3B& color, const float lineWidth, const ChartStyle style) {
    auto node = new SendChartNode();
    if (node->init(level, size, color, lineWidth, style)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

void SendChartNode::draw() {
    CCDrawNode::draw();

    const ccColor4F lineColorF = ccc4FFromccc3B(lineColor);

    if (processedPoints.size() < 2) return;

    switch (chartStyle) {
        case LineChartStyle_Normal:
            for (size_t i = 1; i < processedPoints.size(); i++) {
                drawSegment(processedPoints[i - 1], processedPoints[i], lineWidth, lineColorF);
            }
        case LineChartStyle_Step:
            for (size_t i = 1; i < processedPoints.size(); i++) {
                CCPoint prevPoint = processedPoints[i - 1];
                CCPoint currPoint = processedPoints[i];
                CCPoint stepPoint = ccp(currPoint.x, prevPoint.y);

                drawSegment(prevPoint, stepPoint, lineWidth, lineColorF);
                drawSegment(stepPoint, currPoint, lineWidth, lineColorF);
            }
    }
}