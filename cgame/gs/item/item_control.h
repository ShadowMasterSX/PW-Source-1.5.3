#ifndef __ONLINEGAME_GS_ITEM_CONTROL_H__
#define __ONLINEGAME_GS_ITEM_CONTROL_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

class item_control_mob : public item_body
{
public:
	virtual bool ArmorDecDurability(int) { return false;}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_TANK_CONTROL; }
private:
	virtual unsigned short GetDataCRC() { return 0; }
	virtual bool Load(archive & ar)
	{
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = "";
		len = 0;
	}
	virtual item_body* Clone() const { return  new item_control_mob (*this); }
	virtual int OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual bool IsItemCanUseWithTarget(item::LOCATION l) { return true;}
	virtual int OnGetUseDuration() { return 5*20;}	
public:
	DECLARE_SUBSTANCE(item_control_mob);
};
#endif

