#ifndef __ONLINEGAME_GS_FACTION_WORLD_CTRL_H__
#define __ONLINEGAME_GS_FACTION_WORLD_CTRL_H__


namespace GNET
{
	class FactionFortressResult;
}

class faction_world_ctrl : public world_data_ctrl
{
public:
	enum
	{
		TECHNOLOGY_COUNT = 5,	//目前五种科技点
		MATERIAL_COUNT = 8,		//目前八种材料
		BUILDING_MAX = 20,		//目前最多允许二十个建筑
		SAVED_COMMON_VALUE_START = 290001,	//需要存盘的全局变量范围[290001,290100]
		SAVED_COMMON_VALUE_END 	= 290100,
		SAVED_ACTIVED_SPAWNER_START = 100000,	//需要存盘的控制器范围[100000,100099]
		SAVED_ACTIVED_SPAWNER_END = 100099,
		MAX_FORTRESS_LEVEL = 50,
		MAX_TECH_LEVEL = 7,
		PLAYER_LIMIT_IN_BATTLE = 40,
	};

	struct building_data
	{
		int id;				//帮派基地建筑在模板中的id
		int finish_time;	//0已建完 >0完工时间点 
	};
		
	int tick_counter;
	int write_timer;
	int lock;
	//关于副本的一些状态
	int iskick;		//基地是否处于踢人状态
	//需要存盘的帮派基地数据
	int factionid;	
	int level;
	int exp;			
	int exp_today;		//当天获得的exp
	int exp_today_time;	//当天的起始时间
	int tech_point;		//剩余的科技点
	int technology[TECHNOLOGY_COUNT];
	int material[MATERIAL_COUNT];
	int building_count;
	building_data building[BUILDING_MAX];
	int common_value[SAVED_COMMON_VALUE_END-SAVED_COMMON_VALUE_START+1];
	char actived_spawner[SAVED_ACTIVED_SPAWNER_END-SAVED_ACTIVED_SPAWNER_START+1];
	//以下数值deliveryd负责更新
	int health;			//健康度
	int offense_faction;//当前的进攻帮派
	int offense_starttime;//进攻开始时间
	int offense_endtime;//进攻结束时间
	//副本内玩家信息	
	bool inbattle;
	int player_count_limit;			//战斗中此值为PLAYER_LIMIT_IN_BATTLE 其他为999
	int defender_count;
	int attacker_count;
	cs_user_map  _attacker_list;
	cs_user_map  _defender_list;
	cs_user_map  _all_list;
	int _user_list_lock;
public:
	faction_world_ctrl():tick_counter(0),write_timer(0),lock(0),
		iskick(0),factionid(0),level(0),exp(0),exp_today(0),exp_today_time(0),tech_point(0),building_count(0),
		health(0),offense_faction(0),offense_starttime(0),offense_endtime(0),
		inbattle(false),player_count_limit(999),defender_count(0),attacker_count(0),_user_list_lock(0)
	{
		memset(technology,0,sizeof(technology));
		memset(material,0,sizeof(material));
		memset(building,0,sizeof(building));
		memset(common_value,0,sizeof(common_value));
		memset(actived_spawner,0,sizeof(actived_spawner));
	}
	virtual ~faction_world_ctrl() {}
	virtual world_data_ctrl * Clone()
	{
		return new faction_world_ctrl(*this);
	}
	virtual void Reset();
	virtual void Tick(world * pPlane);
	virtual void OnSetCommonValue(int key, int value);	
	virtual void OnTriggerSpawn(int controller_id);
	virtual void OnClearSpawn(int controller_id);
	virtual void OnServerShutDown();
	virtual int GetFactionId(){ return factionid; }
	virtual bool LevelUp();
	virtual bool SetTechPoint(size_t tech_index);
	virtual bool ResetTechPoint(world * pPlane, size_t tech_index);
	virtual bool Construct(world * pPlane, int id, int accelerate);
	virtual bool HandInMaterial(int id, size_t count);
	virtual bool HandInContrib(int contrib);
	virtual bool MaterialExchange(size_t src_index,size_t dst_index,int material);
	virtual bool Dismantle(world * pPlane, int id);
	virtual bool GetInfo(int roleid, int cs_index, int cs_sid);

	void OnBuildingDestroyed(world * pPlane, int id); 
		
public:
	void Init(world * pPlane, const GNET::faction_fortress_data * data,const GNET::faction_fortress_data2 * data2);
	void OnNotifyData(world * pPlane, const GNET::faction_fortress_data2 * data2);

private:
	void SaveFactionData(GNET::FactionFortressResult * callback);
	void ResetCommonValueAndSpawner(world * pPlane);

public:
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

	bool AddAttacker()
	{
		if(attacker_count >= player_count_limit) return false;
		int p = interlocked_increment(&attacker_count);
		if(p > player_count_limit)
		{
			interlocked_decrement(&attacker_count);
			return false;
		}
		else
		{
			return true;
		}
	}
	
	bool AddDefender()
	{
		if(defender_count >= player_count_limit) return false;
		int p = interlocked_increment(&defender_count);
		if(p > player_count_limit)
		{
			interlocked_decrement(&defender_count);
			return false;
		}
		else
		{
			return true;
		}
	}

	void DelAttacker()
	{
		interlocked_decrement(&attacker_count);
	}
	
	void DelDefender()
	{
		interlocked_decrement(&defender_count);
	}
	
	void PlayerEnter(gplayer * pPlayer,int mask); 	//MASK: 1 attacker, 2 defneder
	void PlayerLeave(gplayer * pPlayer,int mask); 	//MASK: 1 attacker, 2 defneder

};
#endif
