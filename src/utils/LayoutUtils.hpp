#ifndef SENDDB_LAYOUTUTILS_HPP
#define SENDDB_LAYOUTUTILS_HPP

struct XAxisLayout {
    int tickIntervalSeconds;
    int labelIntervalSeconds;
    int gridLineIntervalSeconds;
    int firstTickOffset;
    int firstLabelOffset;
    int firstGridOffset;
};

class LayoutUtils {
public:
    static XAxisLayout calculateXAxisLayout(int startTimestamp, int timeRange);
};

#endif