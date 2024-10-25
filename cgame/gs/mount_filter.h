#ifndef __ONLINEGAME_GS_MOUNT_FILTER_H__
#define __ONLINEGAME_GS_MOUNT_FILTER_H__
#include "filter.h"

class mount_filter : public filter
{
	int _mount_id;
	unsigned short _mount_color;
	float _speedup;
	float _drop_rate;
public:
	DECLARE_SUBSTANCE(mount_filter);
	mount_filter() {}
	mount_filter(gactive_imp * imp,int filter_id, int mount_id, unsigned short mount_color, float speed_up,float drop_rate)
		:filter(object_interface(imp),FILTER_MASK_ADJUST_DAMAGE|FILTER_MASK_MERGE|FILTER_MASK_REMOVE_ON_DEATH)
	{
		_filter_id = filter_id;
		_mount_id = mount_id;
		_mount_color = mount_color;
		_speedup = speed_up;
		_drop_rate = drop_rate;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease();
	virtual void AdjustDamage(damage_entry&,  const XID &, const attack_msg&,float);
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	virtual void Merge(filter * f);
};

#endif

