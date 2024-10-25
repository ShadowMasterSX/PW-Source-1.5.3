#ifndef __ONLINEGAME_GS_PARALLEL_WORLD_MANAGER_H__
#define __ONLINEGAME_GS_PARALLEL_WORLD_MANAGER_H__

#include "instance_manager.h"

/*平行世界管理器用于地图的分线,基于副本的方式实现,但又与大地图有很多相似之处,其主要特点如下
 1 根据人数自动释放、创建world
 2 玩家可以在world间自由切换，
 3 如玩家进入时未指定world，则服务器为玩家选择
 */

class parallel_world_manager : public instance_world_manager
{
public:
	parallel_world_manager()
	{
		_idle_time = 300;
		_life_time = -1;	
	}

	virtual int GetWorldType(){ return WORLD_TYPE_PARALLEL_WORLD; }
	virtual world_message_handler * CreateMessageHandler();
	virtual void Heartbeat();
	virtual void SetFilterWhenLogin(gplayer_imp * pImp, instance_key * ikey);
	virtual int PlayerSwitchWorld(gplayer * pPlayer, const instance_hash_key & key);
	virtual void PlayerQueryWorld(gplayer * pPlayer);
	virtual void TransformInstanceKey(const instance_key::key_essence & key, instance_hash_key & hkey)
	{
		hkey.key1 = key.key_level4;
		hkey.key2 = 0;
	}
	virtual int CheckPlayerSwitchRequest(const XID & who,const instance_key * key,const A3DVECTOR & pos,int ins_timer);//注意:此函数可能改变key
	virtual world * GetWorldInSwitch(const instance_hash_key & ikey,int & world_index,int );
	virtual world * GetWorldOnLogin(const instance_hash_key & ikey,int & world_index);
private:
	void ModifyInstanceKey(instance_key::key_essence & key, const instance_hash_key & hkey)
	{
		key.key_level4 = hkey.key1;
	}
	world * GetFirstAvailabelWorld(int & world_index);
	world * GetAvailabelWorldRandom(int & world_index);
	inline bool WorldCapacityLow()
	{
		return _active_plane_count*_player_limit_per_instance - GetPlayerAlloced() <= (int)(_player_limit_per_instance*0.5f+0.5f); 
	}
	inline bool WorldCapacityHigh()
	{
		return _active_plane_count*_player_limit_per_instance - GetPlayerAlloced() >= (int)(_player_limit_per_instance*1.8f+0.5f);
	}
	void AutoAllocWorld();
};

class parallel_world_message_handler : public instance_world_message_handler
{
protected:
	virtual ~parallel_world_message_handler(){}
public:
	parallel_world_message_handler(instance_world_manager * man):instance_world_message_handler(man) {}
};

#endif
