#pragma once

#include <vector>

struct PeakTrendingScore {
    double score;
    int timestamp;
};

class SendUtils {
public:
    static double calculateTrendingScore(long long timestamp, const std::vector<int>& sends);
    static PeakTrendingScore calculatePeakTrendingScore(const std::vector<int>& sends);

    static double calculateIndividualTrendingScore(long long timestamp, int sendTimestamp);
};