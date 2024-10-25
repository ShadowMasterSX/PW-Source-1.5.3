#ifndef __ONLINEGAME_GS_NPC_TRANSFORM_FILTER_H__
#define __ONLINEGAME_GS_NPC_TRANSFORM_FILTER_H__

#include "filter.h"

class towerbuild_filter : public filter
{
protected:
	int _time;
	int _final_form;

	enum 
	{
		MASK = 	FILTER_MASK_ADJUST_DAMAGE|FILTER_MASK_UNIQUE|FILTER_MASK_REMOVE_ON_DEATH
			|FILTER_MASK_HEARTBEAT
	};

public:
	DECLARE_SUBSTANCE(towerbuild_filter);
	towerbuild_filter(gactive_imp * imp,int filter_id,int final_form, int t)
		:filter(object_interface(imp),MASK),_time(t),_final_form(final_form)
	{
		_filter_id = filter_id;
	}

	towerbuild_filter(){}

	virtual void OnAttach() {}
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _time << _final_form;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _time >> _final_form;
		return true;
	}
	virtual void AdjustDamage(damage_entry & ent, const XID &, const attack_msg&, float);
	virtual void Heartbeat(int tick);

};


#endif


