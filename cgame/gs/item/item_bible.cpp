#include "../world.h"
#include "item_bible.h"
#include "item_addon.h"
#include "../clstab.h"
#include "../actobject.h"
#include "../item_list.h"
#include "../worldmanager.h"

DEFINE_SUBSTANCE(bible_item,item_body,CLS_ITEM_BIBLE)

bool 
bible_item::Load(archive & ar)
{
	_extra_addon = set_addon_manager::GetAddonList(_tid);
	return true;
}

void 
bible_item::OnActivate(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj)
{
	if(_extra_addon)
	{       
		for(size_t i = 0;i < _extra_addon->size(); i ++)
		{
			addon_manager::Activate((*_extra_addon)[i],NULL,obj);
		}
	}

	//激活无需喂食宠物的借口
	obj->ActivePetNoFeed(true);
}

void 
bible_item::OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj)
{
	if(_extra_addon)
	{
		for(size_t i = 0;i < _extra_addon->size(); i ++)
		{
			addon_manager::Deactivate((*_extra_addon)[i],NULL,obj);
		}
	}
	//取消无需喂食宠物的借口
	obj->ActivePetNoFeed(false);
}


