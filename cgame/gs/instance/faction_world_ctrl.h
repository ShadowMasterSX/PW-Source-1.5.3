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
		TECHNOLOGY_COUNT = 5,	//Ŀǰ���ֿƼ���
		MATERIAL_COUNT = 8,		//Ŀǰ���ֲ���
		BUILDING_MAX = 20,		//Ŀǰ��������ʮ������
		SAVED_COMMON_VALUE_START = 290001,	//��Ҫ���̵�ȫ�ֱ�����Χ[290001,290100]
		SAVED_COMMON_VALUE_END 	= 290100,
		SAVED_ACTIVED_SPAWNER_START = 100000,	//��Ҫ���̵Ŀ�������Χ[100000,100099]
		SAVED_ACTIVED_SPAWNER_END = 100099,
		MAX_FORTRESS_LEVEL = 50,
		MAX_TECH_LEVEL = 7,
		PLAYER_LIMIT_IN_BATTLE = 40,
	};

	struct building_data
	{
		int id;				//���ɻ��ؽ�����ģ���е�id
		int finish_time;	//0�ѽ��� >0�깤ʱ��� 
	};
		
	int tick_counter;
	int write_timer;
	int lock;
	//���ڸ�����һЩ״̬
	int iskick;		//�����Ƿ�������״̬
	//��Ҫ���̵İ��ɻ�������
	int factionid;	
	int level;
	int exp;			
	int exp_today;		//�����õ�exp
	int exp_today_time;	//�������ʼʱ��
	int tech_point;		//ʣ��ĿƼ���
	int technology[TECHNOLOGY_COUNT];
	int material[MATERIAL_COUNT];
	int building_count;
	building_data building[BUILDING_MAX];
	int common_value[SAVED_COMMON_VALUE_END-SAVED_COMMON_VALUE_START+1];
	char actived_spawner[SAVED_ACTIVED_SPAWNER_END-SAVED_ACTIVED_SPAWNER_START+1];
	//������ֵdeliveryd�������
	int health;			//������
	int offense_faction;//��ǰ�Ľ�������
	int offense_starttime;//������ʼʱ��
	int offense_endtime;//��������ʱ��
	//�����������Ϣ	
	bool inbattle;
	int player_count_limit;			//ս���д�ֵΪPLAYER_LIMIT_IN_BATTLE ����Ϊ999
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
