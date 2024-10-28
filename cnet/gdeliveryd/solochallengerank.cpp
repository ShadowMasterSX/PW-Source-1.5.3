
#include "dbsolochallengerankload.hrp"
#include "dbsolochallengeranksave.hrp"

#include <utility>

#include "solochallengerank.h"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"
#include "worldchat.hpp"
#include "maplinkserver.h"
#include "crosssolochallengerank.hpp"


#define SOLOCHALLENGERANK_UPDATE_INTERVAL 60    // 每60秒检查一次
#define SOLOCHALLENGERANK_LOCK_DAY 3            // 每周三早上6点封榜 7点清榜 8点例行维护 9点开榜
#define SOLOCHALLENGERANK_LOCK_TIME 6
#define SOLOCHALLENGERANK_CLEAR_TIME 7
#define SOLOCHALLENGERANK_UNLOCK_TIME 9
#define SOLOCHALLENGERANK_TOP_NUM 10


namespace GNET
{

SoloChallengeRank SoloChallengeRank::instance;


void LoadData(const SoloChallengeRankDataExt& data_ext, SoloChallengeRank::RankMap& m_total, SoloChallengeRank::RankMap* m_cls, SoloChallengeRank::UPDATEFUNC UpdateFunc)
{
    if ((data_ext.data.size() == 0) || (m_cls == NULL) || (UpdateFunc == NULL)) return;

    SoloChallengeRankDataVector::const_iterator iter = data_ext.data.begin(), iter_end = data_ext.data.end();

    for (; iter != iter_end; ++iter)
    {
        int cls = iter->cls;
        if ((cls < 0) || (cls >= USER_CLASS_COUNT)) continue;

        if (iter->type == 0)
            UpdateFunc(m_total, SoloChallengeRank::SOLO_CHALLENGE_RANK_TOTAL_SIZE, *iter);
        else if (iter->type == 1)
            UpdateFunc(m_cls[cls], SoloChallengeRank::SOLO_CHALLENGE_RANK_CLS_SIZE, *iter);
    }
}

void SaveData(const SoloChallengeRank::SCRVector& r_total, const SoloChallengeRank::SCRVector* r_cls, SoloChallengeRank::SCRVector& rank_db)
{
    if ((r_total.size() == 0) || (r_cls == NULL)) return;

    rank_db.reserve(SoloChallengeRank::SOLO_CHALLENGE_RANK_MAX_SIZE);
    rank_db.insert(rank_db.begin(), r_total.begin(), r_total.end());

    for (int i = 0; i < USER_CLASS_COUNT; ++i)
    {
        rank_db.insert(rank_db.end(), r_cls[i].begin(), r_cls[i].end());
    }
}


int GetRank(const SoloChallengeRank::RankMap& rank, int roleid)
{
    int index = 1;
    SoloChallengeRank::RankMap::const_iterator iter = rank.begin(), iter_end = rank.end();

    for (; iter != iter_end; ++iter, ++index)
    {
        if (iter->second.roleid == roleid) break;
    }

    if (iter == iter_end) index = 0;
    return index;
}

bool UpdateRank(SoloChallengeRank::RankMap& rank, unsigned int size, const SoloChallengeRankData& info)
{
    if (rank.size() < size)
    {
        rank.insert(std::make_pair(info.total_time, info));

        if (info.type == 0)
        {
            Log::log(LOG_INFO, "SoloChallengeRank::UpdateRank(), update1, roleid=%d, level=%d, cls=%d, total_time=%d, name_size=%d, update_time=%d, rank_size=%d.",
                info.roleid, info.level, info.cls, info.total_time, info.name.size(), info.update_time, rank.size());
        }

        return true;
    }
    else
    {
        SoloChallengeRank::RankMap::iterator iter = --rank.end();
        int last_one_total_time = iter->first;

        if (info.total_time < last_one_total_time)
        {
            rank.erase(iter);
            rank.insert(std::make_pair(info.total_time, info));

            if (info.type == 0)
            {
                Log::log(LOG_INFO, "SoloChallengeRank::UpdateRank(), update2, roleid=%d, level=%d, cls=%d, total_time=%d, name_size=%d, update_time=%d, last_one_total_time=%d, rank_size=%d.",
                    info.roleid, info.level, info.cls, info.total_time, info.name.size(), info.update_time, last_one_total_time, rank.size());
            }

            return true;
        }
        else
        {
            if (info.type == 0)
            {
                Log::log(LOG_INFO, "SoloChallengeRank::UpdateRank(), update3, roleid=%d, level=%d, cls=%d, total_time=%d, name_size=%d, update_time=%d, last_one_total_time=%d, rank.size=%d.",
                    info.roleid, info.level, info.cls, info.total_time, info.name.size(), info.update_time, last_one_total_time, rank.size());
            }

            return false;
        }
    }
}

SoloChallengeRank::RankMap::iterator GetRankGlobal(SoloChallengeRank::RankMap& rank, int roleid, unsigned char zoneid)
{
    SoloChallengeRank::RankMap::iterator iter = rank.begin(), iter_end = rank.end();
    for (; iter != iter_end; ++iter)
    {
        if ((iter->second.roleid == roleid) && (iter->second.zoneid == zoneid)) break;
    }

    return iter;
}

bool UpdateRankGlobal(SoloChallengeRank::RankMap& rank, unsigned int size, const SoloChallengeRankData& info)
{
    SoloChallengeRank::RankMap::iterator iter = GetRankGlobal(rank, info.roleid, info.zoneid);
    if (iter != rank.end())
    {
        if (info.total_time < iter->second.total_time)
        {
            rank.erase(iter);
            rank.insert(std::make_pair(info.total_time, info));
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return UpdateRank(rank, size, info);
    }
}


bool SoloChallengeRank::Update()
{
    if (!initialized) return true;
    if (GDeliveryServer::GetInstance()->IsCentralDS())
    {       
        if (++g_check_times < 60) return true;
        g_check_times = 0;

        SortRankGlobal();
        if (g_rank_total.empty()) return true;

        SoloChallengeRankDataExt data_ext;
        data_ext.update_time = GetTime();
        data_ext.zoneid = GDeliveryServer::GetInstance()->GetZoneid();

        SCRVector& rank_db = data_ext.data.GetVector();
        SaveData(g_rank_total, g_rank_cls, rank_db);

        GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBSOLOCHALLENGERANKSAVE, DBSoloChallengeRankSaveArg(1, data_ext)));
        Log::log(LOG_INFO, "SoloChallengeRank::Update(), save global solochallengerank, save_time=%d, rank_db_size=%d.",
            data_ext.update_time, data_ext.data.size());

        return true;
    }

    time_t cur_time = GetTime();
    struct tm* cur_tm = localtime(&cur_time);

    // 每小时第一次检查时写表
    int check_time = cur_tm->tm_hour;
    if (check_time != last_check_time)
    {
        last_check_time = check_time;
        SoloChallengeRankDataExt data_ext;

        if (CheckClearRank(cur_tm))
        {
            update_time = cur_time;
            data_ext.update_time = update_time;
            data_ext.zoneid = GDeliveryServer::GetInstance()->GetZoneid();

            GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBSOLOCHALLENGERANKSAVE, DBSoloChallengeRankSaveArg(0, data_ext)));
            Log::log(LOG_INFO, "SoloChallengeRank::Update(), clear local solochallengerank, clear_time=%d.",
                data_ext.update_time);

            ClearRank();
            return true;
        }

        if (!changed) return true;

        SortRank();
        if (!rank_total.empty())
        {
            update_time = cur_time;
            data_ext.update_time = update_time;
            data_ext.zoneid = GDeliveryServer::GetInstance()->GetZoneid();

            SCRVector& rank_db = data_ext.data.GetVector();
            SaveData(rank_total, rank_cls, rank_db);

            GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBSOLOCHALLENGERANKSAVE, DBSoloChallengeRankSaveArg(0, data_ext)));
            Log::log(LOG_INFO, "SoloChallengeRank::Update(), save local solochallengerank, save_time=%d, rank_db_size=%d.",
                data_ext.update_time, data_ext.data.size());
        }
    }

    return true;
}

bool SoloChallengeRank::Initialize()
{
    IntervalTimer::AddTimer(this, SOLOCHALLENGERANK_UPDATE_INTERVAL);    // second
    return true;
}

void SoloChallengeRank::OnDBConnect(Protocol::Manager* manager, int sid)
{
    if (initialized) return;
    manager->Send(sid, Rpc::Call(RPC_DBSOLOCHALLENGERANKLOAD, DBSoloChallengeRankLoadArg()));
}

void SoloChallengeRank::OnDBLoad(const SoloChallengeRankDataExt& data_ext_local, const SoloChallengeRankDataExt& data_ext_global)
{
    if (initialized) return;
    initialized = true;
    time_t cur_time = GetTime();

    if (GDeliveryServer::GetInstance()->IsCentralDS())
    {
        if (data_ext_global.data.size() > 0)
        {
            LoadData(data_ext_global, g_map_total, g_map_cls, UpdateRank);
            g_changed = true;
            SortRankGlobal();
        }
        update_time = data_ext_global.update_time;

        Log::log(LOG_INFO, "SoloChallengeRank::OnDBLoad(), load global solochallengerank, load_time=%d, update_time=%d, rank_db_size=%d.",
            cur_time, data_ext_global.update_time, data_ext_global.data.size());
    }
    else
    {
        if (data_ext_local.data.size() > 0)
        {
            LoadData(data_ext_local, map_total, map_cls, UpdateRank);
            changed = true;
            SortRank();
        }
        update_time = data_ext_local.update_time;

        Log::log(LOG_INFO, "SoloChallengeRank::OnDBLoad(), load local solochallengerank, load_time=%d, update_time=%d, rank_db_size=%d.",
            cur_time, data_ext_local.update_time, data_ext_local.data.size());
    }
}

bool SoloChallengeRank::UpdateRankInfo(int roleid, int level, int cls, const Octets& name, int total_time)
{
    if (!initialized) return false;
    if (GDeliveryServer::GetInstance()->IsCentralDS()) return false;
    if ((cls < 0) || (cls >= USER_CLASS_COUNT) || (total_time <= 0)) return false;

    time_t cur_time = GetTime();
    struct tm* cur_tm = localtime(&cur_time);

    if ((cur_tm->tm_wday == SOLOCHALLENGERANK_LOCK_DAY) &&
        (cur_tm->tm_hour >= SOLOCHALLENGERANK_LOCK_TIME) &&
        (cur_tm->tm_hour < SOLOCHALLENGERANK_UNLOCK_TIME))
    {
        Log::log(LOG_INFO, "SoloChallengeRank::UpdateRankInfo(), rank closed, roleid=%d, level=%d, cls=%d, total_time=%d, name_size=%d, update_time=%d.",
            roleid, level, cls, total_time, name.size(), cur_time);

        return false;
    }

    SoloChallengeRankData info(roleid, level, cls, total_time, name);
    info.zoneid = GDeliveryServer::GetInstance()->GetZoneid();
    info.update_time = cur_time;

    if (GetRank(map_total, roleid) == 0)
    {
        info.type = 0;
        changed = UpdateRank(map_total, SOLO_CHALLENGE_RANK_TOTAL_SIZE, info);
    }

    if (GetRank(map_cls[cls], roleid) == 0)
    {
        info.type = 1;
        changed = UpdateRank(map_cls[cls], SOLO_CHALLENGE_RANK_CLS_SIZE, info);
    }

    if (map_total.size() <= SOLOCHALLENGERANK_TOP_NUM)
    {
        WorldChat chat;
        chat.channel = GN_CHAT_CHANNEL_SYSTEM;
        chat.emotion = 0;

        // 105 - gs/config.h : SOLO_CHALLENGE_RANK_CHAT_MSG_ID
        chat.roleid = 105;
        chat.name = Octets();
        chat.msg = name;
        chat.data = Octets();

        LinkServer::GetInstance().BroadcastProtocol(&chat);
    }

    return changed;
}

bool SoloChallengeRank::GetRankInfo(int ranktype, int cls, int& next_sort_time, SCRVector& data)
{
    if (!initialized) return false;
    if (GDeliveryServer::GetInstance()->IsCentralDS()) return false;
    if ((cls < -1) || (cls >= USER_CLASS_COUNT)) return false;

    next_sort_time = GetNextSortTime();

    if (ranktype == 0)
    {
        if (cls == -1)
            data = rank_total;
        else
            data = rank_cls[cls];
    }
    else if (ranktype == 1)
    {
        if (cls == -1)
            data = g_rank_total;
        else
            data = g_rank_cls[cls];
    }

    return true;
}

void SoloChallengeRank::SetAdjustTime(int time)
{
    if (GDeliveryServer::GetInstance()->IsCentralDS()) return;
    adjust_time = time;
}


void SoloChallengeRank::SortRank()
{
    if (!changed || map_total.empty()) return;

    rank_total.clear();
    rank_total.reserve(SOLO_CHALLENGE_RANK_TOTAL_SIZE);

    for (int i = 0; i < USER_CLASS_COUNT; ++i)
    {
        rank_cls[i].clear();
        rank_cls[i].reserve(SOLO_CHALLENGE_RANK_CLS_SIZE);
    }

    RankMap::const_iterator iter = map_total.begin(), iter_end = map_total.end();
    for (; iter != iter_end; ++iter)
    {
        rank_total.push_back(iter->second);
    }

    for (int i = 0; i < USER_CLASS_COUNT; ++i)
    {
        RankMap::const_iterator iter = map_cls[i].begin(), iter_end = map_cls[i].end();
        for (; iter != iter_end; ++iter)
        {
            rank_cls[i].push_back(iter->second);
        }
    }

    changed = false;
}

void SoloChallengeRank::ClearRank()
{
    map_total.clear();
    rank_total.clear();

    for (int i = 0; i < USER_CLASS_COUNT; ++i)
    {
        map_cls[i].clear();
        rank_cls[i].clear();
    }

    changed = false;
}


void SoloChallengeRank::SortRankGlobal()
{
    if (!g_changed || g_map_total.empty()) return;

    g_rank_total.clear();
    g_rank_total.reserve(SOLO_CHALLENGE_RANK_TOTAL_SIZE);

    for (int i = 0; i < USER_CLASS_COUNT; ++i)
    {
        g_rank_cls[i].clear();
        g_rank_cls[i].reserve(SOLO_CHALLENGE_RANK_CLS_SIZE);
    }

    RankMap::const_iterator iter = g_map_total.begin(), iter_end = g_map_total.end();
    for (; iter != iter_end; ++iter)
    {
        g_rank_total.push_back(iter->second);
    }

    for (int i = 0; i < USER_CLASS_COUNT; ++i)
    {
        RankMap::const_iterator iter = g_map_cls[i].begin(), iter_end = g_map_cls[i].end();
        for (; iter != iter_end; ++iter)
        {
            g_rank_cls[i].push_back(iter->second);
        }
    }

    g_changed = false;
}

void SoloChallengeRank::ClearRankGlobal()
{
    g_map_total.clear();
    g_rank_total.clear();

    for (int i = 0; i < USER_CLASS_COUNT; ++i)
    {
        g_map_cls[i].clear();
        g_rank_cls[i].clear();
    }

    g_changed = false;
}


time_t SoloChallengeRank::GetTime()
{
    return (Timer::GetTime() + adjust_time);
}

int SoloChallengeRank::GetNextSortTime()
{
    time_t cur_time = GetTime();
    time_t next_time = cur_time + 3600;

    struct tm* next_tm = localtime(&next_time);
    next_tm->tm_min = 0;
    next_tm->tm_sec = 0;

    int next_sort_time = mktime(next_tm) + SOLOCHALLENGERANK_UPDATE_INTERVAL;
    return (next_sort_time - cur_time);
}

bool SoloChallengeRank::CheckClearRank(struct tm* cur_tm)
{
    if (GDeliveryServer::GetInstance()->IsCentralDS()) return false;

    // 每周三早上7点第一次检查时清表
    if ((cur_tm->tm_wday == SOLOCHALLENGERANK_LOCK_DAY) &&
        (cur_tm->tm_hour == SOLOCHALLENGERANK_CLEAR_TIME))
        return true;

    // 如果错过上述检查 则按上次存盘时间检查
    int dtime = 0;
    if (cur_tm->tm_wday < SOLOCHALLENGERANK_LOCK_DAY)
    {
        dtime = 3600 * 24 * (cur_tm->tm_wday + 7 - SOLOCHALLENGERANK_LOCK_DAY);
    }
    else if (cur_tm->tm_wday == SOLOCHALLENGERANK_LOCK_DAY)
    {
        if (cur_tm->tm_hour < SOLOCHALLENGERANK_CLEAR_TIME)
            dtime = 3600 * 24 * 7;
    }
    else
    {
        dtime = 3600 * 24 * (cur_tm->tm_wday - SOLOCHALLENGERANK_LOCK_DAY);
    }

    cur_tm->tm_hour = SOLOCHALLENGERANK_CLEAR_TIME;
    cur_tm->tm_min = 0;
    cur_tm->tm_sec = 0;

    int last_update_time = mktime(cur_tm) - dtime;
    return ((update_time > 0) && (update_time < last_update_time));
}


// 取普通服排行快照(普通服调用)
// 普通服调用 - 将普通服排行榜打包成消息 发送给中央服
void SoloChallengeRank::OnSaveLocalRank(CrossSoloChallengeRank& msg)
{
    if (GDeliveryServer::GetInstance()->IsCentralDS()) return;

    SortRank();
    changed = true;

    SCRVector& rank_db = msg.data_ext.data.GetVector();
    SaveData(rank_total, rank_cls, rank_db);

    msg.data_ext.update_time = GetTime();
	msg.data_ext.zoneid = GDeliveryServer::GetInstance()->GetZoneid();

    Log::log(LOG_INFO, "SoloChallengeRank::OnSaveLocalRank(), pack local solochallengerank, pack_time=%d, rank_db_size=%d.",
        msg.data_ext.update_time, rank_db.size());
}

// 插入普通服排行榜数据(中央服调用)
// 中央服调用 - 将普通服排行榜插入到中央服排行榜 即更新中央服的跨服排行榜
void SoloChallengeRank::OnLoadLocalRank(const CrossSoloChallengeRank& msg)
{
    if (!GDeliveryServer::GetInstance()->IsCentralDS()) return;

    if (msg.data_ext.data.size() > 0)
    {
        LoadData(msg.data_ext, g_map_total, g_map_cls, UpdateRankGlobal);
        g_changed = true;
    }

    Log::log(LOG_INFO, "SoloChallengeRank::OnLoadLocalRank(), update global solochallengerank, update_time=%d, zone_id=%d, data_ext_data_size=%d.",
        GetTime(), msg.data_ext.zoneid, msg.data_ext.data.size());
}

// 取中央服排行快照(中央服调用)
// 中央服调用 - 将中央服排行榜打包成消息 发送给普通服
void SoloChallengeRank::OnSaveGlobalRank(CrossSoloChallengeRank& msg)
{
    if (!GDeliveryServer::GetInstance()->IsCentralDS()) return;

    SortRankGlobal();
    SCRVector& rank_db = msg.data_ext.data.GetVector();
    SaveData(g_rank_total, g_rank_cls, rank_db);

    msg.data_ext.update_time = GetTime();
	msg.data_ext.zoneid = GDeliveryServer::GetInstance()->GetZoneid();

    Log::log(LOG_INFO, "SoloChallengeRank::OnSaveGlobalRank(), pack global solochallengerank, pack_time=%d, rank_db_size=%d.",
        msg.data_ext.update_time, rank_db.size());
}

// 刷新全服排行快照(普通服调用)
// 普通服调用 - 将中央服排行榜插入到普通服排行榜 即更新普通服的跨服排行榜
void SoloChallengeRank::OnLoadGlobalRank(const CrossSoloChallengeRank& msg)
{
    if (GDeliveryServer::GetInstance()->IsCentralDS()) return;

    if (msg.data_ext.data.size() > 0)
    {
        ClearRankGlobal();
        LoadData(msg.data_ext, g_map_total, g_map_cls, UpdateRank);
        g_changed = true;
        SortRankGlobal();
    }

    Log::log(LOG_INFO, "SoloChallengeRank::OnLoadGlobalRank(), update local solochallengerank, update_time=%d, zone_id=%d, data_ext_data_size=%d.",
        GetTime(), msg.data_ext.zoneid, msg.data_ext.data.size());
}


};

