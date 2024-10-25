#ifndef _ITEM_DYNSKILL_H_
#define _ITEM_DYNSKILL_H_

#include <stddef.h>
#include "../item.h" 
#include "../config.h"

//可装备的物品，装备后Player增加可用的动态技能

class dynskill_item : public item_body
{
public:
	DECLARE_SUBSTANCE(dynskill_item);
	
public:
	//item_body中纯虚函数
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_DYNSKILL;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new dynskill_item(*this);}

public:
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return true;}
	virtual void OnPutIn(item::LOCATION l,item_list & list,size_t pos,size_t count,gactive_imp* obj)
	{
		if(l == item::BODY)
		{
			Activate(l,list, pos, count,obj);
		}
	}
	virtual void OnTakeOut(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj)
	{
		if(l == item::BODY)
		{
			Deactivate(l,pos,count,obj);
		}
	}
	virtual void OnActivate(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj);
	virtual void OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj);
};

#endif
