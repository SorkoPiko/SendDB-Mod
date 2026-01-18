#include "SendChartNode.hpp"

#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>

#include <utils/LayoutUtils.hpp>
#include <utils/PointUtils.hpp>
#include <utils/TimeUtils.hpp>

constexpr ccColor4B selectLineColor = {255, 255, 255, 255};

bool SendChartNode::init(const std::optional<Level>& level, const CCSize& size, const float _lineWidth, const ChartStyle style) {
    if (!CCNode::init()) return false;
    scheduleUpdate();

    setContentSize(size);
    chartSize = size;
    lineWidth = _lineWidth;
    chartStyle = style;

    CCClippingNode* clippingNode = Build<CCClippingNode>::create()
            .parent(this);

    CCPoint maskShape[4] = {
        ccp(0, 0),
        ccp(size.width, 0),
        ccp(size.width, size.height),
        ccp(0, size.height)
    };

    const auto mask = CCDrawNode::create();
    mask->m_bUseArea = false;
    mask->drawPolygon(
        maskShape,
        4,
        ccc4FFromccc4B({0, 0, 0, 120}),
        0.2f,
        ccc4f(1, 1, 1, 0.3f)
    );
    clippingNode->setStencil(mask);
    clippingNode->addChild(mask);

    labelsNode = Build<CCNode>::create()
            .parent(this);

    gridNode = CCDrawNode::create();
    gridNode->m_bUseArea = false;
    gridNode->setZOrder(-1);
    clippingNode->addChild(gridNode);

    graphLineNode = CCDrawNode::create();
    graphLineNode->m_bUseArea = false;
    clippingNode->addChild(graphLineNode);

    hoverMenu = CCMenu::create();
    hoverMenu->setPosition({0, 0});
    hoverMenu->setContentSize(size);
    clippingNode->addChild(hoverMenu);

    selectNode = CCDrawNode::create();
    selectNode->m_bUseArea = false;
    selectNode->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
    clippingNode->addChild(selectNode);

    positionLabel = Build<CCLabelBMFont>::create("", "chatFont.fnt")
            .scale(0.4f)
            .anchorPoint({1.0f, 0.0f})
            .pos(size)
            .visible(false)
            .parent(this);

    sendInfoBox = Build<SendInfoBox>::create()
            .anchorPoint({0.5f, 0.0f})
            .pos({size.width / 2, size.height})
            .parent(this);

    if (level.has_value()) {
        auto levelValue = level.value();
        if (!levelValue.sends.empty()) {
            std::ranges::sort(levelValue.sends, [](const Send& a, const Send& b) {
                return a.timestamp < b.timestamp;
            });

            startTimestamp = levelValue.sends.front().timestamp;
            long long lastTimestamp = levelValue.sends.back().timestamp;
            bool timestampsMatch = true;
            long long rateTimestamp = std::numeric_limits<long long>::max();
            bool placedRatePoint = true;

            const long long originalTimeRangeMs = lastTimestamp - startTimestamp;
            const long long padding = std::clamp(24 * 60 * 60 * 1000.0f, originalTimeRangeMs * 0.1f, 7 * 24 * 60 * 60 * 1000.f);
            startTimestamp -= padding;

            if (levelValue.rate.has_value()) {
                if (const auto& rate = levelValue.rate.value(); rate.timestamp > lastTimestamp) {
                    lastTimestamp = rate.timestamp;
                    rateTimestamp = rate.timestamp;
                    timestampsMatch = false;
                    placedRatePoint = false;

                    const long long originalTimeRangeMs2 = lastTimestamp - startTimestamp;
                    const long long padding2 = std::clamp(24 * 60 * 60 * 1000.0f, originalTimeRangeMs2 * 0.1f, 7 * 24 * 60 * 60 * 1000.f);
                    lastTimestamp += padding2;
                }
            }

            const auto now = std::chrono::system_clock::now();
            const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            if (!placedRatePoint) {
                lastTimestamp = std::min(lastTimestamp, nowMs);
            } else if (nowMs > lastTimestamp) {
                timestampsMatch = false;
                lastTimestamp = nowMs;
            }

            const long long timeRangeMs = lastTimestamp - startTimestamp;
            const float timeRangeSeconds = timeRangeMs / 1000.0f;

            int yValue = ceil(levelValue.sends.size() / 5.0) * 5;
            if (levelValue.sends.size() % 5 == 0) yValue += 1;

            chartDimensions = ccp(timeRangeSeconds > 0 ? timeRangeSeconds : 1.0f, yValue);
            viewport = {ccp(0, 0), chartDimensions};

            processedPoints.push_back(scalePoint(LineChartPoint(0.0f, 0.0f, false)));

            int i = 0;
            for (const auto& send : levelValue.sends) {
                i++;
                const auto point = LineChartPoint((send.timestamp - startTimestamp) / 1000.f, i, send.timestamp > rateTimestamp);
                const auto scaledPoint = scalePoint(point);
                processedPoints.push_back(scaledPoint);

                SendChartPoint* chartPoint = Build<SendChartPoint>::create(ccColor3B{180, 180, 180}, send, i)
                        .pos(scaledPoint.toCCPoint())
                        .scale(0.1f)
                        .parent(hoverMenu);
                chartPoint->setPoint(point);
                points.push_back(chartPoint);
            }

            if (!placedRatePoint) {
                const auto point = LineChartPoint((rateTimestamp - startTimestamp) / 1000.f, i, true);
                const auto scaledPoint = scalePoint(point);
                processedPoints.push_back(scaledPoint);
                placedRatePoint = true;

                SendChartPoint* chartPoint = Build<SendChartPoint>::create(levelValue.rate.value())
                        .pos(scaledPoint.toCCPoint())
                        .scale(0.4f)
                        .parent(hoverMenu);
                chartPoint->setPoint(point);
                points.push_back(chartPoint);
            }

            if (!timestampsMatch) {
                const auto point = LineChartPoint((lastTimestamp - startTimestamp) / 1000.f, i, lastTimestamp > rateTimestamp);
                const auto scaledPoint = scalePoint(point);
                processedPoints.push_back(scaledPoint);
            }
        }

        levelData = levelValue;
    }

    drawLabelsAndGrid();

    return true;
}

void SendChartNode::update(const float delta) {
    const auto mousePos = getMousePos();
    const auto position = convertToNodeSpace(mousePos);

    selectNode->clear();
    if (!PointUtils::isPointInsideNode(this, mousePos)) {
        positionLabel->setVisible(false);
        hovering = false;
        return;
    }
    hovering = true;

    selectNode->drawSegment(
        ccp(position.x, 0),
        ccp(position.x, chartSize.height),
        0.2f,
        ccc4FFromccc4B(selectLineColor)
    );
    selectNode->drawSegment(
        ccp(0, position.y),
        ccp(chartSize.width, position.y),
        0.2f,
        ccc4FFromccc4B(selectLineColor)
    );

    if (touchPoint.has_value()) {
        const auto touchPos = touchPoint.value();

        selectNode->drawSegment(
            ccp(touchPos.x, 0),
            ccp(touchPos.x, chartSize.height),
            0.2f,
            ccc4FFromccc4B(selectLineColor)
        );
        selectNode->drawSegment(
            ccp(0, touchPos.y),
            ccp(chartSize.width, touchPos.y),
            0.2f,
            ccc4FFromccc4B(selectLineColor)
        );

        selectNode->drawRect(
            {touchPos, position - touchPos},
            ccc4FFromccc4B({255, 255, 255, 50}),
            0.0f,
            ccc4FFromccc4B({255, 255, 255, 0})
        );
    }

    float closestDistanceSq = std::numeric_limits<float>::max();
    hoveredPoint = nullptr;

    for (const auto& point : points) {
        if (PointUtils::isPointInsideNode(point, mousePos)) {
            const float distanceSq = PointUtils::squaredDistanceFromNode(point, mousePos);
            if (; distanceSq < closestDistanceSq) {
                closestDistanceSq = distanceSq;
                hoveredPoint = point;
            }
        }
    }

    for (const auto& point : points) {
        point->onHover(point == hoveredPoint);
    }

    positionLabel->setVisible(true);
    positionLabel->setString(fmt::format("{} - {}", static_cast<int>(position.y / chartSize.height * viewport.size.height + viewport.origin.y), TimeUtils::timestampToDateTime(getTimestampFromX(position.x)).c_str()).c_str());
}

void SendChartNode::onClick(const CCPoint& position) {
    if (hoveredPoint && PointUtils::isPointInsideNode(hoveredPoint, position)) {
        sendInfoBox->setPosition(hoveredPoint->getPosition());
        if (const auto sendData = hoveredPoint->getSendData(); sendData.has_value()) {
            sendInfoBox->setSendData(sendData, hoveredPoint->getSendIndex());
        } else if (const auto rateData = hoveredPoint->getRateData(); rateData.has_value()) {
            sendInfoBox->setRateData(rateData);
        }
    } else {
        sendInfoBox->clearData();
        touchPoint = convertToNodeSpace(position);
    }
}

void SendChartNode::onRelease(const CCPoint& position) {
    if (PointUtils::isPointInsideNode(this, position) && touchPoint.has_value()) {
        handleZoom(touchPoint.value(), convertToNodeSpace(position));
    }
    touchPoint = std::nullopt;
}

void SendChartNode::handleZoom(const CCPoint& start, const CCPoint& end) {
    const float minX = std::min(start.x, end.x);
    const float maxX = std::max(start.x, end.x);
    const float minY = std::min(start.y, end.y);
    const float maxY = std::max(start.y, end.y);

    const auto fixedStart = ccp(minX, minY);
    const auto fixedEnd = ccp(maxX, maxY);

    const auto scaledStart = screenToChartPoint(fixedStart);
    const auto scaledEnd = screenToChartPoint(fixedEnd);
    const CCRect newDimensions = {fixedStart, fixedEnd - fixedStart};
    const CCRect scaledDimensions = {scaledStart, scaledEnd - scaledStart};
    if (newDimensions.size.width < 5.0f || newDimensions.size.height < 5.0f) {
        viewport = {{0, 0}, chartDimensions};
    } else {
        viewport = scaledDimensions;
    }

    for (const auto& point : points) {
        const auto scaledPoint = applyViewportScaling(point->getPoint().value().toCCPoint());
        point->setPosition(scaledPoint);
    }

    drawLabelsAndGrid();
}

void SendChartNode::drawLabelsAndGrid() const {
    labelsNode->removeAllChildrenWithCleanup(true);
    gridNode->clear();

    int labelEveryY = 10;
    int gridLineEveryY = 5;
    constexpr ccColor4B labelLineColor = {150, 150, 150, 255};
    constexpr ccColor4B labelColor = {200, 200, 200, 255};
    constexpr ccColor4B gridColor = {100, 100, 100, 80};

    const float startX = viewport.origin.x;
    const float endX = viewport.origin.x + viewport.size.width;
    const float startY = viewport.origin.y;
    const float endY = viewport.origin.y + viewport.size.height;

    const int maxX = static_cast<int>(ceil(endX));
    const int maxY = static_cast<int>(ceil(endY));

    const int startTimeSeconds = static_cast<int>(startTimestamp / 1000LL + startX);
    const int timeRangeSeconds = static_cast<int>(ceil(viewport.size.width));

    const XAxisLayout xLayout = LayoutUtils::calculateXAxisLayout(startTimeSeconds, maxX);

    if (maxY < 20) {
        labelEveryY = 5;
        gridLineEveryY = 2;
    }

    for (int seconds = xLayout.firstTickOffset; seconds <= timeRangeSeconds; seconds += xLayout.tickIntervalSeconds) {
        if (seconds < 0) continue;

        const float chartX = startX + seconds;
        if (chartX < 0 || chartX > chartDimensions.width) continue;
        const float scaledX = (chartX - viewport.origin.x) / viewport.size.width * chartSize.width;
        if (scaledX < 0 || scaledX > chartSize.width) continue;

        auto tickSprite = Build<CCSprite>::createSpriteName("gridLine01_001.png")
                .posX(scaledX)
                .rotation(90)
                .color({labelLineColor.r, labelLineColor.g, labelLineColor.b})
                .opacity(labelLineColor.a)
                .parent(labelsNode);

        const int absoluteTimeSeconds = startTimeSeconds + seconds;

        if ((seconds - xLayout.firstLabelOffset) % xLayout.labelIntervalSeconds == 0 && seconds >= xLayout.firstLabelOffset) {
            tickSprite.scaleX(0.2f);

            std::string labelText = TimeUtils::timestampToDate(static_cast<long long>(absoluteTimeSeconds) * 1000LL);

            Build<CCLabelBMFont>::create(labelText.c_str(), "chatFont.fnt")
                    .posX(scaledX)
                    .scale(0.4f)
                    .posY(-tickSprite->getScaledContentSize().width - 6.0f)
                    .anchorPoint({0.5f, 1.0f})
                    .color({labelColor.r, labelColor.g, labelColor.b})
                    .opacity(labelColor.a)
                    .parent(labelsNode);
        } else {
            tickSprite.scaleX(0.1f);
            tickSprite.scaleY(0.8f);
        }
        tickSprite.posY(-tickSprite->getScaledContentSize().width);
    }

    for (int i = static_cast<int>(floor(startY)); i <= maxY; i++) {
        if (i < 0) continue;
        const float scaledY = (i - viewport.origin.y) / viewport.size.height * chartSize.height;
        if (scaledY < 0 || scaledY > chartSize.height) continue;

        auto tickSprite = Build<CCSprite>::createSpriteName("gridLine01_001.png")
                .posY(scaledY)
                .color({labelLineColor.r, labelLineColor.g, labelLineColor.b})
                .opacity(labelLineColor.a)
                .parent(labelsNode);

        if (i % labelEveryY == 0) {
            tickSprite.scaleX(0.2f);

            Build<CCLabelBMFont>::create(std::to_string(i).c_str(), "chatFont.fnt")
                    .posY(scaledY)
                    .scale(0.4f)
                    .posX(-tickSprite->getScaledContentSize().width - 6.0f)
                    .anchorPoint({1.0f, 0.5f})
                    .color({labelColor.r, labelColor.g, labelColor.b})
                    .opacity(labelColor.a)
                    .parent(labelsNode);
        } else {
            tickSprite.scaleX(0.1f);
            tickSprite.scaleY(0.8f);
        }

        tickSprite.posX(-tickSprite->getScaledContentSize().width);
    }

    for (int seconds = xLayout.firstGridOffset; seconds <= maxX; seconds += xLayout.gridLineIntervalSeconds) {
        if (seconds < 0) continue;

        const float chartX = startX + seconds;
        if (chartX < 0 || chartX > chartDimensions.width) continue;
        const float scaledX = (chartX - viewport.origin.x) / viewport.size.width * chartSize.width;
        if (scaledX < 0 || scaledX > chartSize.width) continue;

        gridNode->drawSegment(
            ccp(scaledX, 0),
            ccp(scaledX, chartSize.height),
            0.2f,
            ccc4FFromccc4B(gridColor)
        );
    }

    for (int i = static_cast<int>(floor(startY)); i <= maxY; i++) {
        if (i < 0 || i % gridLineEveryY != 0) continue;
        const float scaledY = (i - viewport.origin.y) / viewport.size.height * chartSize.height;
        if (scaledY < 0 || scaledY > chartSize.height) continue;
        gridNode->drawSegment(
            ccp(0, scaledY),
            ccp(chartSize.width, scaledY),
            0.2f,
            ccc4FFromccc4B(gridColor)
        );
    }
}

LineChartPoint SendChartNode::scalePoint(const LineChartPoint& point) const {
    return {
        point.x / chartDimensions.width * chartSize.width,
        point.y / chartDimensions.height * chartSize.height,
        point.rated
    };
}

CCPoint SendChartNode::scalePoint(const CCPoint& point) const {
    return {
        point.x / chartDimensions.width * chartSize.width,
        point.y / chartDimensions.height * chartSize.height
    };
}

CCPoint SendChartNode::screenToChartPoint(const CCPoint& screenPoint) const {
    return CCPoint{
        screenPoint.x / chartSize.width * viewport.size.width + viewport.origin.x,
        screenPoint.y / chartSize.height * viewport.size.height + viewport.origin.y
    };
}


float SendChartNode::getTimestampFromX(const float x) const {
    const float seconds = x / chartSize.width * viewport.size.width + viewport.origin.x;
    return startTimestamp + seconds * 1000.0f;
}

SendChartNode* SendChartNode::create(const std::optional<Level>& level, const CCSize& size, const float lineWidth, const ChartStyle style) {
    auto node = new SendChartNode();
    if (node->init(level, size, lineWidth, style)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

CCPoint SendChartNode::applyViewportScaling(const CCPoint& point) const {
    return CCPoint{
        (point.x - viewport.origin.x) / viewport.size.width * chartSize.width,
        (point.y - viewport.origin.y) / viewport.size.height * chartSize.height
    };
}

void SendChartNode::draw() {
    CCNode::draw();
    graphLineNode->clear();

    constexpr ccColor3B sendColor = {0, 255, 0};
    constexpr ccColor3B rateColor = {212, 175, 55};

    const ccColor4F sendColorF = ccc4FFromccc3B(sendColor);
    const ccColor4F rateColorF = ccc4FFromccc3B(rateColor);

    const auto scale = ccp(
        chartDimensions.width / viewport.size.width,
        chartDimensions.height / viewport.size.height
    );
    const auto offset = -scalePoint(viewport.origin);

    if (processedPoints.size() < 2) return;

    switch (chartStyle) {
        case LineChartStyle_Line:
            for (size_t i = 1; i < processedPoints.size(); i++) {
                ccColor4F color = sendColorF;
                if (processedPoints[i - 1].rated) color = rateColorF;

                graphLineNode->drawSegment((processedPoints[i - 1].toCCPoint() + offset) * scale, (processedPoints[i].toCCPoint() + offset) * scale, lineWidth, color);
            }
            break;
        case LineChartStyle_Step:
            for (size_t i = 1; i < processedPoints.size(); i++) {
                ccColor4F color = sendColorF;
                if (processedPoints[i - 1].rated) color = rateColorF;

                CCPoint prevPoint = processedPoints[i - 1].toCCPoint();
                CCPoint currPoint = processedPoints[i].toCCPoint();
                CCPoint stepPoint = ccp(currPoint.x, prevPoint.y);

                graphLineNode->drawSegment(prevPoint * scale + offset, stepPoint * scale + offset, lineWidth, color);
                graphLineNode->drawSegment(stepPoint * scale + offset, currPoint * scale + offset, lineWidth, color);
            }
            break;
    }
}