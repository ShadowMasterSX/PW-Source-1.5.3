#ifndef __ONLINEGAME_GS_SHIELDUSER_FILTER_H__
#define __ONLINEGAME_GS_SHIELDUSER_FILTER_H__

#include "filter.h"
#include "filter_man.h"

class shielduser_filter : public filter
{
protected:
	enum 
	{
		FILTER_MASK = FILTER_MASK_WEAK
	};

	shielduser_filter(){}
public:
	DECLARE_SUBSTANCE(shielduser_filter); 
	shielduser_filter(gactive_imp * imp):filter(object_interface(imp),FILTER_MASK)
	{
		_filter_id = FILTER_INDEX_SHIELDUSER;
	}
	
	void OnAttach();
	void OnRelease();
};
#endif
