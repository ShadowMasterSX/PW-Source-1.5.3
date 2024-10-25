#ifndef __ONLINEGAME_GS_PARALLEL_WORLD_MANAGER_H__
#define __ONLINEGAME_GS_PARALLEL_WORLD_MANAGER_H__

#include "instance_manager.h"

/*ƽ��������������ڵ�ͼ�ķ���,���ڸ����ķ�ʽʵ��,��������ͼ�кܶ�����֮��,����Ҫ�ص�����
 1 ���������Զ��ͷš�����world
 2 ��ҿ�����world�������л���
 3 ����ҽ���ʱδָ��world���������Ϊ���ѡ��
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
	virtual int CheckPlayerSwitchRequest(const XID & who,const instance_key * key,const A3DVECTOR & pos,int ins_timer);//ע��:�˺������ܸı�key
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
