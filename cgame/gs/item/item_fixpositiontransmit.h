#ifndef __ONLINE_GAME_GS_ITEM_FIXPOSITIONENERGY_H__
#define __ONLINE_GAME_GS_ITEM_FIXPOSITIONENERGY_H__

#include "../item.h"

class item_fixpositiontransmit: public item_body
{
protected:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual ITEM_TYPE GetItemType() { return ITEM_TYPE_FIXPOSITIONTRANSMIT; }
	
	virtual item_body * Clone() const { return new item_fixpositiontransmit(*this);}
	virtual bool ArmorDecDurability(int) { return false;}

public:
	item_fixpositiontransmit()
	{}
public:
	virtual bool Save(archive & ar)
	{
		return true;
	}

	virtual bool Load(archive & ar)
	{
		return true;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = "";
		len = 0;
	}

	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);

public:
	DECLARE_SUBSTANCE(item_fixpositiontransmit);
};
#endif
