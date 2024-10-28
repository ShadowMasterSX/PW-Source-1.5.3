#ifndef __ONLINE_GAME_GS_COMMON_DATA_H__
#define __ONLINE_GAME_GS_COMMON_DATA_H__

#include <map>
#include <spinlock.h>
#include <stdio.h>

enum
{
	COMMON_VALUE_ID_PLAYER_COUNT = 200001,
	COMMON_VALUE_ID_KICKOUT	= 200002,
	COMMON_VALUE_ID_ATTACKER_COUNT = 200003,
	COMMON_VALUE_ID_DEFENDER_COUNT = 200004,
	COMMON_VALUE_ID_PLAYERDEAD_COUNT = 200005,
};

class common_data
{
	std::map<int,int> _map;
	int _lock;
public:
	common_data():_lock(0)
	{}

	int GetValue(int key)
	{
		spin_autolock keeper(_lock);
		return _map[key];
	}

	void SetValue(int key, int value)
	{
		spin_autolock keeper(_lock);
		_map[key] = value;
	}

	int ModifyValue(int key, int offset)
	{
		spin_autolock keeper(_lock);
		return (_map[key] += offset);
	}

	class stream
	{
	public:
		virtual void dump(const char * str)  {}
		virtual void dump(int key, int value) {}
	};
	void Dump(stream * cb)
	{
		spin_autolock keeper(_lock);
		char buf[512];
		sprintf(buf, "total:%d", _map.size());
		cb->dump(buf);
		for(std::map<int,int>::iterator it = _map.begin(); it != _map.end(); ++it)
		{
			sprintf(buf, "var[%d]=%d", it->first, it->second);
			cb->dump(buf);
		}
		
	}

	void Dump(int startkey, stream * cb)
	{
		spin_autolock keeper(_lock);
		std::map<int,int>::iterator it = _map.lower_bound(startkey);
		for( ; it != _map.end(); ++it)
		{
			cb->dump(it->first, it->second);
		}
	}
	
};

#endif

