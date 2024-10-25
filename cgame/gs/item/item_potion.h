#ifndef __ONLINEGAME_GS_POTION_ITEM_H__
#define __ONLINEGAME_GS_POTION_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include "item_addon.h"
#include "../filter.h"
#include <crc.h>

struct  healing_potion_essence
{
	int life;
	size_t time;
	int cool_time;
	int require_level;
};

struct  mana_potion_essence
{
	int mana;
	size_t time;
	int cool_time;
	int require_level;
};

struct  rejuvenation_potion_essence
{
	int mana;
	int life;
	int cool_time;
	int require_level;
};

struct half_antidote_ess
{
	int cool_time;
	int require_level;
};

struct full_antidote_ess
{
	int cool_time;
	int require_level;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const healing_potion_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, healing_potion_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const mana_potion_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, mana_potion_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const rejuvenation_potion_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, rejuvenation_potion_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const half_antidote_ess & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, half_antidote_ess & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const full_antidote_ess & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, full_antidote_ess & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

template <typename ESSENCE>
class base_potion: public item_body
{
protected:
	ESSENCE _ess;

	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual bool IsItemSitDownCanUse(item::LOCATION l) { return true;}

public:
	base_potion()
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
		return true;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = &_ess;
		len = sizeof(_ess);
	}
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_POTION;
	}

};


class healing_potion : public base_potion<healing_potion_essence>
{
private:
	virtual item_body* Clone() const { return new healing_potion(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemBroadcastUse() {return true;}
public:
	DECLARE_SUBSTANCE(healing_potion);
private:
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
};

class mana_potion : public base_potion<mana_potion_essence>
{
private:
	virtual item_body* Clone() const { return  new mana_potion(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemBroadcastUse() {return true;}
public:
	DECLARE_SUBSTANCE(mana_potion);
private:
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
};

class rejuvenation_potion : public base_potion<rejuvenation_potion_essence>
{
private:
	virtual item_body* Clone() const { return  new rejuvenation_potion(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemBroadcastUse() {return true;}
public:
	DECLARE_SUBSTANCE(rejuvenation_potion);
private:
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
};

class half_antidote: public base_potion<half_antidote_ess>
{
private:
	virtual item_body* Clone() const { return  new half_antidote(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemBroadcastUse() {return true;}
public:
	DECLARE_SUBSTANCE(half_antidote);
private:
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
};

class full_antidote : public base_potion<full_antidote_ess>
{
private:
	virtual item_body* Clone() const { return  new full_antidote(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemBroadcastUse() {return true;}
public:
	DECLARE_SUBSTANCE(full_antidote);
private:
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
};
#endif

