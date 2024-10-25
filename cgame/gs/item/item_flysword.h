#ifndef __ONLINEGAME_GS_AEROCRAFT_H__
#define __ONLINEGAME_GS_AEROCRAFT_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"
#include <crc.h>

struct flysword_essence 
{
	int cur_time;
	int max_time;
	int require_level;
	size_t time_per_element;
	float speed_increase;
};

struct cls_flysword_essence 
{
	int cur_time;
	int max_time;
	short require_level;
	char level;
	char improve_level;
	int require_class;
	size_t time_per_element;
	float speed_increase;		//普通飞行速度
	float speed_increase2;		//高速飞行速度
};

struct angel_wing_essence
{
	int require_level;
	size_t mp_launch;
	size_t mp_per_second;
	float speed_increase;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const flysword_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, flysword_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const cls_flysword_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, cls_flysword_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const angel_wing_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, angel_wing_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class aerocraft_item : public item_body
{
public:
	virtual bool ArmorDecDurability(int) { return false;}
	virtual void OnTakeOut(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj)
	{ 
		if(l == item::BODY) 
		{
			Deactivate(l,pos,count,obj);
			obj->_filters.RemoveFilter(FILTER_FLY_EFFECT);
		}
	}
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_FLYSWORD; }
	virtual void GetDurability(int &dura,int &max_dura) { dura = 100; max_dura = 100; }
};

template <typename ESSENCE, size_t CRC_OFFSET>
class general_aerocraft_item : public aerocraft_item
{
protected:
	struct {
		ESSENCE ess;
		char name_type;
		char name_size;
		char name[MAX_USERNAME_LENGTH];
	}_ess;
	unsigned short _crc;

	void CalcCRC()
	{
		//计算crc要跳过前面的当前时间
		_crc = crc16(((unsigned char *)&_ess) + CRC_OFFSET,sizeof(ESSENCE) - CRC_OFFSET);
	}
private:
	virtual unsigned short GetDataCRC() { return _crc; }
	virtual bool Load(archive & ar)
	{
		ar >> _ess.ess >> _ess.name_type >> _ess.name_size; 
		if(_ess.name_size > MAX_USERNAME_LENGTH) _ess.name_size = MAX_USERNAME_LENGTH;
		if(_ess.name_size)
		{
			ar.pop_back(_ess.name,_ess.name_size);
		}
		CalcCRC();
		return true;
	}
	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess;
		len = sizeof(ESSENCE) + sizeof(2) + _ess.name_size;
	}

	virtual void OnActivate(item::LOCATION l,size_t pos,size_t count, gactive_imp* obj)
	{
	        obj->_en_point.flight_speed += _ess.ess.speed_increase;
	}

	virtual void OnDeactivate(item::LOCATION l,size_t pos,size_t count,gactive_imp* obj)
	{
		obj->_en_point.flight_speed -= _ess.ess.speed_increase;
		//这里不进行取消飞行效果的操作，因为更换装备时也会发生这个操作
	}

	template <typename FILTER>
	int TriggerObject(item::LOCATION l,gactive_imp * obj,int arg)
	{
		if(l != item::BODY) return -1;
		if(!IsActive()) return -2;
		if(obj->_filters.IsFilterExist(FILTER_FLY_EFFECT))
		{
			//取消飞行效果
			obj->_filters.RemoveFilter(FILTER_FLY_EFFECT);
		}
		else
		{
			//加入飞行效果
			obj->_filters.AddFilter(new FILTER(obj,FILTER_FLY_EFFECT,arg));
		}
		return 0;
	}
	
};

class flysword_item : public general_aerocraft_item<flysword_essence,offsetof(flysword_essence,max_time)>
{
private:
	virtual item_body* Clone() const { return  new flysword_item(*this); }
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj);
	virtual bool IsItemCanUse(item::LOCATION l) { return l == item::BODY && _ess.ess.cur_time > 0;}
	virtual int OnFlying(int tick);
	virtual int OnCharge(int element_level, size_t count,int & cur_time);
	virtual int OnGetUseDuration() { return 0;}	//0 代表要排队，但使用时间为0
public:
	DECLARE_SUBSTANCE(flysword_item);
};

class cls_flysword_item : public general_aerocraft_item<cls_flysword_essence,offsetof(cls_flysword_essence,max_time)>
{
private:
	virtual item_body* Clone() const { return  new cls_flysword_item(*this); }
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj);
	virtual bool IsItemCanUse(item::LOCATION l) { return l == item::BODY;}
	virtual int OnFlying(int tick);
	virtual int OnCharge(int elment_level, size_t count,int & cur_time);
	virtual int OnGetLevel();
	virtual int OnGetUseDuration() { return 0;}	//0 代表要排队，但使用时间为0
	virtual int OnGetFlyTime();
	virtual int GetImproveLevel();
	virtual bool FlyswordImprove(float speed_inc, float speed_inc2); 
	virtual bool Sign(unsigned short color, const char * signature, unsigned int signature_len);
public:
	DECLARE_SUBSTANCE(cls_flysword_item);
};

class angel_wing_item : public general_aerocraft_item<angel_wing_essence,0>
{
private:
	virtual item_body* Clone() const { return  new angel_wing_item(*this); }
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj);
	virtual bool IsItemCanUse(item::LOCATION l); 
	virtual int OnGetUseDuration() { return 0;}	//0 代表要排队，但使用时间为0
	virtual ITEM_TYPE GetItemType() {return ITEM_TYPE_WING; }
	virtual void OnActivate(item::LOCATION ,size_t pos, size_t count, gactive_imp*);
	virtual void OnDeactivate(item::LOCATION ,size_t pos, size_t count,gactive_imp*);
public:
	DECLARE_SUBSTANCE(angel_wing_item);
};
#endif

