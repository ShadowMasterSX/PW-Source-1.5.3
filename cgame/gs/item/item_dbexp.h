#ifndef __ONLINEGAME_GS_DOUBLE_EXP_ITEM_H__
#define __ONLINEGAME_GS_DOUBLE_EXP_ITEM_H__

#include "../item.h"
#include "../config.h"

struct dbl_exp_essence 
{
	int dbl_time;
};

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & wrapper, const dbl_exp_essence & ess)
{
	return wrapper.push_back(&ess,sizeof(ess));
}

template <typename WRAPPER> WRAPPER & operator >>(WRAPPER & wrapper, dbl_exp_essence & ess)
{
	return wrapper.pop_back(&ess,sizeof(ess));
}

class item_dbl_exp : public item_body
{
protected:
	dbl_exp_essence  _ess;

	virtual item_body* Clone() const { return new item_dbl_exp(*this); }
	virtual bool ArmorDecDurability(int) { return false;}
	virtual bool IsItemCanUse(item::LOCATION l) { return true;}
	virtual int OnUse(item::LOCATION l,gactive_imp * obj,size_t count);
public:
	DECLARE_SUBSTANCE(item_dbl_exp);
	item_dbl_exp()
	{
		_ess.dbl_time = 0;
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
		return ITEM_TYPE_DBEXP;
	}
};
#endif

