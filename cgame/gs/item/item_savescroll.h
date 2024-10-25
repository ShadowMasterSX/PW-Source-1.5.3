#ifndef __ONLINEGAME_GS_ITEM_RESURRECT_SCROLL_H__
#define __ONLINEGAME_GS_ITEM_RESURRECT_SCROLL_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

struct resurrect_item_essence 
{
	int duration;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const resurrect_item_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, resurrect_item_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}
class resurrect_scroll_item : public item_body
{
	resurrect_item_essence _ess;
	unsigned short _crc;
public:
	virtual bool ArmorDecDurability(int) { return false;}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_RESURRECT_SCROLL; }
private:
	virtual unsigned short GetDataCRC() { return 0; }
	virtual bool Load(archive & ar)
	{
		ar >> _ess;
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		len = sizeof(_ess);
		*data = & _ess;
	}

	virtual item_body* Clone() const { return  new resurrect_scroll_item(*this); }
	virtual int OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual int OnGetUseDuration() { return _ess.duration;}	
	virtual bool IsItemBroadcastUse() {return true;}
public:
	DECLARE_SUBSTANCE(resurrect_scroll_item);
};
#endif

