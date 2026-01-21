#ifndef SENDDB_MESSAGES_HPP
#define SENDDB_MESSAGES_HPP

enum class LevelSendPopupInfo {
    Info,

    SendCategory,
    SendCount,
    SendDifference,
    SendCreatorAverage,

    TrendingCategory,
    TrendingNotEligible,
    TrendingScore,
    TrendingPeakScore,
    TrendingCreatorScore,
    TrendingContribution,

    RankingAll,
    RankingCreator,
    RankingTrending,
    RankingRate,
    RankingGamemode,
    RankingJoined
};


class Messages {
public:
    static std::string getSendPopupInfoTitle(const LevelSendPopupInfo e) {
        switch (e) {
            case LevelSendPopupInfo::Info: return "Confused?";

            case LevelSendPopupInfo::SendCategory: return "Sends";
            case LevelSendPopupInfo::SendCount: return "Total Sends";
            case LevelSendPopupInfo::SendDifference: return "Send Difference";
            case LevelSendPopupInfo::SendCreatorAverage: return "Average Sends";

            case LevelSendPopupInfo::TrendingCategory: return "Trending";
            case LevelSendPopupInfo::TrendingNotEligible: return "Not Eligible";
            case LevelSendPopupInfo::TrendingScore: return "Trending Score";
            case LevelSendPopupInfo::TrendingPeakScore: return "Peak Trending Score";
            case LevelSendPopupInfo::TrendingCreatorScore: return "Creator Trending Score";
            case LevelSendPopupInfo::TrendingContribution: return "Score Contribution";

            case LevelSendPopupInfo::RankingAll: return "Global Ranking";
            case LevelSendPopupInfo::RankingCreator: return "Creator Ranking";
            case LevelSendPopupInfo::RankingTrending: return "Trending Ranking";
            case LevelSendPopupInfo::RankingRate: return "Rate Ranking";
            case LevelSendPopupInfo::RankingGamemode: return "Gamemode Ranking";
            case LevelSendPopupInfo::RankingJoined: return "Joined Ranking";

            default: return "unknown";
        }
    }

    static std::string getSendPopupInfoContent(const LevelSendPopupInfo e) {
        switch (e) {
            case LevelSendPopupInfo::Info: return "Don't understand something? Click on it to get more information.";

            case LevelSendPopupInfo::SendCategory: return "View information about the <cy>level's send count</c>.";
            case LevelSendPopupInfo::SendCount: return "The <cg>total number of sends</c> the level has received.";
            case LevelSendPopupInfo::SendDifference: return "The <cr>difference</c> between this level's send count and the creator's levels' average send count.";
            case LevelSendPopupInfo::SendCreatorAverage: return "The creator's levels' <cy>average send count</c>.";

            case LevelSendPopupInfo::TrendingCategory: return "View information about the level's <cp>trending score</c>. The trending score is calculated based on how many sends the level has received recently, with more recent sends contributing more to the score.";
            case LevelSendPopupInfo::TrendingNotEligible: return "This level is <cr>not eligible</c> for trending score ranking because it has been rated. You can still see its trending score, but it does not have a ranking amongst unrated levels.";
            case LevelSendPopupInfo::TrendingScore: return "The level's <cp>live trending score</c>.";
            case LevelSendPopupInfo::TrendingPeakScore: return "The level's <cy>peak trending score</c> (and when it was achieved).";
            case LevelSendPopupInfo::TrendingCreatorScore: return "The sum of the <cg>creator's levels' trending scores.</c>";
            case LevelSendPopupInfo::TrendingContribution: return "The <cg>percentage contribution</c> of this level's trending score to the creator's total trending score.";

            case LevelSendPopupInfo::RankingAll: return "The level's send count ranking amongst <cy>all levels</c>.";
            case LevelSendPopupInfo::RankingCreator: return "The level's send count ranking amongst the <cg>creator's levels</c>.";
            case LevelSendPopupInfo::RankingTrending: return "The level's trending score ranking amongst <cp>unrated levels</c>.";
            case LevelSendPopupInfo::RankingRate: return "The level's send count ranking amongst <cy>levels with the same rate</c> (rated/unrated).";
            case LevelSendPopupInfo::RankingGamemode: return "The level's send count ranking amongst <cy>levels with the same gamemode</c> (classic/platformer).";
            case LevelSendPopupInfo::RankingJoined: return "The level's send count ranking amongst <cr>levels with the same rate</c> (rated/unrated) <cr>and gamemode</c> (classic/platformer).";

            default: return "unknown";
        }
    }
};

#endif