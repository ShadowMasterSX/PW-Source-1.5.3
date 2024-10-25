#ifndef __ONLINEGAME_GS_COUNTRYBATTLE_CTRL_H__
#define __ONLINEGAME_GS_COUNTRYBATTLE_CTRL_H__

class countrybattle_ctrl : public world_data_ctrl
{
public:
	struct
	{
		int battle_id;
		int country_attacker;
		int country_defender;
		int attacker_count;
		int defender_count;
		int player_count_limit;
		int end_timestamp;
		int attacker_total;
		int defender_total;
		int max_total;
		int battle_type;
	}_data;

	enum
	{
		BR_NULL = 0,
		BR_WINNER_OFFENSE,
		BR_WINNER_DEFENCE,
	};
	struct country_score
	{
		int score;
		int goal;
		int kill_count;
		int death_count;
	};
	struct personal_score
	{
		int cls;
		int soulpower;
		int combat_time;
		int attend_time;
		int kill_count;
		int death_count;
		int score;
		int dmg_output;
		int dmg_output_weighted;
		int dmg_endure;
		int dmg_output_npc;
		int kill_count_weighted;
	};
	typedef abase::hash_map<int, personal_score, abase::_hash_function, abase::fast_alloc<> > PSCORE_MAP;

	enum
	{
		RANK_MAX_SIZE = 5,
		RANK_POS_INTERVAL = 5,
		DEATH_MAX_INTERVAL = 20,
	};
	struct score_rank_entry
	{
		int roleid;
		int rank_val;
		A3DVECTOR pos;
	};
	typedef abase::vector< score_rank_entry, abase::fast_alloc<> > SCORE_RANK_LIST;
	struct score_rank_info
	{
		SCORE_RANK_LIST ranks;
		int timestamp;
	};
	struct death_entry
	{
		int roleid;
		int timestamp;
		A3DVECTOR pos;
	};
	typedef abase::vector< death_entry, abase::fast_alloc<> > DEATH_INFO_LIST;
	
	int _lock;
	int _tick_counter;
	int _battle_result;
	int _battle_end_timer;
	bool _need_send_ds;
	//夺旗模式
	enum
	{
		STAGE_PREPARE = 0,		//准备阶段
		STAGE_CONTEST,			//争夺阶段
		STAGE_CONVOY,			//护送阶段
	};
	enum
	{
		PREPARE_TIME = 45,
		FLAG_CARRIER_CHECK_INTERVAL = 4,
	};
	enum
	{
		FLAG_MSG_GENERATE,
		FLAG_MSG_PICKUP,
		FLAG_MSG_HANDIN,
		FLAG_MSG_LOST,
	};	
	int _stage;
	int _timeout;
	int _flag_controller;
	struct
	{
		XID id;
		A3DVECTOR pos;
		bool offense;
		char state;
	}_flag_carrier;

	//摧毁防御塔模式
	abase::vector<char> _attacker_available_tower;
	abase::vector<char> _defender_available_tower;
	bool _tower_generated;

	//据点模式
	enum
	{
		SH_FULL_OCCUPY_TIME = 30,	
		SH_OCCUPY_GAIN_INTERVAL = 10,
	};	
	abase::vector<abase::pair<int, int> > _stronghold_state;	//stronghold state, counter
	bool _stronghold_init;

	country_score _defence_score;
	country_score _offense_score;
	int _lock_defence_map;
	int _lock_offense_map;
	PSCORE_MAP _defence_pscore_map;
	PSCORE_MAP _offense_pscore_map;

	int _tick_rank_info;
	score_rank_info _defence_rank_info;
	score_rank_info _offense_rank_info;
	DEATH_INFO_LIST _defence_death_info;
	DEATH_INFO_LIST _offense_death_info;

	cs_user_map  _attacker_list;
	cs_user_map  _defender_list;
	cs_user_map  _all_list;
	int _user_list_lock;
public:
	countrybattle_ctrl():_lock(0),_tick_counter(0),_battle_result(0),_battle_end_timer(0), _need_send_ds(true),
						 _stage(STAGE_PREPARE),_timeout(PREPARE_TIME),_flag_controller(0),
						 _tower_generated(false),
						 _stronghold_init(false),
						 _lock_defence_map(0),_lock_offense_map(0),
						 _tick_rank_info(0), _user_list_lock(0)
	{
		memset(&_data,0,sizeof(_data));
		memset(&_defence_score,0,sizeof(_defence_score));
		memset(&_offense_score,0,sizeof(_offense_score));
		memset(&_flag_carrier,0,sizeof(_flag_carrier));
		_flag_carrier.id = XID(-1,-1);
	}
	virtual ~countrybattle_ctrl(){}
	virtual world_data_ctrl * Clone()
	{
		return new countrybattle_ctrl(*this);
	}
	virtual void Reset()
	{
		memset(&_data,0,sizeof(_data));
	}
	void Init(int battle_type);
	virtual void Tick(world * pPlane);
	virtual void BattleFactionSay(int faction, const void * buf, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level);
	virtual void UpdatePersonalScore(bool offense, int roleid, int combat_time, int attend_time, int dmg_output, int dmg_output_weighted, int dmg_endure, int dmg_output_npc);
	virtual void OnPlayerDeath(gplayer * pPlayer, const XID & killer, int player_soulpower,const A3DVECTOR& pos);
	virtual bool PickUpFlag(gplayer * pPlayer);
	virtual bool HandInFlag(gplayer * pPlayer);
	virtual void UpdateFlagCarrier(int roleid, const A3DVECTOR & pos);
	virtual void OnTowerDestroyed(world * pPlane, bool offense, int tid);
	virtual void OccupyStrongHold(int mine_tid, gplayer* pPlayer);
	virtual bool GetStrongholdNearby(bool offense, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag);
	virtual bool GetPersonalScore(bool offense, int roleid, int& combat_time, int& attend_time, int& kill_count, int& death_count, int& country_kill_count,int& country_death_count);
	virtual bool GetCountryBattleInfo(int & attacker_count, int & defender_count);
	virtual void GetStongholdState(int roleid, int cs_index, int cs_sid);
	virtual bool GetLiveShowResult(int roleid, int cs_index, int cs_sid, world* pPlane);

private:
	void FlagModeInit();
	void TowerModeInit();
	void StrongholdModeInit();
	void FlagModeHeartbeat(world * pPlane);
	void TowerModeHeartbeat(world * pPlane);
	void StrongholdModeHeartbeat(world * pPlane);
	void CheckBattleResult(world * pPlane);
	void BattleEnd(world * pPlane);
	void SendDSBattleEnd();
	void BroadcastFlagCarrier();
	void BroadcastFlagMsg(int msg, bool offense);
	void BroadcastStrongholdState();
	bool GenerateTower(world * pPlane, bool offense, int group);
	void UpdateScoreRankList(world* pPlane, const cs_user_map& player_map, int& player_lock, const PSCORE_MAP& score_map, int& score_lock, score_rank_info& rank_info);
	void UpdateExpiredDeathInfo(DEATH_INFO_LIST& death_list, int& lock);
	void MakeLiveResultData(packet_wrapper& h1, world* pPlane, int& lock, score_rank_info& rank_info, DEATH_INFO_LIST& death_list);

	inline void AddMapNode(cs_user_map & map, gplayer * pPlayer)
	{
		int cs_index = pPlayer->cs_index;
		std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
		if(cs_index >= 0 && val.first >= 0)
		{
			map[cs_index].push_back(val);
		}
	}

	inline void DelMapNode(cs_user_map & map, gplayer * pPlayer)
	{
		int cs_index = pPlayer->cs_index;
		std::pair<int,int> val(pPlayer->ID.id,pPlayer->cs_sid);
		if(cs_index >= 0 && val.first >= 0)
		{
			cs_user_list & list = map[cs_index];
			int id = pPlayer->ID.id;
			for(size_t i = 0; i < list.size(); i ++)
			{
				if(list[i].first == id)
				{
					list.erase(list.begin() + i);
					i --;
				}
			}
		}
	}

	inline void __TryInsertPScoreMap(PSCORE_MAP& map, int& lock, int roleid, int cls, int soulpower)
	{
		spin_autolock keeper(lock);
		PSCORE_MAP::iterator it = map.find(roleid);
		if(it == map.end())
		{
			personal_score ps;
			memset(&ps, 0, sizeof(ps));
			ps.cls = cls;
			ps.soulpower = soulpower;
			map.insert(PSCORE_MAP::value_type(roleid, ps));
		}
	}

	inline void __UpdatePScoreKill(PSCORE_MAP& map, int& lock, int roleid, int target_soulpower)
	{
		spin_autolock keeper(lock);
		PSCORE_MAP::iterator it = map.find(roleid);
		if(it != map.end())
		{
			it->second.kill_count ++;
			it->second.kill_count_weighted += target_soulpower;
		}
	}

	inline void __UpdateRankInfoCurPosWithoutLock(world* pPlane, score_rank_info& rank_info)
	{
		int now = g_timer.get_systime();
		if(now - rank_info.timestamp <= RANK_POS_INTERVAL) return;
		
		SCORE_RANK_LIST& ranks = rank_info.ranks;
		SCORE_RANK_LIST::iterator it = ranks.begin();
		while(it != ranks.end())
		{
			world::object_info info;
			bool ret = pPlane->QueryObject(XID(GM_TYPE_PLAYER, it->roleid), info);
			if(!ret)
			{
				it = ranks.erase(it);
				continue;
			}

			it->pos = info.pos;
			++it;
		}
	}
	
	inline void __UpdateExpiredDeathInfoWithoutLock(DEATH_INFO_LIST& death_list)
	{
		int now = g_timer.get_systime();
		unsigned int i = 0;
		for(i = 0; i < death_list.size(); ++i)
		{
			if((now - death_list[i].timestamp) < DEATH_MAX_INTERVAL) break;
		}
		
		death_list.erase(death_list.begin(), death_list.begin() + i);
	}
	
	inline void __UpdatePScoreDeath(PSCORE_MAP& map, DEATH_INFO_LIST& death_list, int& lock, int roleid, const A3DVECTOR& pos)
	{
		spin_autolock keeper(lock);
		PSCORE_MAP::iterator it = map.find(roleid);
		if(it != map.end())
		{
			it->second.death_count ++;
		}
		
		__UpdateExpiredDeathInfoWithoutLock(death_list);
		int now = g_timer.get_systime();
		death_entry entry = {roleid, now, pos};
		death_list.push_back(entry);
	}
	
	inline void __UpdatePScoreTime(PSCORE_MAP& map, int& lock, int roleid, int combat_time, int attend_time, int dmg_output, int dmg_output_weighted, int dmg_endure, int dmg_output_npc)
	{
		spin_autolock keeper(lock);
		PSCORE_MAP::iterator it = map.find(roleid);
		if(it != map.end())
		{
			personal_score & ps = it->second;
			ps.combat_time += combat_time;
			ps.attend_time += attend_time;
			ps.dmg_output += dmg_output;
			ps.dmg_output_weighted += dmg_output_weighted;
			ps.dmg_endure += dmg_endure;
			ps.dmg_output_npc += dmg_output_npc;
		}
	}

	inline void __UpdatePScoreScore(PSCORE_MAP& map, int& lock, int roleid, int score)
	{
		spin_autolock keeper(lock);
		PSCORE_MAP::iterator it = map.find(roleid);
		if(it != map.end())
		{
			it->second.score += score;
		}
	}

	inline bool __GetPScore(PSCORE_MAP& map, int& lock, int roleid, int& combat_time, int& attend_time, int& kill_count, int& death_count)
	{
		spin_autolock keeper(lock);
		PSCORE_MAP::iterator it = map.find(roleid);
		if(it != map.end())
		{
			combat_time = it->second.combat_time;
			attend_time = it->second.attend_time;
			kill_count = it->second.kill_count;
			death_count = it->second.death_count;
			return true;
		}
		return false;
	}
public:
	bool AddAttacker()
	{
		if(_data.attacker_count >= _data.player_count_limit) return false;
		int p = interlocked_increment(&_data.attacker_count);
		if(p > _data.player_count_limit)
		{
			interlocked_decrement(&_data.attacker_count);
			return false;
		}
		else
		{
			return true;
		}
	}
	
	bool AddDefender()
	{
		if(_data.defender_count >= _data.player_count_limit) return false;
		int p = interlocked_increment(&_data.defender_count);
		if(p > _data.player_count_limit)
		{
			interlocked_decrement(&_data.defender_count);
			return false;
		}
		else
		{
			return true;
		}
	}

	void DelAttacker()
	{
		interlocked_decrement(&_data.attacker_count);
	}
	
	void DelDefender()
	{
		interlocked_decrement(&_data.defender_count);
	}
	
	void PlayerEnter(gplayer * pPlayer,int mask); 	//MASK: 1 attacker, 2 defneder
	void PlayerLeave(gplayer * pPlayer,int mask); 	//MASK: 1 attacker, 2 defneder
	
	void DestroyCountryBattle(world* pPlane);
	
	class stream
	{
		public: 
			virtual void dump(const char * str)  {}
	};
	void DumpPlayerScore(int roleid, stream * cb)
	{
		PSCORE_MAP * map = &_defence_pscore_map;
		int * lock = &_lock_defence_map;
		{
			spin_autolock l(lock);
			PSCORE_MAP::iterator it = map->find(roleid);
			if(it != map->end())
			{
				const personal_score & ps = it->second;
				char buf[512];
				sprintf(buf, "roleid(%d)soulpower(%d)combat_t(%d)attend_t(%d)kill(%d)death(%d)score(%d)dmg_o(%d)dmg_o_w(%d)dmg_i(%d)dmg_o_npc(%d)kill_w(%d)",
						roleid, ps.soulpower, ps.combat_time, ps.attend_time, ps.kill_count, ps.death_count, ps.score, ps.dmg_output, ps.dmg_output_weighted, ps.dmg_endure, ps.dmg_output_npc, ps.kill_count_weighted);
				cb->dump(buf);
				return;
			}
		}
		map = &_offense_pscore_map;
		lock = &_lock_offense_map;
		{
			spin_autolock l(lock);
			PSCORE_MAP::iterator it = map->find(roleid);
			if(it != map->end())
			{
				const personal_score & ps = it->second;
				char buf[512];
				sprintf(buf, "roleid(%d)soulpower(%d)combat_t(%d)attend_t(%d)kill(%d)death(%d)score(%d)dmg_o(%d)dmg_o_w(%d)dmg_i(%d)dmg_o_npc(%d)kill_w(%d)",
						roleid, ps.soulpower, ps.combat_time, ps.attend_time, ps.kill_count, ps.death_count, ps.score, ps.dmg_output, ps.dmg_output_weighted, ps.dmg_endure, ps.dmg_output_npc, ps.kill_count_weighted);
				cb->dump(buf);
				return;
			}
		}
		cb->dump("not found");	
	}
};

#endif
