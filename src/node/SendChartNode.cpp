#include "SendChartNode.hpp"

#include <numeric>
#include <ranges>

#include <utils/LayoutUtils.hpp>
#include <utils/PointUtils.hpp>
#include <utils/SendUtils.hpp>
#include <utils/Style.hpp>
#include <utils/TimeUtils.hpp>

#include "AntialiasedDrawNode.hpp"

bool SendChartNode::init(const std::optional<Level>& level, const CCSize& size, const float _lineWidth, const ChartType type, const ChartStyle style) {
    if (!CCNode::init()) return false;
    scheduleUpdate();

    setContentSize(size);
    chartSize = size;
    lineWidth = _lineWidth;
    chartType = type;
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
        ccc4FFromccc4B(gridBackgroundColor),
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

    graphLineNode = AntialiasedDrawNode::create();
    graphLineNode->m_bUseArea = false;
    graphLineNode->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
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
            .parent(this)
            .zOrder(1);

    if (level) {
        auto levelValue = level.value();
        levelData = levelValue;

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
            const long long padding = std::clamp(6 * 60 * 60 * 1000.0f, originalTimeRangeMs * 0.1f, 7 * 24 * 60 * 60 * 1000.f);
            startTimestamp -= padding;

            if (levelValue.rate) {
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

            sendChartDimensions = ccp(timeRangeSeconds > 0 ? timeRangeSeconds : 1.0f, yValue);
            viewport = {ccp(0, 0), sendChartDimensions};

            sendChartPoints.push_back(LineChartPoint(0.0f, 0.0f, false));

            const auto sendTimestamps = ranges::map<std::vector<int>>(levelValue.sends, [](const Send& s) {
                return s.timestamp / 1000;
            });

            int i = 0;
            for (const auto& send : levelValue.sends) {
                i++;
                const auto point = LineChartPoint((send.timestamp - startTimestamp) / 1000.f, i, send.timestamp > rateTimestamp);
                sendChartPoints.push_back(point);

                const double trendingScore = SendUtils::calculateTrendingScore(send.timestamp, sendTimestamps);
                const auto scaledPoint = scalePoint(point);
                SendChartPoint* chartPoint = Build<SendChartPoint>::create(pointColor, send, i, trendingScore)
                        .pos(scaledPoint.toCCPoint())
                        .scale(0.1f)
                        .parent(hoverMenu);
                chartPoint->setPoint(point);
                sendPoints.push_back(chartPoint);
            }

            if (!placedRatePoint) {
                const auto point = LineChartPoint((rateTimestamp - startTimestamp) / 1000.f, i, true);
                sendChartPoints.push_back(point);

                const double trendingScore = SendUtils::calculateTrendingScore(rateTimestamp, sendTimestamps);
                const auto scaledPoint = scalePoint(point);
                SendChartPoint* chartPoint = Build<SendChartPoint>::create(levelValue.rate.value(), trendingScore)
                        .pos(scaledPoint.toCCPoint())
                        .scale(0.4f)
                        .parent(hoverMenu);
                chartPoint->setPoint(point);
                sendPoints.push_back(chartPoint);
            }

            if (!timestampsMatch) {
                const auto point = LineChartPoint((lastTimestamp - startTimestamp) / 1000.f, i, lastTimestamp > rateTimestamp);
                sendChartPoints.push_back(point);
            }

            int points = Mod::get()->getSettingValue<int>("trendingSamplePoints");
            std::vector<float> sampledX;
            if (sendChartDimensions.width > 0.0f) {
                const float step = sendChartDimensions.width / static_cast<float>(points);
                for (int i = 0; i <= points; ++i) {
                    sampledX.push_back(i * step);
                }
            }

            float maxTrendingScore = 0.0f;
            placedRatePoint = false;
            for (const auto& x : sampledX) {
                const long long timestamp = startTimestamp + static_cast<long long>(x * 1000.0f);

                const double trendingScore = SendUtils::calculateTrendingScore(timestamp, sendTimestamps);
                if (trendingScore > maxTrendingScore) maxTrendingScore = trendingScore;

                trendingChartPoints.push_back(LineChartPoint(x, static_cast<float>(trendingScore), timestamp > rateTimestamp));
            }

            for (const auto& send : levelValue.sends) {
                const double trendingScore = SendUtils::calculateTrendingScore(send.timestamp, sendTimestamps);
                if (trendingScore > maxTrendingScore) maxTrendingScore = trendingScore;

                const float x = (send.timestamp - startTimestamp) / 1000.f;
                const auto point = LineChartPoint(x, static_cast<float>(trendingScore), send.timestamp > rateTimestamp);
                trendingChartPoints.push_back(point);

                const auto scaledPoint = scalePoint(point);
                SendChartPoint* chartPoint = Build<SendChartPoint>::create(pointColor, send, &send - &levelValue.sends.front() + 1, trendingScore)
                        .pos(scaledPoint.toCCPoint())
                        .scale(0.1f)
                        .parent(hoverMenu);
                chartPoint->setPoint(point);
                trendingPoints.push_back(chartPoint);
            }

            if (!placedRatePoint && levelValue.rate) {
                const double trendingScore = SendUtils::calculateTrendingScore(rateTimestamp, sendTimestamps);
                if (trendingScore > maxTrendingScore) maxTrendingScore = trendingScore;

                const float x = (rateTimestamp - startTimestamp) / 1000.f;
                const auto point = LineChartPoint(x, static_cast<float>(trendingScore), true);
                trendingChartPoints.push_back(point);

                const auto scaledPoint = scalePoint(point);
                SendChartPoint* chartPoint = Build<SendChartPoint>::create(levelValue.rate.value(), trendingScore)
                        .pos(scaledPoint.toCCPoint())
                        .scale(0.4f)
                        .parent(hoverMenu);
                chartPoint->setPoint(point);
                trendingPoints.push_back(chartPoint);
            }

            std::ranges::sort(trendingChartPoints, [](const LineChartPoint& a, const LineChartPoint& b) {
                return a.x < b.x;
            });

            trendingChartDimensions = ccp(timeRangeSeconds > 0 ? timeRangeSeconds : 1.0f, ceil((maxTrendingScore + 15000.0f) / 25000.0f) * 25000.0f);
        }
    }

    drawGraph();
    drawLabelsAndGrid();

    return true;
}

void SendChartNode::update(const float delta) {
    const auto mousePos = getMousePos();
    const auto position = convertToNodeSpace(mousePos);

    if (!touchPoint) {
        selectNode->clear();

        selectNode->drawSegment(
            ccp(position.x, 0),
            ccp(position.x, chartSize.height),
            0.2f,
            ccc4FFromccc4B(selectionLineColor)
        );
        selectNode->drawSegment(
            ccp(0, position.y),
            ccp(chartSize.width, position.y),
            0.2f,
            ccc4FFromccc4B(selectionLineColor)
        );
    }

    if (!PointUtils::isPointInsideNode(this, mousePos)) {
        positionLabel->setVisible(false);
        hovering = false;
        return;
    }
    hovering = true;

    float closestDistanceSq = std::numeric_limits<float>::max();
    hoveredPoint = nullptr;

    for (const auto& point : *getPoints()) {
        if (PointUtils::isPointInsideNode(point, mousePos)) {
            const float distanceSq = PointUtils::squaredDistanceFromNode(point, mousePos);
            if (; distanceSq < closestDistanceSq) {
                closestDistanceSq = distanceSq;
                hoveredPoint = point;
            }
        }
    }

    for (const auto& point : std::views::join(std::array{sendPoints, trendingPoints})) {
        point->onHover(point == hoveredPoint);
    }

    positionLabel->setVisible(true);
    positionLabel->setString(fmt::format("{} - {}", static_cast<int>(position.y / chartSize.height * viewport.size.height + viewport.origin.y), TimeUtils::timestampToDateTime(getTimestampFromX(position.x)).c_str()).c_str());
}

void SendChartNode::handleZoom(const CCPoint& start, const CCPoint& end) {
    const float minX = std::min(start.x, std::clamp(end.x, 0.0f, chartSize.width));
    const float maxX = std::max(start.x, std::clamp(end.x, 0.0f, chartSize.width));
    const float minY = std::min(start.y, std::clamp(end.y, 0.0f, chartSize.height));
    const float maxY = std::max(start.y, std::clamp(end.y, 0.0f, chartSize.height));

    const auto fixedStart = ccp(minX, minY);
    const auto fixedEnd = ccp(maxX, maxY);

    const auto scaledStart = screenToChartPoint(fixedStart);
    const auto scaledEnd = screenToChartPoint(fixedEnd);
    const CCRect newDimensions = {fixedStart, fixedEnd - fixedStart};
    const CCRect scaledDimensions = {scaledStart, scaledEnd - scaledStart};
    if (newDimensions.size.width < 5.0f || newDimensions.size.height < 5.0f) {
        if (hoveredPoint) return;
        viewport = {{0, 0}, getChartDimensions()};
    } else {
        viewport = scaledDimensions;
    }

    for (const auto& point : *getPoints()) {
        const auto scaledPoint = applyViewportScaling(point->getPoint().value().toCCPoint());
        point->setPosition(scaledPoint);
    }

    sendInfoBox->clearData();

    drawGraph();
    drawLabelsAndGrid();
    hoverMenu->updateLayout();
}

void SendChartNode::drawGraph() const {
    switch (chartType) {
        case ChartType::Sends: {
            for (auto& point : sendPoints) {
                point->setVisible(true);
            }
            for (auto& point : trendingPoints) {
                point->setVisible(false);
            }
            drawSendGraph();
            break;
        }
        case ChartType::Trending: {
            for (auto& point : sendPoints) {
                point->setVisible(false);
            }
            for (auto& point : trendingPoints) {
                point->setVisible(true);
            }
            drawTrendingGraph();
            break;
        }
    }
}

void SendChartNode::drawSendGraph() const {
    graphLineNode->clear();

    const ccColor4F sendColorF = ccc4FFromccc3B(sendColor);
    const ccColor4F rateColorF = ccc4FFromccc3B(rateColor);

    const auto scale = ccp(
        chartSize.width / viewport.size.width,
        chartSize.height / viewport.size.height
    );
    const auto offset = -viewport.origin;

    if (sendChartPoints.size() < 2) return;

    switch (chartStyle) {
        case ChartStyle::Line:
            for (size_t i = 1; i < sendChartPoints.size(); i++) {
                ccColor4F color = sendColorF;
                if (sendChartPoints[i - 1].rated) color = rateColorF;

                graphLineNode->drawSegment((sendChartPoints[i - 1].toCCPoint() + offset) * scale, (sendChartPoints[i].toCCPoint() + offset) * scale, lineWidth, color);
            }
            break;
        case ChartStyle::Step:
            for (size_t i = 1; i < sendChartPoints.size(); i++) {
                ccColor4F color = sendColorF;
                if (sendChartPoints[i - 1].rated) color = rateColorF;

                CCPoint prevPoint = sendChartPoints[i - 1].toCCPoint();
                CCPoint currPoint = sendChartPoints[i].toCCPoint();
                CCPoint stepPoint = ccp(currPoint.x, prevPoint.y);

                graphLineNode->drawSegment((prevPoint + offset) * scale, (stepPoint + offset) * scale, lineWidth, color);
                graphLineNode->drawSegment((stepPoint + offset) * scale, (currPoint + offset) * scale, lineWidth, color);
            }
            break;
    }
}

void SendChartNode::drawTrendingGraph() const {
    graphLineNode->clear();

    const ccColor4F trendingColorF = ccc4FFromccc3B(trendingColor);
    const ccColor4F rateColorF = ccc4FFromccc3B(secondaryColor);

    const auto scale = ccp(
        chartSize.width / viewport.size.width,
        chartSize.height / viewport.size.height
    );
    const auto offset = -viewport.origin;

    for (size_t i = 1; i < trendingChartPoints.size(); i++) {
        ccColor4F color = trendingColorF;
        if (trendingChartPoints[i - 1].rated) color = rateColorF;

        graphLineNode->drawSegment((trendingChartPoints[i - 1].toCCPoint() + offset) * scale, (trendingChartPoints[i].toCCPoint() + offset) * scale, lineWidth, color);
    }
}

void SendChartNode::drawLabelsAndGrid() const {
    labelsNode->removeAllChildrenWithCleanup(true);
    gridNode->clear();

    const float startX = viewport.origin.x;
    const float endX = viewport.origin.x + viewport.size.width;
    const float startY = viewport.origin.y;
    const float endY = viewport.origin.y + viewport.size.height;

    const int maxX = static_cast<int>(ceil(endX));
    const int maxY = static_cast<int>(ceil(endY));

    const int startTimeSeconds = static_cast<int>(startTimestamp / 1000LL + startX);

    const ChartAxisLayout xLayout = LayoutUtils::calculateTimeAxisLayout(startTimeSeconds, maxX);
    const ChartAxisLayout yLayout = LayoutUtils::calculateNumericAxisLayout(startY, maxY);

    drawAxis(
        {
            true,
            AxisType::Numeric,
            startY,
            chartSize.height,
            viewport.origin.y,
            viewport.size.height,
            chartSize.height,
            getChartDimensions().height,
        },
        yLayout,
        maxY,
        startY
    );

    drawAxis(
        {
            false,
            AxisType::Time,
            startX,
            chartSize.width,
            viewport.origin.x,
            viewport.size.width,
            chartSize.width,
            getChartDimensions().width,
        },
        xLayout,
        maxX,
        startTimeSeconds
    );
}

int cumulativeGCD(const std::vector<int>& numbers) {
    if (numbers.empty()) return 0;

    int result = numbers[0];
    for (size_t i = 1; i < numbers.size(); ++i) {
        result = std::gcd(result, numbers[i]);
        if (result == 1) break;
    }
    return result;
}

void SendChartNode::drawAxis(
    const AxisRenderConfig& config,
    const ChartAxisLayout& layout,
    const int maxValue,
    const int startValue
) const {
    const int minOffset = LayoutUtils::minStartOffset(layout);
    int endOffset = maxValue / layout.unit;
    const int intervalGCD = cumulativeGCD({layout.tick.interval, layout.label.interval, layout.gridLine.interval});

    for (int i = minOffset; i <= endOffset; i += intervalGCD) {
        const float value = layout.startOffset + i * layout.unit - startValue;
        if (value < 0) {
            endOffset += intervalGCD;
            continue;
        }

        const float chartPos = config.start + value;
        if (chartPos < 0 || chartPos > config.chartDimension) continue;
        const float scaledPos = (chartPos - config.viewportOrigin) / config.viewportSize * config.chartSize;
        if (scaledPos < 0 || scaledPos > config.chartSize) continue;

        if ((i - layout.label.startOffset) % layout.label.interval == 0 && i >= layout.label.startOffset) {
            drawAxisTick(config, scaledPos, value + startValue, true);
        } else if ((i - layout.tick.startOffset) % layout.tick.interval == 0 && i >= layout.tick.startOffset) {
            drawAxisTick(config, scaledPos, value + startValue, false);
        }

        if ((i - layout.gridLine.startOffset) % layout.gridLine.interval == 0 && i >= layout.gridLine.startOffset) {
            drawGridLine(config, scaledPos);
        }
    }
}

void SendChartNode::drawAxisTick(
    const AxisRenderConfig& config,
    const float scaledPos,
    const float absoluteValue,
    const bool isLabel
) const {
    auto tickSprite = Build<CCSprite>::createSpriteName("gridLine01_001.png")
            .color(secondaryColor)
            .parent(labelsNode);

    if (config.isVertical) {
        tickSprite.posY(scaledPos);
    } else {
        tickSprite.posX(scaledPos).rotation(90);
    }

    if (isLabel) {
        tickSprite.scaleX(0.2f);
        createAxisLabel(config, scaledPos, absoluteValue, tickSprite);
    } else {
        tickSprite.scaleX(0.1f).scaleY(0.8f);
    }

    if (config.isVertical) {
        tickSprite.posX(-tickSprite->getScaledContentSize().width);
    } else {
        tickSprite.posY(-tickSprite->getScaledContentSize().width);
    }
}

void SendChartNode::drawGridLine(
    const AxisRenderConfig& config,
    const float scaledPos
) const {
    if (config.isVertical) {
        gridNode->drawSegment(
            ccp(0, scaledPos),
            ccp(chartSize.width, scaledPos),
            0.2f,
            ccc4FFromccc4B(gridLineColor)
        );
    } else {
        gridNode->drawSegment(
            ccp(scaledPos, 0),
            ccp(scaledPos, chartSize.height),
            0.2f,
            ccc4FFromccc4B(gridLineColor)
        );
    }
}

void SendChartNode::createAxisLabel(
    const AxisRenderConfig& config,
    const float scaledPos,
    const float absoluteValue,
    Build<CCSprite>& tickSprite
) const {
    CCPoint anchorPoint;
    float labelPosX, labelPosY;

    if (config.isVertical) {
        anchorPoint = ccp(1.0f, 0.5f);
        labelPosX = -tickSprite->getScaledContentSize().width - 6.0f;
        labelPosY = scaledPos;
    } else {
        anchorPoint = ccp(0.5f, 1.0f);
        labelPosX = scaledPos;
        labelPosY = -tickSprite->getScaledContentSize().width - 6.0f;
    }

    Build<CCLabelBMFont>::create(getLabelText(config, absoluteValue).c_str(), "chatFont.fnt")
        .posX(labelPosX)
        .posY(labelPosY)
        .scale(0.4f)
        .anchorPoint(anchorPoint)
        .color(secondaryTextColor)
        .parent(labelsNode);
}

std::string SendChartNode::getLabelText(
    const AxisRenderConfig& config,
    const float absoluteValue
) {
    switch (config.type) {
        case AxisType::Numeric:
            return fmt::format("{:g}", absoluteValue);
        case AxisType::Time:
            return TimeUtils::timestampToDate(static_cast<long long>(absoluteValue) * 1000LL);
    }
    return "";
}

LineChartPoint SendChartNode::scalePoint(const LineChartPoint& point) const {
    return {
        point.x / getChartDimensions().width * chartSize.width,
        point.y / getChartDimensions().height * chartSize.height,
        point.rated
    };
}

CCPoint SendChartNode::scalePoint(const CCPoint& point) const {
    return {
        point.x / getChartDimensions().width * chartSize.width,
        point.y / getChartDimensions().height * chartSize.height
    };
}

CCPoint SendChartNode::screenToChartPoint(const CCPoint& screenPoint) const {
    return CCPoint{
        screenPoint.x / chartSize.width * viewport.size.width + viewport.origin.x,
        screenPoint.y / chartSize.height * viewport.size.height + viewport.origin.y
    };
}

CCPoint SendChartNode::applyViewportScaling(const CCPoint& point) const {
    return CCPoint{
        (point.x - viewport.origin.x) / viewport.size.width * chartSize.width,
        (point.y - viewport.origin.y) / viewport.size.height * chartSize.height
    };
}

long long SendChartNode::getTimestampFromX(const float x) const {
    const float seconds = x / chartSize.width * viewport.size.width + viewport.origin.x;
    return startTimestamp + seconds * 1000.0f;
}

SendChartNode* SendChartNode::create(const std::optional<Level>& level, const CCSize& size, const float lineWidth, const ChartType type, const ChartStyle style) {
    auto node = new SendChartNode();
    if (node->init(level, size, lineWidth, type, style)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

void SendChartNode::onClick(const CCPoint& position) {
    if (hoveredPoint && hoveredPoint != selectedPoint && PointUtils::isPointInsideNode(hoveredPoint, position)) {
        sendInfoBox->setPosition(hoveredPoint->getPosition());
        selectedPoint = hoveredPoint;
        if (const auto sendData = hoveredPoint->getSendData()) {
            sendInfoBox->setSendData(sendData, hoveredPoint->getSendIndex(), hoveredPoint->getTrendingScore());
        } else if (const auto rateData = hoveredPoint->getRateData()) {
            sendInfoBox->setRateData(rateData, hoveredPoint->getTrendingScore());
        }
    } else {
        sendInfoBox->clearData();
        selectedPoint = nullptr;
        touchPoint = convertToNodeSpace(position);
    }
}

void SendChartNode::onRelease(const CCPoint& position) {
    if (touchPoint) {
        handleZoom(touchPoint.value(), convertToNodeSpace(position));
        touchPoint = std::nullopt;
    }
    if (!PointUtils::isPointInsideNode(this, position)) {
        touchPoint = std::nullopt;
    }
}

void SendChartNode::onHover(const CCPoint& position) {
    if (!touchPoint) return;

    const auto touchPos = touchPoint.value();
    const auto chartPos = convertToNodeSpace(position);

    selectNode->clear();
    selectNode->drawSegment(
        ccp(touchPos.x, 0),
        ccp(touchPos.x, chartSize.height),
        0.2f,
        ccc4FFromccc4B(selectionLineColor)
    );
    selectNode->drawSegment(
        ccp(0, touchPos.y),
        ccp(chartSize.width, touchPos.y),
        0.2f,
        ccc4FFromccc4B(selectionLineColor)
    );

    selectNode->drawRect(
        {touchPos, chartPos - touchPos},
        ccc4FFromccc4B(selectionHighlightColor),
        0.0f,
        {0, 0, 0, 0}
    );

    selectNode->drawSegment(
        ccp(chartPos.x, 0),
        ccp(chartPos.x, chartSize.height),
        0.2f,
        ccc4FFromccc4B(selectionLineColor)
    );
    selectNode->drawSegment(
        ccp(0, chartPos.y),
        ccp(chartSize.width, chartPos.y),
        0.2f,
        ccc4FFromccc4B(selectionLineColor)
    );
}

ChartType SendChartNode::getType() const {
    return chartType;
}

void SendChartNode::setType(const ChartType type) {
    chartType = type;
    viewport = {{0, 0}, getChartDimensions()};
    hoveredPoint = nullptr;
    handleZoom({0.0f, 0.0f}, {0.0f, 0.0f});
}

CCSize SendChartNode::getChartDimensions() const {
    switch (chartType) {
        case ChartType::Sends:
            return sendChartDimensions;
        case ChartType::Trending:
            return trendingChartDimensions;
    }
    return {0, 0};
}

std::vector<SendChartPoint*>* SendChartNode::getPoints() {
    switch (chartType) {
        case ChartType::Sends:
            return &sendPoints;
        case ChartType::Trending:
            return &trendingPoints;
    }
    return nullptr;
}