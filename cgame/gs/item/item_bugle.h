#ifndef __ONLINEGAME_GS_BUGLE_H__
#define __ONLINEGAME_GS_BUGLE_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

struct bugle_essence 
{
	int emote_id;
};


template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const bugle_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, bugle_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class bugle_item : public item_body
{
protected:
	bugle_essence _ess;
	virtual unsigned short GetDataCRC() { return (unsigned short)_ess.emote_id; }
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
public:
	virtual bool ArmorDecDurability(int) { return false;}
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
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_BUGLE; }
	virtual void GetDurability(int &dura,int &max_dura) { dura = 100; max_dura = 100; }

	virtual item_body* Clone() const { return  new bugle_item(*this); }
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj)
	{
		return true;
	}

	virtual void OnActivate(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj);
	virtual void OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj);

	DECLARE_SUBSTANCE(bugle_item);
};

#endif

