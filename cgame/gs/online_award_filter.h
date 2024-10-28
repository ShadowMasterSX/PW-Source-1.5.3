
#ifndef __ONLINEGAME_GS_ONLINE_AWARD_FILTER_H__
#define __ONLINEGAME_GS_ONLINE_AWARD_FILTER_H__

#include "filter.h"
#include "filter_man.h"

class online_award_exp_filter : public filter
{
	enum
	{
		MASK = FILTER_MASK_HEARTBEAT/* | FILTER_MASK_NOSAVE*/ | FILTER_MASK_UNIQUE | FILTER_MASK_REMOVE_ON_DEATH
	};

	int _type;
	int _interval;
	int _exp;
	int _counter;

	virtual bool Save(archive & ar)
	{
//		ASSERT(false);
		filter::Save(ar);
		ar << _type << _interval << _exp << _counter;
		return true;
	}

	virtual bool Load(archive & ar)
	{
//		ASSERT(false);
		filter::Load(ar);
		ar >> _type >> _interval >> _exp >> _counter;
		return true;
	}
	
	online_award_exp_filter(){}
public:
	DECLARE_SUBSTANCE(online_award_exp_filter);
	online_award_exp_filter(gactive_imp * imp, int type, int interval, int exp)
		:filter(object_interface(imp),MASK),_type(type),_interval(interval),_exp(exp),_counter(0)
	{
		_filter_id = FILTER_INDEX_ONLINE_AWARD_EXP;
	}

	virtual void OnAttach();
	virtual void OnRelease();
	virtual void Heartbeat(int tick);
};

#endif
