#ifndef __VCLIENT_PLAYERMANAGER_H__
#define __VCLIENT_PLAYERMANAGER_H__

#include <map>
#include "spinlock.h"
#include "ASSERT.h"

class MSG;
class Player;
class PlayerManager
{
public:
	typedef std::map<int/*roleid*/,int/*index*/> QueryMap;

	enum
	{
		PLAYER_POOL_SIZE = 8192,
		SPLICE_SIZE = 64,
		POOL_SPLICE = PLAYER_POOL_SIZE/SPLICE_SIZE,	
	};
	
	PlayerManager();
	~PlayerManager();

public:
	void Tick();
	void SpliceTick(unsigned index);

	int GetPlayerIndex(Player * obj);
	Player * GetPlayerByIndex(int index);
	Player * AllocPlayer();
	void FreePlayer(Player * obj);
	bool MapPlayer(int roleid, int index)
	{
		SpinLock::Scoped l(lock_query_map);
		QueryMap::iterator it = query_map.find(roleid);
		if(it == query_map.end())
		{
			query_map.insert(std::make_pair(roleid, index));	
			return true;
		}
		else
			return false;
	}
	void UnmapPlayer(int roleid)
	{
		SpinLock::Scoped l(lock_query_map);
		query_map.erase(roleid);	
	}
	int FindPlayer(int roleid)		//·µ»Øindex
	{
		SpinLock::Scoped l(lock_query_map);
		QueryMap::iterator it = query_map.find(roleid);
		if(it == query_map.end())
			return -1;
		else
			return it->second;
	}

	bool AddPlayer(int roleid);
	bool RemovePlayer(int roleid);
	bool DispatchS2CCmd(int roleid, void * buf, size_t size);
	bool DispatchMessage(MSG * msg);

private:
	Player*			player_pool;
	SpinLock 		lock_heartbeat;
	SpinLock 		lock_idle_list;
	Player*			idle_list;

	SpinLock		lock_shb[POOL_SPLICE];
	SpinLock 		lock_query_map;
	QueryMap 		query_map;
};








#endif
