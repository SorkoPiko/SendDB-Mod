#pragma once

struct ChartPoint {
    int interval;
    int startOffset;
};

struct ChartAxisLayout {
    float unit;
    float startOffset;
    ChartPoint tick;
    ChartPoint label;
    ChartPoint gridLine;
};

class LayoutUtils {
public:
    static ChartAxisLayout calculateTimeAxisLayout(int minTimestamp, int timeRange);
    static ChartAxisLayout calculateNumericAxisLayout(float minValue, float maxValue);

    static int minStartOffset(ChartAxisLayout layout);
};