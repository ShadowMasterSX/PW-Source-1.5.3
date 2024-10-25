#ifndef __ONLINEGAME_GS_DPSRANKMANAGER_H__
#define __ONLINEGAME_GS_DPSRANKMANAGER_H__


#include <hashmap.h>
#include <map>

#include "actobject.h"
#include "playertemplate.h"

class dps_rank_manager
{
public:
	enum
	{
		DPS_RANK_SIZE = 100,
		DPH_RANK_SIZE = 100,
		CLS_DPS_RANK_SIZE = 10,
		CLS_DPH_RANK_SIZE = 10,
	};

	enum
	{
		STATE_NORMAL = 0x00,
		STATE_ANNOUNCE = 0x01,
	};
	
	struct player_rank_info
	{
		int roleid;
		int level;
		int cls;
		int dps;
		int dph;
		player_rank_info():roleid(0),level(0),cls(-1),dps(0),dph(0){}
		player_rank_info(int _roleid,int _level,int _cls,int _dps,int _dph):roleid(_roleid),level(_level),cls(_cls),dps(_dps),dph(_dph){}
	};
	typedef abase::hash_map<int/*roleid*/, struct player_rank_info*, abase::_hash_function,abase::fast_alloc<> > MAP;
	typedef std::multimap<int/*dps,dph*/, struct player_rank_info*> RANK;
	
	enum
	{
		DPH_ALL_RANK = 0x00,
		DPH_CLS_RANK = 0x01,
		DPS_ALL_RANK = 0x10,
		DPS_CLS_RANK = 0x11,
	};
	
	struct log_entry
	{
		char mask;
		char cls;
		short rank;
		int roleid;
		int dps;
		int dph;
	};
	typedef abase::vector< log_entry, abase::fast_alloc<> > LOG_ENTRY_LIST;
	
public:
	dps_rank_manager():_initialized(false),_lock(0),_changed(false), _tick_counter(0), _write_tickcnt(0), _last_announce_hour(0), _state(STATE_NORMAL) { }	
	~dps_rank_manager()
	{
		Clear();
	}
	bool Initialize();
	bool DBLoad(archive & ar);
	bool DBSave(archive & ar);

	bool UpdateRankInfo(int roleid, int level, int cls, int dps, int dph);
	bool SendRank(int link_id, int roleid, int link_sid, unsigned char rank_mask);
	void RunTick();
	
	//debug模式为了调整时间用，除了playercmd.cpp外，不应有其他地方调用
	static void SetTimeAdjust(int val) { time_adjust = val; }
private:
	void InitAdd(int roleid, int level, int cls, int dps, int dph);
	void SortRank();
	int NextSortTime();
	void BroadcastResult(int roleid, int level, int dps, int dph, char rank);
	void Clear()
	{
		_dps_rank.clear();
		_dph_rank.clear();

		for(int i = 0; i < USER_CLASS_COUNT; ++i) {
			_cls_dps_rank[i].clear();
			_cls_dph_rank[i].clear();
		}
		
		for(MAP::iterator it=_map.begin(); it!=_map.end(); ++it) {
			if(it->second) {
				delete it->second;
				it->second = NULL;
			}
		}
		_map.clear();

		_changed = false;
	}
	
	/**
	 * 保存排行榜信息
	 * @param ar 保存排行榜信息的buffer
	 * @param rank 待保存的排行榜
	 */
	void SaveRankData(archive& ar, const RANK& rank);

	/**
	 * 更新玩家在某一排行榜中的位置
	 * @param pInfo 玩家的信息
	 * @param attack_value 攻击力成绩(dps或dph)
	 * @param rank 要更新的排行榜
	 * @param rank_capacity 该排行榜最大的容量
	 */
	void UpdatePlayerRank(player_rank_info* pInfo, int attack_value, RANK& rank, unsigned int rank_capacity);
	
	/**
	 * 创建某排行榜将要输出log的信息
	 * @param mask 排行榜类别
	 * @param rank 排行榜信息
	 * @param rank_capacity 该排行榜最大的容量
	 */
	void MakeRankLogEntry(char mask, const RANK& rank, int rank_capacity);

	/**
	 * 初始化待写log的信息
	 */
	void InitLogEntryList();
	void OutputRankLog();
	void DoSendRankData(int link_id, int roleid, int link_sid, const RANK& rank, unsigned char rank_mask);

private:
	bool _initialized;
	
	int _lock;

	bool _changed;
	MAP _map;
	RANK _dps_rank; //dps总榜
	RANK _dph_rank; //dph总榜
	RANK _cls_dps_rank[USER_CLASS_COUNT]; //dps职业分类榜
	RANK _cls_dph_rank[USER_CLASS_COUNT]; //dph职业分类榜
	LOG_ENTRY_LIST _log_entry_list; //待写log的信息
	
	int _tick_counter;
	int _write_tickcnt;
	int _last_announce_hour;
	int _state;
	static int time_adjust; //debug模式为了调整时间用
};

#endif
