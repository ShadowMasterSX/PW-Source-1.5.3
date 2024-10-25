#ifndef __ONLINEGAME_GS_PET_FILTERS_H__
#define __ONLINEGAME_GS_PET_FILTERS_H__
#include "filter.h"
#include "actobject.h"

class pet_damage_filter : public filter
{
	size_t _honor_level;
	enum 
	{
		MASK = FILTER_MASK_WEAK,
	};
public:
	DECLARE_SUBSTANCE(pet_damage_filter);
	pet_damage_filter(){}

	pet_damage_filter(gactive_imp * imp, size_t honor_level)
		:filter(object_interface(imp),MASK)
	{
		_honor_level = honor_level;
		_filter_id = FILTER_INDEX_PET_DAMAGE;
	}

	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _honor_level;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _honor_level;
		return true;
	}

private:
	virtual void OnAttach();
	virtual void OnRelease();
	virtual void  OnModify(int ctrlname,void * ctrlval,size_t ctrllen); 
};

#endif

