#ifndef SENDDB_LAYOUTUTILS_HPP
#define SENDDB_LAYOUTUTILS_HPP

struct ChartPoint {
    int interval;
    int startOffset;
};

struct ChartAxisLayout {
    float unit;
    ChartPoint tick;
    ChartPoint label;
    ChartPoint gridLine;
};

class LayoutUtils {
public:
    static ChartAxisLayout calculateTimeAxisLayout(int startTimestamp, int timeRange);

    static int minStartOffset(ChartAxisLayout layout);
};

#endif