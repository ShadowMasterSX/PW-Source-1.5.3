#ifndef __ONLINEGAME_GS_SKILL_FILTER_H__
#define __ONLINEGAME_GS_SKILL_FILTER_H__
#include "filter.h"

class skill_interrupt_filter : public filter
{
	int _session_id;
public:
	DECLARE_SUBSTANCE(skill_interrupt_filter);
	skill_interrupt_filter(gactive_imp * imp,int session_id,int filter_id)
		:filter(object_interface(imp),FILTER_MASK_ADJUST_DAMAGE|FILTER_MASK_UNIQUE|FILTER_MASK_REMOVE_ON_DEATH)
	{
		_filter_id = filter_id;
		_session_id = session_id;
	}

protected:
	virtual void OnAttach() {}
	virtual void AdjustDamage(damage_entry&,  const XID &, const attack_msg&,float);
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	skill_interrupt_filter() {}
};

class gather_interrupt_filter : public filter
{
	int _session_id;
public:
	DECLARE_SUBSTANCE(gather_interrupt_filter);
	gather_interrupt_filter(gactive_imp * imp,int session_id,int filter_id)
		:filter(object_interface(imp),FILTER_MASK_TRANSLATE_RECV_MSG|FILTER_MASK_UNIQUE|FILTER_MASK_REMOVE_ON_DEATH)
	{
		_filter_id = filter_id;
		_session_id = session_id;
	}

protected:
	virtual void OnAttach() {}
	virtual void TranslateRecvAttack(const XID & attacker,attack_msg & msg);
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	gather_interrupt_filter() {}
};

class moving_skill_interrupt_filter : public filter
{
	int _action_id;
public:
	DECLARE_SUBSTANCE(moving_skill_interrupt_filter);
	moving_skill_interrupt_filter(gactive_imp * imp, int action_id, int filter_id)
		:filter(object_interface(imp),FILTER_MASK_ADJUST_DAMAGE|FILTER_MASK_UNIQUE|FILTER_MASK_REMOVE_ON_DEATH)
	{
		_filter_id = filter_id;
		_action_id = action_id;
	}

protected:
	virtual void OnAttach() {}
	virtual void AdjustDamage(damage_entry&,  const XID &, const attack_msg&,float);
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	moving_skill_interrupt_filter() {}
};
#endif

