#ifndef __ONLINEGAME_GS_ITEM_RUNE_H__
#define __ONLINEGAME_GS_ITEM_RUNE_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include "equip_item.h"

struct damage_rune_essence
{
	short rune_type;		// 0 是物理攻击 1 是法术攻击
	short require_weapon_level_min; //要求武器小于等于这个级别才能使用
	short require_weapon_level_max; //要求武器小于等于这个级别才能使用
	short enhance_damage;
};

struct armor_rune_essence
{
	int rune_type;		//没其他用，确定classid时使用， 0 是物理攻击 1 是法术攻击
	int require_player_level; //要求任务等级小于等于这个级别才能使用
	float enhance_percent;
	int reduce_times;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const damage_rune_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}       

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, damage_rune_essence & ess)
{               
	return wrapper.pop_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const armor_rune_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}       

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, armor_rune_essence & ess)
{               
	return wrapper.pop_back(&ess,sizeof(ess));
}               

template <typename ESSENCE>
class base_rune: public item_body
{
protected:
	ESSENCE _ess;

	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual bool ArmorDecDurability(int) { return false;}

public:
	base_rune()
	{
		memset(&_ess,0,sizeof(_ess));
	}

public:
	virtual bool Save(archive & ar)
	{
		ar << _ess;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ar >> _ess; 
		return ar.is_eof();
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess;
		len = sizeof(_ess);
	}

	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_RUNE;
	}
	virtual bool IsItemBroadcastUse() {return true;}

};

class item_damage_rune : public base_rune<damage_rune_essence>
{
private:
	 virtual item_body* Clone() const { return new item_damage_rune(*this); }
public: 
	 DECLARE_SUBSTANCE(item_damage_rune);
private:
	virtual bool IsItemBroadcastUse() {return false;}
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj);
	virtual bool IsItemCanUse(item::LOCATION l) { return false;}
	virtual void OnActivate(item::LOCATION ,size_t pos, size_t count, gactive_imp*);
	virtual void OnDeactivate(item::LOCATION ,size_t pos, size_t count,gactive_imp*);
	virtual void OnTakeOut(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj);
};

/*
class item_physic_rune : public base_rune<damage_rune_essence>
{
private:
	 virtual item_body* Clone() { return new item_physic_rune(*this); }
public: 
	 DECLARE_SUBSTANCE(item_physic_rune);
private:
	 virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool IsItemBroadcastUse() {return false;}
};

class item_magic_rune : public base_rune<damage_rune_essence>
{
private:
	 virtual item_body* Clone() { return new item_magic_rune(*this); }
public: 
	 DECLARE_SUBSTANCE(item_magic_rune);
private:
	 virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
	virtual bool IsItemBroadcastUse() {return false;}
};
*/

class item_defense_rune : public base_rune<armor_rune_essence>
{
private:
	 virtual item_body* Clone() const { return new item_defense_rune(*this); }
public: 
	 DECLARE_SUBSTANCE(item_defense_rune);
private:
	 virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
};

class item_resistance_rune : public base_rune<armor_rune_essence>
{
private:
	 virtual item_body* Clone() const { return new item_resistance_rune(*this); }
public: 
	 DECLARE_SUBSTANCE(item_resistance_rune);
private:
	 virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
};

#endif

