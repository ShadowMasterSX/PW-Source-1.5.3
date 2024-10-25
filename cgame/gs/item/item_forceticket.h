#ifndef _ITEM_FORCETICKET_H_
#define _ITEM_FORCETICKET_H_

#include <stddef.h>
#include "../item.h" 

//调整势力声望获取的道具

struct force_ticket_essence
{
	int require_force;
	int repu_total;
	int repu_inc_ratio;
};

class force_ticket_item : public item_body
{
	struct force_ticket_essence ess;
	
public:
	DECLARE_SUBSTANCE(force_ticket_item);
	force_ticket_item():_crc(0){memset(&ess, 0, sizeof(ess));}

	virtual bool Load(archive & ar)
	{
		ar.pop_back(&ess, sizeof(ess));
		CalcCRC();
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &ess;
		len = sizeof(ess);
 	}
	
	unsigned short _crc;
	unsigned short GetDataCRC() { return _crc; }
	void CalcCRC()
	{   
		_crc = crc16( (unsigned char *)&ess, sizeof(ess));
	}

public:
	//item_body中纯虚函数
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_FORCE_TICKET;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new force_ticket_item(*this);}

public:
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return true;}
	virtual void OnPutIn(item::LOCATION l,item_list & list,size_t pos, size_t count, gactive_imp* obj)
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

	virtual int OnAutoAdjust(int& value, int max);
};



#endif
