#include "SendChartNode.hpp"

#include <utils/LayoutUtils.hpp>
#include <utils/PointUtils.hpp>
#include <utils/TimeUtils.hpp>

bool SendChartNode::init(const std::optional<Level>& level, const CCSize& size, const float _lineWidth, const ChartStyle style) {
    if (!CCNode::init()) return false;
    scheduleUpdate();

    setContentSize(size);
    chartSize = size;
    lineWidth = _lineWidth;
    chartStyle = style;

    const auto clippingNode = CCClippingNode::create();
    addChild(clippingNode);

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

    labelsNode = CCNode::create();
    addChild(labelsNode);

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
    clippingNode->addChild(selectNode);

    positionLabel = CCLabelBMFont::create("", "chatFont.fnt");
    positionLabel->setScale(0.4f);
    positionLabel->setAnchorPoint({1.0f, 0.0f});
    positionLabel->setPosition(size);
    positionLabel->setVisible(false);
    addChild(positionLabel);

    sendInfoBox = SendInfoBox::create();
    sendInfoBox->setAnchorPoint({0.5f, 0.0f});
    sendInfoBox->setPosition({size.width / 2, size.height});
    addChild(sendInfoBox);

    if (level.has_value()) {
        auto levelValue = level.value();
        if (!levelValue.sends.empty()) {
            std::ranges::sort(levelValue.sends, [](const Send& a, const Send& b) {
                return a.timestamp < b.timestamp;
            });

            startTimestamp = levelValue.sends.front().timestamp;
            float lastTimestamp = levelValue.sends.back().timestamp;
            bool timestampsMatch = true;
            float rateTimestamp = FLT_MAX;
            bool placedRatePoint = true;

            const float originalTimeRangeMs = lastTimestamp - startTimestamp;
            const float padding = std::clamp(24 * 60 * 60 * 1000.0f, originalTimeRangeMs * 0.1f, 7 * 24 * 60 * 60 * 1000.f);
            startTimestamp -= padding;

            if (levelValue.rate.has_value()) {
                if (const auto& rate = levelValue.rate.value(); rate.timestamp > lastTimestamp) {
                    lastTimestamp = rate.timestamp;
                    rateTimestamp = rate.timestamp;
                    timestampsMatch = false;
                    placedRatePoint = false;

                    const float originalTimeRangeMs2 = lastTimestamp - startTimestamp;
                    const float padding2 = std::clamp(24 * 60 * 60 * 1000.0f, originalTimeRangeMs2 * 0.1f, 7 * 24 * 60 * 60 * 1000.f);
                    lastTimestamp += padding2;
                }
            }

            const auto now = std::chrono::system_clock::now();
            const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            if (!placedRatePoint) {
                lastTimestamp = std::min(lastTimestamp, static_cast<float>(nowMs));
            } else if (nowMs > lastTimestamp) {
                timestampsMatch = false;
                lastTimestamp = nowMs;
            }

            const float timeRangeMs = lastTimestamp - startTimestamp;
            const float timeRangeSeconds = timeRangeMs / 1000.0f;

            int yValue = ceil(levelValue.sends.size() / 5.0) * 5;
            if (levelValue.sends.size() % 5 == 0) yValue += 1;

            chartDimensions = ccp(timeRangeSeconds > 0 ? timeRangeSeconds : 1.0f, yValue);

            processedPoints.push_back(scalePoint(LineChartPoint(0.0f, 0.0f, false)));

            int i = 0;
            for (const auto& send : levelValue.sends) {
                i++;
                const auto point = LineChartPoint((send.timestamp - startTimestamp) / 1000.f, i, send.timestamp > rateTimestamp);
                const auto scaledPoint = scalePoint(point);
                processedPoints.push_back(scaledPoint);

                auto chartPoint = SendChartPoint::create({200, 200, 200}, send, i);
                chartPoint->setPosition(scaledPoint.toCCPoint());
                chartPoint->setScale(0.1f);
                hoverMenu->addChild(chartPoint);
                points.push_back(chartPoint);
            }

            if (!placedRatePoint) {
                const auto point = LineChartPoint((rateTimestamp - startTimestamp) / 1000.f, i, true);
                const auto scaledPoint = scalePoint(point);
                processedPoints.push_back(scaledPoint);
                placedRatePoint = true;

                auto chartPoint = SendChartPoint::create(levelValue.rate.value());
                chartPoint->setPosition(scaledPoint.toCCPoint());
                chartPoint->setScale(0.4f);
                hoverMenu->addChild(chartPoint);
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

    constexpr ccColor4B selectLineColor = {255, 255, 255, 255};

    selectNode->clear();
    if (!PointUtils::isPointInsideNode(this, mousePos)) {
        positionLabel->setVisible(false);
        return;
    }

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

    float closestDistanceSq = FLT_MAX;
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
    positionLabel->setString(fmt::format("{} - {}", static_cast<int>(position.y / chartSize.height * chartDimensions.height), TimeUtils::timestampToDateTime(getTimestampFromX(position.x)).c_str()).c_str());
}

void SendChartNode::onClick(const CCPoint& position) {
    if (hoveredPoint && PointUtils::isPointInsideNode(hoveredPoint, position)) {
        if (const auto sendData = hoveredPoint->getSendData(); sendData.has_value()) {
            sendInfoBox->setSendData(sendData, hoveredPoint->getSendIndex());
        } else if (const auto rateData = hoveredPoint->getRateData(); rateData.has_value()) {
            sendInfoBox->setRateData(rateData);
        }
        sendInfoBox->setPosition(hoveredPoint->getPosition());
    } else {
        sendInfoBox->clearData();
    }
}

void SendChartNode::drawLabelsAndGrid() const {
    int labelEveryY = 10;
    int gridLineEveryY = 5;
    constexpr ccColor4B labelLineColor = {150, 150, 150, 255};
    constexpr ccColor4B labelColor = {200, 200, 200, 255};
    constexpr ccColor4B gridColor = {100, 100, 100, 80};

    const int maxX = static_cast<int>(ceil(chartDimensions.width));
    const int maxY = static_cast<int>(ceil(chartDimensions.height));

    const int startTimeSeconds = static_cast<int>(startTimestamp / 1000LL);

    const XAxisLayout xLayout = LayoutUtils::calculateXAxisLayout(startTimeSeconds, maxX);

    if (maxY < 20) {
        labelEveryY = 5;
        gridLineEveryY = 2;
    }

    for (int seconds = xLayout.firstTickOffset; seconds <= maxX; seconds += xLayout.tickIntervalSeconds) {
        if (seconds < 0) continue;

        const float scaledX = static_cast<float>(seconds) / chartDimensions.width * chartSize.width;

        const auto tickSprite = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        tickSprite->setPositionX(scaledX);
        tickSprite->setRotation(90);
        tickSprite->setColor({labelLineColor.r, labelLineColor.g, labelLineColor.b});
        tickSprite->setOpacity(labelLineColor.a);
        labelsNode->addChild(tickSprite);

        const int absoluteTimeSeconds = startTimeSeconds + seconds;

        if ((seconds - xLayout.firstLabelOffset) % xLayout.labelIntervalSeconds == 0 && seconds >= xLayout.firstLabelOffset) {
            tickSprite->setScaleX(0.2f);
            tickSprite->setScaleX(0.2f);

            std::string labelText = TimeUtils::timestampToDate(static_cast<long long>(absoluteTimeSeconds) * 1000LL);

            const auto label = CCLabelBMFont::create(labelText.c_str(), "chatFont.fnt");
            label->setPositionX(scaledX);
            label->setScale(0.4f);
            label->setPositionY(-tickSprite->getScaledContentSize().width - label->getScaledContentSize().height);
            label->setColor({labelColor.r, labelColor.g, labelColor.b});
            label->setOpacity(labelColor.a);
            labelsNode->addChild(label);
        } else {
            tickSprite->setScaleX(0.1f);
            tickSprite->setScaleY(0.8f);
        }
        tickSprite->setPositionY(-tickSprite->getScaledContentSize().width);
    }

    for (int i = 0; i <= maxY; i++) {
        const float scaledY = static_cast<float>(i) / chartDimensions.height * chartSize.height;

        const auto tickSprite = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        tickSprite->setPositionY(scaledY);
        tickSprite->setColor({labelLineColor.r, labelLineColor.g, labelLineColor.b});
        tickSprite->setOpacity(labelLineColor.a);
        labelsNode->addChild(tickSprite);

        if (i % labelEveryY == 0) {
            tickSprite->setScaleX(0.2f);

            const auto labelText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelText->setPositionY(scaledY);
            labelText->setScale(0.4f);
            labelText->setPositionX(-tickSprite->getScaledContentSize().width - 6.0f);
            labelText->setAnchorPoint({1.0f, 0.5f});
            labelText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelText->setOpacity(labelColor.a);
            labelsNode->addChild(labelText);
        } else {
            tickSprite->setScaleX(0.1f);
            tickSprite->setScaleY(0.8f);
        }

        tickSprite->setPositionX(-tickSprite->getScaledContentSize().width);
    }

    for (int seconds = xLayout.firstGridOffset; seconds <= maxX; seconds += xLayout.gridLineIntervalSeconds) {
        if (seconds < 0) continue;

        const float scaledX = static_cast<float>(seconds) / chartDimensions.width * chartSize.width;
        gridNode->drawSegment(
            ccp(scaledX, 0),
            ccp(scaledX, chartSize.height),
            0.2f,
            ccc4FFromccc4B(gridColor)
        );
    }

    for (int i = 0; i <= maxY; i++) {
        if (i % gridLineEveryY == 0) {
            const float scaledY = static_cast<float>(i) / chartDimensions.height * chartSize.height;
            gridNode->drawSegment(
                ccp(0, scaledY),
                ccp(chartSize.width, scaledY),
                0.2f,
                ccc4FFromccc4B(gridColor)
            );
        }
    }
}

LineChartPoint SendChartNode::scalePoint(const LineChartPoint& point) const {
    return LineChartPoint(
        point.x / chartDimensions.width * chartSize.width,
        point.y / chartDimensions.height * chartSize.height,
        point.rated
    );
}

float SendChartNode::getTimestampFromX(const float x) const {
    const float seconds = x / chartSize.width * chartDimensions.width;
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

void SendChartNode::draw() {
    CCNode::draw();
    graphLineNode->clear();

    constexpr ccColor3B sendColor = {0, 255, 0};
    constexpr ccColor3B rateColor = {212, 175, 55};

    const ccColor4F sendColorF = ccc4FFromccc3B(sendColor);
    const ccColor4F rateColorF = ccc4FFromccc3B(rateColor);

    if (processedPoints.size() < 2) return;

    switch (chartStyle) {
        case LineChartStyle_Line:
            for (size_t i = 1; i < processedPoints.size(); i++) {
                ccColor4F color = sendColorF;
                if (processedPoints[i - 1].rated) color = rateColorF;

                graphLineNode->drawSegment(processedPoints[i - 1].toCCPoint(), processedPoints[i].toCCPoint(), lineWidth, color);
            }
            break;
        case LineChartStyle_Step:
            for (size_t i = 1; i < processedPoints.size(); i++) {
                ccColor4F color = sendColorF;
                if (processedPoints[i - 1].rated) color = rateColorF;

                CCPoint prevPoint = processedPoints[i - 1].toCCPoint();
                CCPoint currPoint = processedPoints[i].toCCPoint();
                CCPoint stepPoint = ccp(currPoint.x, prevPoint.y);

                graphLineNode->drawSegment(prevPoint, stepPoint, lineWidth, color);
                graphLineNode->drawSegment(stepPoint, currPoint, lineWidth, color);
            }
            break;
    }
}