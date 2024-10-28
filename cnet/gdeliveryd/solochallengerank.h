#ifndef __GNET_SOLOCHALLENGERANK_H
#define __GNET_SOLOCHALLENGERANK_H

#include <map>
#include <vector>

#include <time.h>
#include <itimer.h>

#include "localmacro.h"

#include "solochallengerankdata"
#include "solochallengerankdataext"


namespace GNET
{

class CrossSoloChallengeRank;
class CrossSoloChallengeRank_Re;

class SoloChallengeRank : public IntervalTimer::Observer
{
public:
    enum
    {
        SOLO_CHALLENGE_RANK_TOTAL_SIZE = 200,
        SOLO_CHALLENGE_RANK_CLS_SIZE = 100,

        SOLO_CHALLENGE_RANK_MAX_SIZE = SOLO_CHALLENGE_RANK_CLS_SIZE * USER_CLASS_COUNT + SOLO_CHALLENGE_RANK_TOTAL_SIZE,

    };

    typedef std::vector<SoloChallengeRankData> SCRVector;
    typedef std::multimap<int /* total_time */, SoloChallengeRankData> RankMap;
    typedef bool (*UPDATEFUNC)(RankMap& rank, unsigned int size, const SoloChallengeRankData& info);

public:
    SoloChallengeRank() : initialized(false), g_check_times(0), last_check_time(-1), update_time(0), adjust_time(0)
    {
        ClearRank();
        ClearRankGlobal();
    }

    ~SoloChallengeRank()
    {
        ClearRank();
        ClearRankGlobal();
    }

    bool Update();
    bool Initialize();
    static SoloChallengeRank& GetInstance() { return instance; }

    void OnDBConnect(Protocol::Manager* manager, int sid);
    void OnDBLoad(const SoloChallengeRankDataExt& data_ext_local, const SoloChallengeRankDataExt& data_ext_global);

    bool UpdateRankInfo(int roleid, int level, int cls, const Octets& name, int total_time);
    bool GetRankInfo(int ranktype, int cls, int& next_sort_time, SCRVector& data);
    void SetAdjustTime(int time);
   
public:
    void OnSaveLocalRank(CrossSoloChallengeRank& msg); // 取普通服排行快照(普通服调用)
    void OnLoadLocalRank(const CrossSoloChallengeRank& msg); // 插入普通服排行榜数据(中央服调用)
    void OnSaveGlobalRank(CrossSoloChallengeRank& msg);// 取中央服排行快照(中央服调用)
    void OnLoadGlobalRank(const CrossSoloChallengeRank& msg);// 刷新全服排行快照(普通服调用)

private:
    void SortRank();
    void ClearRank();

    void SortRankGlobal();
    void ClearRankGlobal();

    time_t GetTime();
    int GetNextSortTime();
    bool CheckClearRank(struct tm* cur_tm);

private:
    RankMap map_total, map_cls[USER_CLASS_COUNT];
    SCRVector rank_total, rank_cls[USER_CLASS_COUNT];

    RankMap g_map_total, g_map_cls[USER_CLASS_COUNT];
    SCRVector g_rank_total, g_rank_cls[USER_CLASS_COUNT];

    static SoloChallengeRank instance;
    bool initialized;
    bool changed;

    int g_check_times;
    bool g_changed;

    int last_check_time;
    int update_time;
    int adjust_time;    // for debug only
};

};

#endif

