#ifndef __GT_AWARD_FILTER_H__
#define __GT_AWARD_FILTER_H__

#include "filter.h"
#include "filter_man.h"

class gt_award_filter : public filter
{
protected:
	enum
	{
		FILTER_MASK = FILTER_MASK_WEAK
	};

	int _atk;
	int _def;
	
	virtual bool Save(archive &ar)
	{
		filter::Save(ar);
		ar << _atk << _def;
		return true;
	}

	virtual bool Load(archive &ar)
	{
		filter::Load(ar);
		ar >> _atk >> _def;
		return true;
	}
	
	gt_award_filter() {}
public:
	DECLARE_SUBSTANCE(gt_award_filter);
	gt_award_filter(gactive_imp *imp,int atk,int def):filter(object_interface(imp),FILTER_MASK),_atk(atk),_def(def)
	{
		_filter_id = FILTER_INDEX_GTAWARD;
	}
	void OnAttach();
	void OnRelease();
};
#endif
