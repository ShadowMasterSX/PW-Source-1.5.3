#include "string.h"
#include "world.h"
#include "common/message.h"
#include "playerinstancereenter.h"
#include "worldmanager.h"
#include "instance/instance_manager.h"
#include "player_imp.h"

bool player_instance_reenter::CanReenter()
{
	if(_timeout < g_timer.get_systime()) return false;
	if(0 == _instance_tag) return false;
	if(WORLD_TYPE_INSTANCE != _world_type) return false; // 只支持返回普通副本
	return true;
}

void player_instance_reenter::NotifyClient(gplayer_imp* player)
{
	if(CanReenter())
	{
		player->_runner->instance_reenter_notify(_instance_tag,_timeout);
	}
}

bool TransformInstanceHashKey(int wtype,const instance_hash_key& hkey,instance_key::key_essence& key)
{
	switch(wtype)
	{
		case WORLD_TYPE_INSTANCE:
			return instance_world_manager::TransformInstanceHashKey(hkey,key);
	}

	return false;
}

void player_instance_reenter::LoadFromDB(int tag,int type,instance_hash_key key,int timeout,A3DVECTOR pos)
{
	if(world_manager::GetWorldTag() == 1) // 在大地图登陆才生效副本重入规则
	{
		_timeout = timeout;
		_instance_tag = tag;
		_world_type = type;
		_instance_hkey = key;
		_logout_pos = pos;
	}
}

bool player_instance_reenter::TryReenter(gplayer_imp* player)
{
	if(CanReenter())
	{
		instance_key key;
		memset(&key,0,sizeof(key));
		player->GetInstanceKey(_instance_tag, key);
		if(TransformInstanceHashKey(_world_type, _instance_hkey, key.essence))
		{
			key.target = key.essence; 
			key.special_mask |= IKSM_REENTER;
			return 0==world_manager::GetInstance()->PlaneSwitch(player,_logout_pos,_instance_tag,key,0);
		}
	}
	return false;
}

