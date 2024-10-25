#ifndef __ONLINEGAME_GS_TRICKBATTLE_CTRL_H__
#define __ONLINEGAME_GS_TRICKBATTLE_CTRL_H__

class trickbattle_ctrl : public world_data_ctrl
{
public:
	struct
	{
		int battle_id;
		int attacker_count;
		int defender_count;
		int player_count_limit;
		int end_timestamp;
	}_data;

	struct personal_score
	{
		int kill_count;
		int death_count;
		int score;
		bool changed;
	};
	typedef abase::hash_map<int, personal_score, abase::_hash_function, abase::fast_alloc<> > PSCORE_MAP;
	
	typedef abase::hash_map<int/*chariot*/, int/*chariot_count*/> CHARIOT_MAP;

	int _lock;
	int _tick_counter;
	int _battle_result;
	int _battle_end_timer;

	int _lock_pscore_map;
	PSCORE_MAP _pscore_map;		//保存玩家得分变化值

	int _lock_chariot_map;
	CHARIOT_MAP _attacker_chariot_map;
	CHARIOT_MAP _defender_chariot_map;

	cs_user_map  _attacker_list;
	cs_user_map  _defender_list;
	cs_user_map  _all_list;
	int _user_list_lock;
public:
	trickbattle_ctrl():_lock(0),_tick_counter(0),_battle_result(0),_battle_end_timer(0),
						 _lock_pscore_map(0),_lock_chariot_map(0),_user_list_lock(0)
	{
		memset(&_data,0,sizeof(_data));
	}
	virtual ~trickbattle_ctrl(){}
	virtual world_data_ctrl * Clone()
	{
		return new trickbattle_ctrl(*this);
	}
	virtual void Reset()
	{
		memset(&_data,0,sizeof(_data));
	}
	virtual void Tick(world * pPlane);
	virtual void BattleFactionSay(int faction, const void * buf, size_t size, char emote_id, const void * aux_data, size_t dsize, int self_id, int self_level);
	virtual void UpdatePersonalScore(int roleid, int kill, int death, int score);
	
	virtual void AddChariot(int type, int chariot)
	{
		spin_autolock keeper(_lock_chariot_map);
		if(type & 0x01)
		{
			CHARIOT_MAP::iterator it = _attacker_chariot_map.find(chariot);
			if (it == _attacker_chariot_map.end())
				_attacker_chariot_map[chariot] = 1;
			else
				it->second ++;
		}
		else if (type & 0x02)
		{
			CHARIOT_MAP::iterator it = _defender_chariot_map.find(chariot);
			if (it == _defender_chariot_map.end())
				_defender_chariot_map[chariot] = 1;
			else
				it->second ++;
		}
	}
	virtual void DelChariot(int type, int chariot)
	{
		spin_autolock keeper(_lock_chariot_map);
		if(type & 0x01)
		{
			CHARIOT_MAP::iterator it = _attacker_chariot_map.find(chariot);
			if (it != _attacker_chariot_map.end() && it->second > 0)
				it->second --;
		}
		else if (type & 0x02)
		{
			CHARIOT_MAP::iterator it = _defender_chariot_map.find(chariot);
			if (it != _defender_chariot_map.end() && it->second > 0)
				it->second --;
		}
	}
	virtual void GetChariots(int type, abase::hash_map<int, int> & chariot_map)
	{
		spin_autolock keeper(_lock_chariot_map);
		if (type & 0x01)
		{
			CHARIOT_MAP::iterator it = _attacker_chariot_map.begin(), eit = _attacker_chariot_map.end();
			for ( ; it != eit; ++it)
			{
				chariot_map[it->first] = it->second;
			}
		}
		else if (type & 0x02)
		{
			CHARIOT_MAP::iterator it = _defender_chariot_map.begin(), eit = _defender_chariot_map.end();
			for ( ; it != eit; ++it)
			{
				chariot_map[it->first] = it->second;
			}
		}
	}

private:
	void CheckBattleResult(world * pPlane);
	void BattleEnd(world * pPlane);
	void SendDSBattleScore();
	void SendDSBattleEnd();
	
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
};

#endif
