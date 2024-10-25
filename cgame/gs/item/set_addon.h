#ifndef __ONLINE_GAME_GS_SET_ADDON_H__
#define __ONLINE_GAME_GS_SET_ADDON_H__

#include "item_addon.h"

class itemdataman;
class set_addon_manager
{
	set_addon_manager(){}
	typedef abase::hash_map<int ,ADDON_LIST *>  MAP;

	MAP _map;

	bool __InsertAddonList(const int * idlist, size_t count, ADDON_LIST * list)
	{
		int num = 0;
		for(size_t i = 0; i < count ;i ++)
		{
			int id = idlist[i];
			if(id > 0)
			{
				ADDON_LIST * tmp = _map[id];
				if(tmp) return false; //ÖØ¸´µÄid
				_map[id] = list;
				num ++;
			}
		}
		return num > 0;
	}

	ADDON_LIST * __GetAddonList(int id)
	{
		MAP::iterator it = _map.find(id);
		if(it == _map.end()) return NULL;
		return it->second;
	}
public:
	static set_addon_manager & GetInstance()
	{
		static set_addon_manager instance;
		return instance;
	}
	static bool InsertAddonList(const int * idlist, size_t count, ADDON_LIST * list)
	{
		return GetInstance().__InsertAddonList(idlist, count, list);
	}

	static ADDON_LIST * GetAddonList(int id)
	{
		return GetInstance().__GetAddonList(id);
	}
	
	static bool LoadTemplate(itemdataman & dataman);
};

#endif

