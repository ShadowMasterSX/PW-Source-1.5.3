#ifndef __ONLINEGAME_GS_OCCUP_PACKAGE_ITEM_H__
#define __ONLINEGAME_GS_OCCUP_PACKAGE_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include <crc.h>

class item_occup_package: public item_body
{
protected:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual int  OnGetUseDuration() { return 0;}	
	virtual bool IsItemBroadcastUse() {return false;}
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual int  OnUse(item::LOCATION l,gactive_imp * obj,size_t count);

public:
	item_occup_package()
	{
	}

public:
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_OCCUP_PACKAGE;
	}

	virtual item_body* Clone() const
	{ 
		return  new item_occup_package(*this); 
	}
public:
	DECLARE_SUBSTANCE(item_occup_package);

};
#endif

