#ifndef _ITEM_QUERYOTHERPROPERTY_H_
#define _ITEM_QUERYOTHERPROPERTY_H_

#include <stddef.h>
#include "../item.h" 
#include "../config.h"

//查询其他人的基本属性

class queryotherproperty_item : public item_body
{
public:
	DECLARE_SUBSTANCE(queryotherproperty_item);

public:
	//item_body中纯虚函数
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_QUERYOTHERPROPERTY;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new queryotherproperty_item(*this);}
public:
	virtual int OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack);
	virtual bool IsItemCanUseWithTarget(item::LOCATION l) { return true;}
	virtual int OnGetUseDuration() { return -1;}
	virtual bool IsItemBroadcastUse() {return false;}
};

#endif
