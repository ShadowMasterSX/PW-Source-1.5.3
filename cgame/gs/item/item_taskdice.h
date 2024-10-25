#ifndef __ONLINEGAME_GS_TASKDICE_H__
#define __ONLINEGAME_GS_TASKDICE_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

struct taskdice_essence 
{
	int task_id;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const taskdice_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, taskdice_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class item_taskdice : public item_body
{
protected:
	taskdice_essence  _ess;

	virtual item_body* Clone() const { return new item_taskdice(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
public:
	DECLARE_SUBSTANCE(item_taskdice);
	item_taskdice()
	{
		_ess.task_id = 0;
	}

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
		return ITEM_TYPE_TASKDICE;
	}
};

#endif

