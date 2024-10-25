#ifndef __ONLINE_GAME_GS_PVP_LIMIT_FILTER_H__
#define __ONLINE_GAME_GS_PVP_LIMIT_FILTER_H__

#include "filter.h"
class pvp_limit_filter : public filter
{
	int _counter;
	//可能需要记录当前安全区的指针
public:
	DECLARE_SUBSTANCE(pvp_limit_filter);
	pvp_limit_filter() {}
	pvp_limit_filter(gactive_imp * imp,int filter_id)
		:filter(object_interface(imp),
		FILTER_MASK_TRANSLATE_RECV_MSG|
		FILTER_MASK_TRANSLATE_ENCHANT|
		FILTER_MASK_TRANSLATE_SEND_MSG|
		FILTER_MASK_TRANSLATE_SEND_ENCHANT|
		FILTER_MASK_HEARTBEAT|
		FILTER_MASK_WEAK)
	{
		_filter_id = filter_id;
		_counter  = 0;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease(); 
	virtual void TranslateSendAttack(const XID & target,attack_msg & msg);
	virtual void TranslateRecvAttack(const XID & attacker,attack_msg & msg);
	virtual void TranslateEnchant(const XID & attacker,enchant_msg & msg);
	virtual void TranslateSendEnchant(const XID & attacker,enchant_msg & msg);
	virtual void Heartbeat(int tick);
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _counter;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _counter;
		return true;
	}

	
};

#endif

