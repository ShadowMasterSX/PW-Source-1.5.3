#ifndef _ITEM_SHARPENER_H_
#define _ITEM_SHARPENER_H_

#include <stddef.h>
#include "../item.h" 
#include "../config.h"

class sharpener_item : public item_body
{
public:
	DECLARE_SUBSTANCE(sharpener_item);
	
public:
	//item_bodyÖÐ´¿Ðéº¯Êý
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_SHARPENER;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new sharpener_item(*this);}
public:
	virtual bool IsItemCanUseWithArg(item::LOCATION l, size_t arg_size)
	{
		return arg_size == sizeof(unsigned int);
	}
	virtual int OnUse(item::LOCATION l, int index, gactive_imp * imp, const char * arg, size_t arg_size);
};

#endif
