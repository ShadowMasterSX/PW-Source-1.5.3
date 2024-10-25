#ifndef __ONLINEGAME_GS_ITEM_GENERALCARD_DICE_H__
#define __ONLINEGAME_GS_ITEM_GENERALCARD_DICE_H__

#include "../config.h"
#include "../item.h" 

class generalcard_dice_item : public item_body
{
public:
	DECLARE_SUBSTANCE(generalcard_dice_item);
	
public:
	//item_bodyÖÐ´¿Ðéº¯Êý
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_GENERALCARD_DICE;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new generalcard_dice_item(*this);}
public:
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual int OnGetUseDuration() { return -1;}
};


#endif
