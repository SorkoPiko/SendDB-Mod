#include "SendUtils.hpp"

#include <algorithm>
#include <cmath>

double SendUtils::calculateTrendingScore(const long long timestamp, const std::vector<int>& sends) {
    double score = 0.0;
    for (const int send : sends) {
        score += calculateIndividualTrendingScore(timestamp, send);
    }
    return score;
}

PeakTrendingScore SendUtils::calculatePeakTrendingScore(const std::vector<int>& sends) {
    double peakScore = 0.0;
    int peakTimestamp = 0;

    std::vector<int> sendsCopy = sends;
    std::ranges::sort(sendsCopy, std::less<int>());

    for (const int timestamp : sendsCopy) {
        const double score = calculateTrendingScore(timestamp * 1000LL, sendsCopy);
        if (score > peakScore) {
            peakScore = score;
            peakTimestamp = timestamp;
        }
    }

    return {peakScore, peakTimestamp};
}

double SendUtils::calculateIndividualTrendingScore(const long long timestamp, const int sendTimestamp) {
    const double ageInDays = (timestamp - sendTimestamp * 1000LL) / 86400000.0;
    if (ageInDays < 0.0 || ageInDays > 30.0) return 0.0;
    return 25000.0 / std::pow(ageInDays + 2, 1);
}
