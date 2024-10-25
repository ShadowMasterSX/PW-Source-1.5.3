#ifndef _ITEM_CONGREGATE_H_
#define _ITEM_CONGREGATE_H_

#include <stddef.h>
#include "../item.h" 
#include "../config.h"

class congregate_item : public item_body
{
public:
	DECLARE_SUBSTANCE(congregate_item);
	
public:
	//item_body�д��麯��
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_CONGREGATE;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new congregate_item(*this);}
public:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual int OnGetUseDuration() { return 0;}	//0 ����Ҫ�Ŷӣ���ʹ��ʱ��Ϊ0
};

#endif
