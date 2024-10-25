#ifndef __ONLINEGAME_GS_ITEM_MOB_ITEM_GEN_H__
#define __ONLINEGAME_GS_ITEM_MOB_ITEM_GEN_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"

class item_mob_gen : public item_body
{
protected:
	int _mob_id;
protected:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual item_mob_gen* Clone() const {return  new item_mob_gen(*this);}
public:
	DECLARE_SUBSTANCE(item_mob_gen);
	item_mob_gen()
	{}

public:
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_MOB_GEN;
	}
	virtual bool IsItemBroadcastUse() {return false;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool Save(archive & ar)
	{
		ar << _mob_id;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ar >> _mob_id;
		return true;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_mob_id; 
		len = sizeof(_mob_id);
	}

};

#endif

