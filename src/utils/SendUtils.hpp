#ifndef SENDDB_SENDUTILS_HPP
#define SENDDB_SENDUTILS_HPP

#include <vector>

struct PeakTrendingScore {
    double score;
    int timestamp;
};

class SendUtils {
public:
    static double calculateTrendingScore(int timestamp, const std::vector<int>& sends);
    static PeakTrendingScore calculatePeakTrendingScore(const std::vector<int>& sends);

    static double calculateIndividualTrendingScore(int timestamp, int sendTimestamp);
};

#endif