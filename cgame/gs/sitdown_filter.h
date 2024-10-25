#ifndef __ONLINEGAME_GS_SITDOWN_FILTER_H__
#define __ONLINEGAME_GS_SITDOWN_FILTER_H__
#include "filter.h"
#include "actobject.h"

class sit_down_filter : public filter
{
	int _timeout;
protected:
	sit_down_filter(){}

public:
	DECLARE_SUBSTANCE(sit_down_filter);
	sit_down_filter(gactive_imp * imp)
		:filter(object_interface(imp),FILTER_MASK_HEARTBEAT|FILTER_MASK_UNIQUE),
		_timeout(1)
	{
		_filter_id = FILTER_INDEX_SITDOWN;
	}

	
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _timeout;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _timeout;
		return true;
	}

private:
	virtual void OnAttach(){}
	virtual void OnRelease();
	virtual void Heartbeat(int tick);
};

#endif

