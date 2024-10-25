#ifndef __ONLINEGAME_GS_ITEM_TOSSMATTER_H__
#define __ONLINEGAME_GS_ITEM_TOSSMATTER_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

struct tossmatter_essence 
{
	int require_level;
	int require_strength;
	int require_agility;
	int damage_low;
	int damage_high;
	float attack_range;
	size_t use_time;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const tossmatter_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, tossmatter_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class tossmatter_item : public item_body
{
	tossmatter_essence _ess;
	unsigned short _crc;
public:
	virtual bool ArmorDecDurability(int) { return false;}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_TOSSMATTER; }
private:
	virtual unsigned short GetDataCRC() { return _crc; }
	virtual bool Load(archive & ar)
	{
		ar >> _ess; 
		CalcCRC();
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess;
		len = sizeof(_ess);
	}
	void CalcCRC()
	{
		//计算crc要跳过前面的当前时间
		_crc = crc16(((unsigned char *)&_ess),sizeof(_ess));
	}

	virtual item_body* Clone() const { return  new tossmatter_item(*this); }
	virtual int OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual bool IsItemCanUseWithTarget(item::LOCATION l) { return true;}
	virtual int OnGetUseDuration() { return TOSSMATTER_USE_TIME;}	// 1.5*20
public:
	DECLARE_SUBSTANCE(tossmatter_item);
};
#endif

