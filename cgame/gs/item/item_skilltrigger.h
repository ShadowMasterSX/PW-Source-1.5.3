#ifndef __ONLINEGAME_GS_ITEM_SKILL_TRIGGER_H__
#define __ONLINEGAME_GS_ITEM_SKILL_TRIGGER_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include <crc.h>

struct skilltrigger_essence 
{
	int level_require;
	int id_skill;
	int skill_level;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const skilltrigger_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, skilltrigger_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class skilltrigger_item : public item_body
{
protected:
	skilltrigger_essence  _ess;
public:
	virtual bool ArmorDecDurability(int) { return false;}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_SKILLTOME; }
	virtual void GetDurability(int &dura,int &max_dura) { dura = 100; max_dura = 100; }
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
private:
	virtual item_body* Clone() const { return  new skilltrigger_item(*this); }
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual int OnGetUseDuration() { return 0;}	//0 代表要排队，但使用时间为0
	virtual bool IsItemBroadcastUse() {return true;}
public:
	DECLARE_SUBSTANCE(skilltrigger_item);
};

#endif

