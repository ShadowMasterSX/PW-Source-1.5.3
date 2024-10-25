#ifndef __ONLINE_GAME_GS_PVP_DUEL_FILTER_H__
#define __ONLINE_GAME_GS_PVP_DUEL_FILTER_H__

#include "filter.h"
#include <common/types.h>
class pvp_duel_filter : public filter
{
	XID  _target;
	int  _mode;
	bool _is_release;
public:
	DECLARE_SUBSTANCE(pvp_duel_filter);
	pvp_duel_filter() {}
	pvp_duel_filter(gactive_imp * imp,const XID & target , int filter_id)
		:filter(object_interface(imp),
		FILTER_MASK_HEARTBEAT|
		FILTER_MASK_TRANSLATE_RECV_MSG|
		FILTER_MASK_TRANSLATE_ENCHANT|
		FILTER_MASK_UNIQUE),_target(target), _mode(0), _is_release(false)
	{
		_filter_id = filter_id;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease(); 
	virtual void Heartbeat(int tick);
	virtual void TranslateEnchant(const XID & attacker,enchant_msg & msg);
	virtual void TranslateRecvAttack(const XID & attacker,attack_msg & msg);
	virtual void OnModify(int ctrlname, void * ctrlval, size_t ctrlen);
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _target << _mode << _is_release;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _target >> _mode >> _is_release;
		return true;
	}
	
};

#endif

