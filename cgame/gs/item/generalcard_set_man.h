#ifndef __ONLINE_GAME_GS_GENERALCARD_SET_MAN_H__
#define __ONLINE_GAME_GS_GENERALCARD_SET_MAN_H__

#include <hashmap.h>

struct generalcard_set
{
	int id;				//武将卡套装模板ID
	size_t total_count;	//套装中卡牌总数
	float enhance;		//套装激活后基础属性提升比例	
	generalcard_set():id(0),total_count(0),enhance(0.f){}
};

class itemdataman;
class generalcard_set_manager
{
	typedef abase::hash_map<int, generalcard_set *> MAP;
	MAP _map;
	
	generalcard_set_manager(){}

	bool __Insert(const int * cardidlist, size_t count, generalcard_set * cardset)
	{
		int num = 0;
		for(size_t i = 0; i < count ; i ++)
		{
			int id = cardidlist[i];
			if(id > 0)
			{
				generalcard_set * tmp = _map[id];
				if(tmp) return false; //重复的id
				_map[id] = cardset;
				num ++;
			}
		}
		return num > 0;
	}
	
	generalcard_set * __Get(int cardid)
	{
		MAP::iterator it = _map.find(cardid);
		if(it == _map.end()) return NULL;
		return it->second;
	}

public:
	static generalcard_set_manager & GetInstance()
	{
		static generalcard_set_manager instance;
		return instance;
	}

	static bool Insert(const int * cardidlist, size_t count, generalcard_set * cardset)
	{
		return GetInstance().__Insert(cardidlist, count, cardset);
	}

	static generalcard_set * Get(int cardid)
	{
		return GetInstance().__Get(cardid);
	}

	static bool LoadTemplate(itemdataman & dataman);
};

#endif
