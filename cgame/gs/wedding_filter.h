#ifndef __ONLINEGAME_GS_WEDDING_FILTER_H__
#define __ONLINEGAME_GS_WEDDING_FILTER_H__

#include "filter.h"
#include "filter_man.h"

class wedding_filter : public filter
{
protected:
	enum
	{
		MASK = FILTER_MASK_HEARTBEAT | FILTER_MASK_NOSAVE | FILTER_MASK_UNIQUE
	};
	
	enum
	{
		NORMAL,
		WAIT_ESCAPE,
	};
	int _groom;
	int _bride;
	int _scene;
	int _state;
	int _timeout;

	virtual bool Save(archive & ar)
	{
		ASSERT(false);
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ASSERT(false);
		return false;
	}
	
	wedding_filter(){}
public:
	DECLARE_SUBSTANCE(wedding_filter);
	wedding_filter(gactive_imp * imp, int groom, int bride, int scene)
		:filter(object_interface(imp),MASK),_groom(groom),_bride(bride),_scene(scene),_state(NORMAL),_timeout(10)
	{
		_filter_id = FILTER_INDEX_WEDDING;		
	}

	virtual void OnAttach();
	virtual void OnRelease(); 
	virtual void Heartbeat(int tick);	
};

#endif
