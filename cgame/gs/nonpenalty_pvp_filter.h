#ifndef __ONLINE_GAME_GS_NONPENALTY_PVP_FILTER_H__
#define __ONLINE_GAME_GS_NONPENALTY_PVP_FILTER_H__

#include "filter.h"
#include "filter_man.h"

class nonpenalty_pvp_filter : public timeout_filter
{
	enum
	{
		MASK = FILTER_MASK_HEARTBEAT | FILTER_MASK_NOSAVE | FILTER_MASK_UNIQUE
	};

	nonpenalty_pvp_filter(){}
public:
	DECLARE_SUBSTANCE(nonpenalty_pvp_filter);
	nonpenalty_pvp_filter(gactive_imp * imp, int period)
		:timeout_filter(object_interface(imp),period,MASK)
	{
		_filter_id = FILTER_INDEX_NONPENALTY_PVP;	
	}

	virtual void OnAttach();
	virtual void OnRelease();
};

#endif
