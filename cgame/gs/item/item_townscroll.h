#ifndef __ONLINEGAME_GS_ITEM_TOWN_SCROLL_H__
#define __ONLINEGAME_GS_ITEM_TOWN_SCROLL_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

struct townscroll_essence 
{
	int duration;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const townscroll_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, townscroll_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class townscroll_item : public item_body
{
	townscroll_essence _ess;
	unsigned short _crc;
public:
	virtual bool ArmorDecDurability(int) { return false;}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_TOWNSCROLL; }
private:
	virtual unsigned short GetDataCRC() { return 0; }
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

	virtual item_body* Clone() const { return  new townscroll_item(*this); }
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual int OnGetUseDuration() { return _ess.duration;}	
	virtual bool IsItemBroadcastUse() {return true;}
public:
	DECLARE_SUBSTANCE(townscroll_item);
};

class townscroll2_item : public item_body
{
public:
	virtual bool ArmorDecDurability(int) { return false;}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_TOWNSCROLL; }
private:
	virtual unsigned short GetDataCRC() { return 0; }
	virtual bool Load(archive & ar)
	{
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = NULL;
		len = 0;
	}

	virtual item_body* Clone() const { return  new townscroll2_item(*this); }
	virtual bool IsItemCanUseWithArg(item::LOCATION l,size_t buf_size) { return buf_size == sizeof(int);}
	virtual int OnUse(item::LOCATION ,int index, gactive_imp*,const char * arg, size_t arg_size) ;
	virtual int OnGetUseDuration() { return -1;}	
	virtual bool IsItemBroadcastUse() {return false;}
public:
	DECLARE_SUBSTANCE(townscroll2_item);
};
#endif

