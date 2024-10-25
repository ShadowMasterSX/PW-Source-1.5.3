#ifndef __ONLINEGAME_GS_EFFECT_FILTER_H__
#define __ONLINEGAME_GS_EFFECT_FILTER_H__
#include "filter.h"

class player_effect_filter : public timeout_filter
{
	short _effect;
	enum 
	{
		MASK = FILTER_MASK_HEARTBEAT | FILTER_MASK_SAVE_DB_DATA | FILTER_MASK_UNIQUE 
	};
public:
	DECLARE_SUBSTANCE(player_effect_filter);
	player_effect_filter(gactive_imp * imp,int filter_id,int timeout, short effect)
		:timeout_filter(object_interface(imp),timeout,MASK),_effect(effect)
	{
		_filter_id = filter_id;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease(); 
	virtual bool Save(archive & ar)
	{
		timeout_filter::Save(ar);
		ar << _effect;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		timeout_filter::Load(ar);
		ar >> _effect;
		return true;
	}
	
	player_effect_filter() {}
};

#endif

