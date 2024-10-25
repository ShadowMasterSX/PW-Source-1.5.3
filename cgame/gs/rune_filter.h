#ifndef __ONLINEGAME_GS_RUNE_FILTER_H__
#define __ONLINEGAME_GS_RUNE_FILTER_H__

#include "filter.h"

class base_defense_rune_filter : public filter
{
public:
	enum{
		RUNE_TYPE_DEFENSE,
		RUNE_TYPE_RESISTANCE,
	};
protected:
	int _times;
	float _damage_adjust;
	base_defense_rune_filter(gactive_imp * imp,int filter_id)
		:filter(object_interface(imp),FILTER_MASK_ADJUST_DAMAGE|FILTER_MASK_UNIQUE|FILTER_MASK_REMOVE_ON_DEATH|FILTER_MASK_SAVE_DB_DATA)
	{
		_filter_id = filter_id;
	}

	virtual void OnAttach() {}
	virtual void OnRelease() {}
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _times << _damage_adjust;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _times >> _damage_adjust;
		return true;
	}

	base_defense_rune_filter() {}
};

class defense_rune_filter : public base_defense_rune_filter
{
public:
	DECLARE_SUBSTANCE(defense_rune_filter);
	defense_rune_filter(gactive_imp * imp,int filter_id,int rtimes, float adjust)
		:base_defense_rune_filter(imp,filter_id)
	{
		_times = rtimes;
		_damage_adjust = adjust ;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease();
	virtual void AdjustDamage(damage_entry&, const XID &, const attack_msg&,float);
	defense_rune_filter() {}
};

class resistance_rune_filter : public base_defense_rune_filter
{
public:
	DECLARE_SUBSTANCE(resistance_rune_filter);
	resistance_rune_filter(gactive_imp * imp,int filter_id,int rtimes, float adjust)
		:base_defense_rune_filter(imp,filter_id)
	{
		_times = rtimes;
		_damage_adjust = adjust;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease();
	virtual void AdjustDamage(damage_entry&, const XID &, const attack_msg&,float);
	resistance_rune_filter() {}
};

#endif

