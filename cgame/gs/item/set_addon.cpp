#include "../template/itemdataman.h"
#include "set_addon.h"


bool set_addon_manager::LoadTemplate(itemdataman & dataman)
{
	DATA_TYPE  dt;
	unsigned int id = dataman.get_first_data_id(ID_SPACE_ESSENCE,dt);
	for(; id != 0; id = dataman.get_next_data_id(ID_SPACE_ESSENCE,dt))
	{
		//进行套装属性的处理
		if(dt == DT_SUITE_ESSENCE)
		{
			const SUITE_ESSENCE &suite=*(const SUITE_ESSENCE*)dataman.get_data_ptr(id,ID_SPACE_ESSENCE,dt);
			ASSERT(&suite && dt == DT_SUITE_ESSENCE);
			ADDON_LIST * list = new ADDON_LIST;
			for(size_t i = 0; i < 11; i ++)
			{
				int addon_id = suite.addons[i].id;
				if(addon_id > 0)
				{
					addon_data data;
					if(!dataman.generate_addon(addon_id, data))
					{
						//套装里存在错误的属性
						__PRINTINFO("套装里存在错误的属性\n");
						continue;
					}
					if(addon_manager::TestUpdate(data) != addon_manager::ADDON_MASK_ACTIVATE)
					{
						//不是active属性不能使用 
						__PRINTINFO("套装里存在非active的属性\n");
						continue;
					}
					list->push_back(data);
				}
			}
			if(list->size() > 0)
			{
				bool brst = InsertAddonList((int*)&suite.equipments[0].id,12,list);
				if(!brst) 
				{
					delete list;
					__PRINTINFO("套装%d里存在错误的条目\n",id);
					return false;
				}
			}
			else
			{
				delete list;
			}
			continue;
		}

		//进行天书的处理
		if(dt == DT_BIBLE_ESSENCE)
		{
			const BIBLE_ESSENCE &ess=*(const BIBLE_ESSENCE*)dataman.get_data_ptr(id,ID_SPACE_ESSENCE,dt);
			ASSERT(&ess && dt == DT_BIBLE_ESSENCE);
			ADDON_LIST * list = new ADDON_LIST;
			for(size_t i = 0; i < 10; i ++)
			{
				int addon_id = ess.id_addons[i];
				if(addon_id > 0)
				{
					addon_data data;
					if(!dataman.generate_addon(addon_id, data))
					{
						//套装里存在错误的属性
						__PRINTINFO("天书里存在错误的属性\n");
						continue;
					}
					if(addon_manager::TestUpdate(data) != addon_manager::ADDON_MASK_ACTIVATE)
					{
						//不是active属性不能使用 
						__PRINTINFO("天书里存在非active的属性\n");
						continue;
					}

					
					list->push_back(data);
				}
			}
			if(list->size() > 0)
			{
				bool brst = InsertAddonList((int*)&id,1,list);
				if(!brst) 
				{
					delete list;
					__PRINTINFO("天书%d里存在错误的条目\n",id);
					return false;
				}
			}
			else
			{
				delete list;
			}
			continue;
			
		}

		if(dt == DT_POKER_ESSENCE)
		{
			const POKER_ESSENCE & ess = *(const POKER_ESSENCE *)dataman.get_data_ptr(id,ID_SPACE_ESSENCE,dt);
			ASSERT(&ess && dt == DT_POKER_ESSENCE);
			ADDON_LIST * list = new ADDON_LIST;
			for(size_t i = 0; i < 4; i ++)
			{
				int addon_id = ess.addon[i];
				if(addon_id > 0)
				{
					addon_data data;
					if(!dataman.generate_addon(addon_id, data))
					{
						__PRINTINFO("武将卡里存在错误的属性\n");
						continue;
					}
					if(addon_manager::TestUpdate(data) != addon_manager::ADDON_MASK_ACTIVATE)
					{
						//不是active属性不能使用 
						__PRINTINFO("武将卡里存在非active的属性\n");
						continue;
					}

					
					list->push_back(data);
				}
			}
			if(list->size() > 0)
			{
				bool brst = InsertAddonList((int*)&id,1,list);
				if(!brst) 
				{
					delete list;
					__PRINTINFO("武将卡%d里存在错误的条目\n",id);
					return false;
				}
			}
			else
			{
				delete list;
			}
			continue;
		}
	}
	return true;
}
