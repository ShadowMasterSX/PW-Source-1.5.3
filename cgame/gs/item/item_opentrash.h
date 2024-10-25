#ifndef __ONLINEGAME_GS_OPEN_TRASH_H__
#define __ONLINEGAME_GS__H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

class item_open_trash : public item_body
{
protected:

	virtual item_body* Clone() const { return new item_open_trash(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
public:
	DECLARE_SUBSTANCE(item_open_trash);
	item_open_trash()
	{
	}

	virtual bool Save(archive & ar)
	{
		return true;
	}

	virtual bool Load(archive & ar)
	{
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = NULL;
		len = 0;
	}
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_DUMMY;
	}
};

#endif

