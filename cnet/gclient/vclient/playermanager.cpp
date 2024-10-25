#include "dbgprt.h"
#include "msg.h"
#include "msgmanager.h"
#include "player.h"
#include "playermanager.h"

PlayerManager::PlayerManager()
{
	player_pool = new Player[PLAYER_POOL_SIZE];	
	for(int i=0; i<PLAYER_POOL_SIZE; i++)
		player_pool[i].next = player_pool + i + 1;
	player_pool[PLAYER_POOL_SIZE].next = NULL;
	idle_list = player_pool;
}

PlayerManager::~PlayerManager()
{
	for(int i=0; i<PLAYER_POOL_SIZE; i++)
	{
		Player * player = &player_pool[i];
		if(!player->active || !player->imp) continue;
		player->Lock();
		if(!player->active || !player->imp)
		{
			player->Unlock();
			continue;
		}

		player->imp->Release();
		player->imp = NULL;
		player->Clear();
		player->Unlock();
	}
}

void PlayerManager::SpliceTick(unsigned int index)
{
	SpinLock::Scoped l(lock_shb[index]);

	int iend = (index+1)*SPLICE_SIZE;

	for(int i=index*SPLICE_SIZE; i<iend; i++)
	{
		Player * player = &player_pool[i];
		if(!player->active || !player->imp) continue;
		player->Lock();
		if(!player->active || !player->imp)
		{
			player->Unlock();
			continue;
		}

		player->imp->Tick();
		player->Unlock();
	}

}

void PlayerManager::Tick()
{
	SpinLock::Scoped l(lock_heartbeat);
	for(int i=0; i<PLAYER_POOL_SIZE; i++)
	{
		Player * player = &player_pool[i];
		if(!player->active || !player->imp) continue;
		player->Lock();
		if(!player->active || !player->imp)
		{
			player->Unlock();
			continue;
		}

		player->imp->Tick();
		player->Unlock();
	}
}

int PlayerManager::GetPlayerIndex(Player * obj)
{ 
	return obj - player_pool;
}

Player * PlayerManager::GetPlayerByIndex(int index)
{ 
	return &player_pool[index]; 
}
	
Player * PlayerManager::AllocPlayer()
{
	//返回一个加锁的obj
	Player * obj;
	{
		SpinLock::Scoped l(lock_idle_list);
		if(!idle_list) return NULL;
		
		obj = idle_list;
		idle_list = idle_list->next;
	}
	obj->Lock();
	obj->active = true;
	obj->next = NULL;
	return obj;		
}

void PlayerManager::FreePlayer(Player * obj)
{
	//obj必须是加锁的
	obj->Clear();
	SpinLock::Scoped l(lock_idle_list);
	obj->next = idle_list;
	idle_list = obj;
}

bool PlayerManager::AddPlayer(int roleid)
{
	Player * player = AllocPlayer();
	if(!player)
	{
		return false;
	}

	if(!MapPlayer(roleid, GetPlayerIndex(player)))
	{
		FreePlayer(player);
		player->Unlock();
		return false;
	}
	__PRINTINFO("AddPlayer: roleid=%d player=%p index=%d\n",roleid,player,GetPlayerIndex(player));

	player->id = roleid;
	player->imp = new PlayerImp(player);
	player->imp->Init();
	player->Unlock();
	return true;
}

bool PlayerManager::RemovePlayer(int roleid)
{
	int index  = FindPlayer(roleid);
	if(index < 0) return false;

	Player * player = GetPlayerByIndex(index);
	player->Lock();

	if(!player->active || player->id != roleid || !player->imp)
	{
		player->Unlock();
		return false;
	}
	__PRINTINFO("RemovePlayer: roleid=%d player=%p index=%d\n",roleid,player,GetPlayerIndex(player));

	player->imp->Release();
	player->imp = NULL;
	UnmapPlayer(roleid);
	FreePlayer(player);

	player->Unlock();
	return true;
}

bool PlayerManager::DispatchS2CCmd(int roleid, void * buf, size_t size)
{
	int index  = FindPlayer(roleid);
	if(index < 0) return false;

	Player * player = GetPlayerByIndex(index);
	player->Lock();

	if(!player->active || player->id != roleid || !player->imp)
	{
		player->Unlock();
		return false;
	}

	player->imp->HandleS2CCmd(buf,size);
	player->Unlock();
	return true;
}

bool PlayerManager::DispatchMessage(MSG * msg)
{
	int roleid = msg->target;
	int index  = FindPlayer(roleid);
	if(index < 0) return false;

	Player * player = GetPlayerByIndex(index);
	player->Lock();

	if(!player->active || player->id != roleid || !player->imp)
	{
		player->Unlock();
		return false;
	}

	player->imp->HandleMessage(msg);
	player->Unlock();
	return true;
}
