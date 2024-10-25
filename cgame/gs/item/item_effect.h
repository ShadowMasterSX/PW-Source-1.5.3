#ifndef __ONLINEGAME_GS_ITEM_EFFECT_H__
#define __ONLINEGAME_GS_ITEM_EFFECT_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"

class item_base_effect: public item_body
{
protected:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual int  GetFilterID() = 0;
	virtual int  GetCoolDown(int &cd_idx) = 0;
	virtual int GetTimeOut() = 0;

public:
	item_base_effect()
	{}

public:
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_GENERAL_EFFECT;
	}
	virtual bool IsItemBroadcastUse() {return true;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);

};

struct facepill_essence
{
	int duration;
	int require_class;
};
class item_facepill : public item_base_effect
{
	facepill_essence _ess;
public:
	DECLARE_SUBSTANCE(item_facepill);
	virtual item_facepill * Clone() const {return  new item_facepill(*this);}
	virtual int  GetFilterID();
	virtual int  GetCoolDown(int &cd_idx);
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual int GetTimeOut() { return _ess.duration;}

	virtual bool Save(archive & ar)
	{
		ar << _ess.duration << _ess.require_class;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ar >> _ess.duration >> _ess.require_class;
		return true;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess; 
		len = sizeof(_ess);
	}

	 
};

#endif

