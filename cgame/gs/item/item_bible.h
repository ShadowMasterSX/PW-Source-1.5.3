#ifndef __ONLINEGAME_GS_BIBLE_H__
#define __ONLINEGAME_GS_BIBLE_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

class bible_item : public item_body
{
protected:
	const ADDON_LIST * _extra_addon;//额外附加的addon 主要用于套装和其它属性
	virtual unsigned short GetDataCRC() { return 0xFF;}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = "";
		len = 0;
	}
	virtual bool Load(archive & ar);
public:
	bible_item():_extra_addon(NULL)
	{}
	
	virtual bool ArmorDecDurability(int) { return false;}
	virtual void OnPutIn(item::LOCATION l,item_list & list,size_t pos,size_t count,gactive_imp* obj) 
	{
		//do nothing  因为激活由外面的RefreshEquip控制
	}
	
	virtual void OnTakeOut(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj)
	{ 
		if(l == item::BODY) 
		{
			Deactivate(l,pos,count,obj);
		}
	}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_BIBLE; }
	virtual void GetDurability(int &dura,int &max_dura) { dura = 100; max_dura = 100; }

	virtual item_body* Clone() const { return  new bible_item(*this); }
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj)
	{
		return true;
	}

	virtual void OnActivate(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj);
	virtual void OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj);

	DECLARE_SUBSTANCE(bible_item);
};

#endif

