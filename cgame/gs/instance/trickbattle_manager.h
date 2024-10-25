#ifndef __ONLINEGAME_GS_TRICKBATTLE_MANAGER_H__
#define __ONLINEGAME_GS_TRICKBATTLE_MANAGER_H__

#include "instance_manager.h"

struct trick_battle_param
{
	int battle_id;
	int player_count;	//每方玩家限制的人数
	int end_timestamp;	//结束时间
};

/*------------------------战场副本管理-------------------------------*/
class trickbattle_world_manager : public instance_world_manager 
{
private:
	struct town_entry
	{
		int faction;
		A3DVECTOR target_pos;
	};
	
	abase::vector<town_entry, abase::fast_alloc<> > _town_list;
	bool GetTown(int faction, A3DVECTOR &pos, int & tag)
	{
		int list[64];
		int counter = 0;
		for(size_t i = 0; i < _town_list.size() && counter < 64; i ++)
		{
			if(_town_list[i].faction & faction)
			{
				list[counter] = i;
				counter ++;
			}
		}
		if(counter > 0)
		{
			int index = abase::Rand(0,counter-1);
			pos = _town_list[list[index]].target_pos;
			tag = GetWorldTag();
			return true;
		}
		return false;
	}

	struct energy_mine_entry
	{
		int mine_tid;
		int energy;
	};

	abase::vector<energy_mine_entry, abase::fast_alloc<> > _energy_mine_list;
	
	int GetMineEnergy(int mine)
	{
		for(size_t i=0; i<_energy_mine_list.size(); i++)
		{
			if(_energy_mine_list[i].mine_tid == mine)
				return _energy_mine_list[i].energy;
		}
		return 0;
	}
public:
	trickbattle_world_manager():instance_world_manager()
	{
		//战场副本应该是固定时间清除
		_idle_time = 300;
		_life_time = -1;
	}
	virtual int GetWorldType(){ return WORLD_TYPE_TRICKBATTLE; }
	virtual world * CreateWorldTemplate();
	virtual world_message_handler * CreateMessageHandler();
	virtual void Heartbeat();
	virtual void FinalInit(const char * servername);
	virtual void OnDeliveryConnected();
	virtual int OnMineGathered(world * pPlane, int mine_tid, gplayer* pPlayer);

	virtual void UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag);
	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey);
	virtual void PlayerAfterSwitch(gplayer_imp * pImp);
	virtual void GetLogoutPos(gplayer_imp * pImp, int & world_tag ,A3DVECTOR & pos);
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag);
	virtual void RecordTownPos(const A3DVECTOR &pos,int faction);
	virtual void SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos, int special_mask);
	
	virtual void TransformInstanceKey(const instance_key::key_essence & key, instance_hash_key & hkey)
	{
		hkey.key1 = key.key_level4;
		hkey.key2 = 0;
	}
	virtual int CheckPlayerSwitchRequest(const XID & who,const instance_key * key,const A3DVECTOR & pos,int ins_timer);
	virtual world * GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int );
	virtual bool CreateTrickBattle(const trick_battle_param &);
};

class trickbattle_world_message_handler : public instance_world_message_handler
{
protected:
	virtual ~trickbattle_world_message_handler(){}
	virtual void PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pimp,instance_key &  ikey);
public:
	trickbattle_world_message_handler(instance_world_manager * man):instance_world_message_handler(man) {}
};

#endif
