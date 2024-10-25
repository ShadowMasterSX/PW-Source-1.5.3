#ifndef _ITEM_DYNSKILL_H_
#define _ITEM_DYNSKILL_H_

#include <stddef.h>
#include "../item.h" 
#include "../config.h"

//��װ������Ʒ��װ����Player���ӿ��õĶ�̬����

class dynskill_item : public item_body
{
public:
	DECLARE_SUBSTANCE(dynskill_item);
	
public:
	//item_body�д��麯��
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
