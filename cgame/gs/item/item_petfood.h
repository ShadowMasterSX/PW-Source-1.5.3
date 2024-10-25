#ifndef __ONLINEGAME_GS_PETFOOD_ITEM_H__
#define __ONLINEGAME_GS_PETFOOD_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include <crc.h>

struct pe_food
{
	int food_type;
	int honor;
};

class item_pet_food: public item_body
{
	pe_food _ess;
protected:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual int OnGetUseDuration() { return 0;}	
	virtual bool IsItemBroadcastUse() {return false;}
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);

public:
	item_pet_food()
	{
		_ess.food_type = 0;
		_ess.honor = 0;
	}

public:
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_PET_FOOD;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess; 
		len = sizeof(_ess);
	}

	virtual item_body* Clone() const
	{ 
		return  new item_pet_food(*this); 
	}

	virtual bool Save(archive & ar)
	{
		ar.push_back(&_ess,sizeof(_ess));
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ar.pop_back(&_ess,sizeof(_ess));
		return true;
	}
public:
	DECLARE_SUBSTANCE(item_pet_food);

};
#endif

