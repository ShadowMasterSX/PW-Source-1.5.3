#include "global_drop.h"
#include "dbgprt.h"
#include <math.h>

abase::hashtab<drop_template::drop_entry*,int, abase::_hash_function > drop_template::_drop_table(1024);

bool 
drop_template::LoadDropList()
{
	vector<EXTRADROPTABLE>& list = get_extra_drop_table();

	for(size_t i = 0; i < list.size(); i ++)
	{
		if(list[i].type != EDT_TYPE_REPLACE && list[i].type != EDT_TYPE_ADDON)
		{
			__PRINTINFO("掉落表类型错误\n");
			return false;
		}

		drop_entry * pEnt = new drop_entry;
		pEnt->type = list[i].type;
		memcpy(pEnt->drop_num_probability,list[i].drop_num_probability,sizeof(pEnt->drop_num_probability));

		float sp = 0;
		for(size_t j = 0; j < 256; j ++)
		{
			if(list[i].drop_items[j].id == 0) break;

			drop_node node(list[i].drop_items[j].id,  list[i].drop_items[j].probability);
			pEnt->drop_list.push_back(node);
			sp += list[i].drop_items[j].probability;

		}

		if(!pEnt->drop_list.size())
		{
			__PRINTINFO("发现空置的掉落表\n");
			continue;
		}

		if(fabs(1.0f - sp) > 1e-6)
		{
			__PRINTINFO("发现掉落表中存在归一化的内容\n");
			continue;
		}

		for(size_t j = 0; j < list[i].id_monsters.size(); j ++)
		{
			int id = list[i].id_monsters[j];
			if(!_drop_table.put(id, pEnt))
			{
				__PRINTINFO("掉落表中发现重复的id%d\n",id);
				return false;
			}
		}
	}

	vector<EXTRADROPTABLE> list2;

	list.swap(list2);
	return true;
}

