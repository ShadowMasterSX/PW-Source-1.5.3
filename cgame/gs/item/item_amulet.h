#ifndef __ONLINEGAME_GS_AMULET_ITEM_H__
#define __ONLINEGAME_GS_AMULET_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include <crc.h>

//这几样物品和喇叭 天书不进行循环测试，所以在PutIn时就会激活

struct  amulet_essence
{
	int point;
	float trigger_percent;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const amulet_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, amulet_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class base_amulet : public item_body
{
protected:
	amulet_essence _ess;
	virtual bool Save(archive & ar)
	{
		ar << _ess; 
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ar >> _ess; 
		return true;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess;
		len = sizeof(_ess);
	}
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_AMULET;
	}
	virtual void OnPutIn(item::LOCATION l,item_list & list,size_t pos,size_t count,gactive_imp* obj) 
	{
		if(l == item::BODY) 
		{
			Activate(l,list,pos,count,obj);
		}
	}
	virtual void OnTakeOut(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj)
	{       
		if(l == item::BODY)
		{       
			Deactivate(l,pos,count,obj);
		}
	}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return true;}
	virtual int OnAutoTrigger(gactive_imp * obj, int cooldown_idx,int offset);
	virtual void OnTrigger(gactive_imp * , int value) = 0;
public:
};

class hp_amulet_item : public base_amulet
{
public:
	DECLARE_SUBSTANCE(hp_amulet_item);
	item_body * Clone() const { return new hp_amulet_item(*this);}

	virtual void OnActivate(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj);
	virtual void OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj);
	virtual void OnTrigger(gactive_imp * , int value);
};

class mp_amulet_item : public base_amulet
{
public:
	DECLARE_SUBSTANCE(mp_amulet_item);
	item_body * Clone() const { return new mp_amulet_item(*this);}

	virtual void OnActivate(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj);
	virtual void OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj);
	virtual void OnTrigger(gactive_imp * , int value);
};
#endif

