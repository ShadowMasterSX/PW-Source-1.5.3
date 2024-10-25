#ifndef __ONLINEGAME_GS_TRAVEL_FILTER_H__
#define __ONLINEGAME_GS_TRAVEL_FILTER_H__
#include "filter.h"

class travel_filter : public filter
{
	enum
	{
		MASK = FILTER_MASK_TRANSLATE_RECV_MSG | FILTER_MASK_TRANSLATE_ENCHANT |
			FILTER_MASK_TRANSLATE_SEND_MSG | FILTER_MASK_HEARTBEAT | 
			FILTER_MASK_UNIQUE | FILTER_MASK_REMOVE_ON_DEATH
	};
	int _min_time;
	int _max_time;
	A3DVECTOR _target;
	float _speed;
	int _vehicle;
	int _line_no;
public:
	travel_filter() {}
	DECLARE_SUBSTANCE(travel_filter);
	travel_filter(gactive_imp * imp,int filter_id,int min_time,int max_time,
			const A3DVECTOR & target,float speed, int vehicle,int line_no)
		:filter(object_interface(imp),MASK),_min_time(min_time),_max_time(max_time),_target(target),_speed(speed),_vehicle(vehicle),_line_no(line_no)
	{
		_filter_id = filter_id;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease(); 
	virtual void Heartbeat(int tick);
	virtual void TranslateSendAttack(const XID & target,attack_msg & msg);
	virtual void TranslateRecvAttack(const XID & attacker,attack_msg & msg);
	virtual void TranslateEnchant(const XID & attacker,enchant_msg & msg);
	virtual void OnModify(int ctrlname,void * ctrlval,size_t ctrllen);
public:
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _min_time << _max_time << _target << _speed << _vehicle << _line_no;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _min_time >> _max_time >> _target >> _speed >> _vehicle >> _line_no;
		return true;
	}
};
#endif

