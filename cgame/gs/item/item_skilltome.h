#ifndef __ONLINEGAME_GS_ITEM_SKILL_TOME_H__
#define __ONLINEGAME_GS_ITEM_SKILL_TOME_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

//这个类已经不使用了
struct skilltome_essence 
{
	int id_skill;
	int skill_level;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const skilltome_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, skilltome_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class skilltome_item : public item_body
{
protected:
	skilltome_essence  _ess;
	unsigned short _crc;
public:
	virtual bool ArmorDecDurability(int) { return false;}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_SKILLTOME; }
	virtual void GetDurability(int &dura,int &max_dura) { dura = 100; max_dura = 100; }
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
		_crc = crc16(((unsigned char *)&_ess) ,sizeof(skilltome_essence));
	}
private:
	virtual item_body* Clone() const { return  new skilltome_item(*this); }
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool IsItemCanUse(item::LOCATION l);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual int OnGetUseDuration() { return 0;}	//0 代表要排队，但使用时间为0
public:
	DECLARE_SUBSTANCE(skilltome_item);
};

#endif

