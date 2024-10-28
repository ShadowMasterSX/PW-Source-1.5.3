#ifndef __ONLINEGAME_GS_MNFACTION_MANAGER_H__
#define __ONLINEGAME_GS_MNFACTION_MANAGER_H__

#include "instance_manager.h"

struct mnfaction_battle_param
{
	int domain_id;
	unsigned char domain_type;
	int64_t owner_faction_id;
	int64_t attacker_faction_id;
	int64_t defender_faction_id;
	int end_timestamp;//跨服帮派战结束时间
};

/*-------------------跨服帮派战副本管理--------------------*/
class mnfaction_world_manager : public instance_world_manager
{
public:
	enum
	{
		BATTLE_CREATE_SUCCESS = 0,
		BATTLE_CREATE_FAILED,
		BATTLE_CREATE_ALREADY,
	};
public:
	mnfaction_world_manager() : instance_world_manager()
	{
		_idle_time = 300;
		_life_time = -1;
		attacker_boss_tid = 0;
		defender_boss_tid = 0;
		attacker_small_boss_tid = 0;
		defender_small_boss_tid = 0;
		have_creat_battle_domain_ids.clear();
	}
	virtual int GetWorldType(){return WORLD_TYPE_MNFACTION;}
	virtual world * CreateWorldTemplate();
	virtual world_message_handler * CreateMessageHandler();
	virtual void Heartbeat();
	virtual void OnDeliveryConnected();
	virtual void FinalInit(const char * servername);
	virtual void UserLogin(int cs_index,int cs_sid,int uid,const void * auth_data, size_t auth_size, bool isshielduser, char flag);
	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey);
	int CreateMNFactionBattle(const mnfaction_battle_param &);
	int CheckPlayerSwitchRequest(const XID & who,const instance_key * ikey,const A3DVECTOR & pos,int ins_timer);
	//virtual world * GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int );
	virtual void SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos, int special_mask);
	virtual void TransformInstanceKey(const instance_key::key_essence & key, instance_hash_key & hkey)
	{
		hkey.key1 = key.key_level4;
		hkey.key2 = 0;
	}

	virtual void PlayerAfterSwitch(gplayer_imp * pImp);
	virtual void GetLogoutPos(gplayer_imp * pImp, int &world_tag, A3DVECTOR & pos);
	instance_hash_key GetLogoutInstanceKey(gplayer_imp *pImp) const;

	//战斗规则相关内容
public:
	virtual int CanBeGathered(int player_faction, int mine_tid, world *pPlane, const XID &player_xid);
	virtual int OnMineGathered(world * pPlane, int mine_tid, gplayer* pPlayer);
	int OnMobDeath(world * pPlane, int faction,  int tid);
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag);
private:
	int attacker_boss_tid;
	int defender_boss_tid;
	int attacker_small_boss_tid;
	int defender_small_boss_tid;
	std::set<int> have_creat_battle_domain_ids;
};

class mnfaction_world_message_handler : public instance_world_message_handler
{
protected:
	virtual ~mnfaction_world_message_handler(){}
	virtual void PlayerPreEnterServer(gplayer *pPlayer, gplayer_imp * pImp,instance_key & ikey);
public:
	mnfaction_world_message_handler(instance_world_manager * man):instance_world_message_handler(man) {}
	virtual int RecvExternMessage(int msg_tag, const MSG & msg);
};

#endif
