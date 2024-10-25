#ifndef _ITEM_ELF_EQUIP_H_
#define _ITEM_ELF_EQUIP_H_

#include <stddef.h>
#include "../item.h" 
#include "../config.h"

class elf_equip_item : public item_body
{
public:
	DECLARE_SUBSTANCE(elf_equip_item);

public:
	//item_bodyÖÐ´¿Ðéº¯Êý
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_ELF_EQUIP;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new elf_equip_item(*this);}
};

#endif
