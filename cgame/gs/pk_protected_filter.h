#ifndef __ONLINE_GAME_GS_PK_PROTECTED_FILTER_H__
#define __ONLINE_GAME_GS_PK_PROTECTED_FILTER_H__

#include "filter.h"
class pk_protected_filter : public filter
{
	int _counter;
	//可能需要记录当前安全区的指针
public:
	DECLARE_SUBSTANCE(pk_protected_filter);
	pk_protected_filter() {}
	pk_protected_filter(gactive_imp * imp, int filter_id)
		:filter(object_interface(imp),
		FILTER_MASK_HEARTBEAT|
		FILTER_MASK_WEAK)
	{
		_filter_id = filter_id;
		_counter  = 0;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease(); 
	virtual void Heartbeat(int tick);
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _counter;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _counter;
		return true;
	}

	
};

#endif

