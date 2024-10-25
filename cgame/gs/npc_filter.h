#ifndef __ONLINEGAME_GS_NPC_S_FILTER_H__
#define __ONLINEGAME_GS_NPC_S_FILTER_H__
#include "filter.h"
#include "actobject.h"

class npc_passive_filter : public timeout_filter
{
	bool _is_aggressive;
protected:
	npc_passive_filter(){}

public:
	DECLARE_SUBSTANCE(npc_passive_filter);
	npc_passive_filter(gactive_imp * imp)
		:timeout_filter(object_interface(imp),NPC_REBORN_PASSIVE_TIME,FILTER_MASK_HEARTBEAT|FILTER_MASK_REMOVE_ON_DEATH|FILTER_MASK_UNIQUE),_is_aggressive(false)
	{
		_filter_id = FILTER_INDEX_PASSIVE_WAIT;
	}
	
	virtual bool Save(archive & ar)
	{
		timeout_filter::Save(ar);
		ar << _is_aggressive;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		timeout_filter::Load(ar);
		ar >> _is_aggressive;
		return true;
	}

private:
	virtual void OnAttach();
	virtual void OnRelease();
};

#endif

