#ifndef __ONLINEGAME_GS_INVISIBLE_FILTER_H__
#define __ONLINEGAME_GS_INVISIBLE_FILTER_H__

#include "filter.h"
#include "filter_man.h"

class invisible_filter : public filter
{
protected:
	enum
	{
		MASK = FILTER_MASK_HEARTBEAT | FILTER_MASK_REMOVE_ON_DEATH | FILTER_MASK_UNIQUE | FILTER_MASK_TRANSLATE_SEND_MSG | FILTER_MASK_TRANSLATE_SEND_ENCHANT | FILTER_MASK_DO_ENCHANT | FILTER_MASK_ADJUST_DAMAGE 
	};

	int _protected_timeout;
	int _timeout;
	int _mana_per_second;
	int _invisible_degree;
	int _speeddown;

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _protected_timeout >> _timeout >> _mana_per_second >> _invisible_degree >> _speeddown;
		return true;
	}

	virtual bool Save(archive & ar)
	{
		filter::Save(ar);	
		ar << _protected_timeout << _timeout << _mana_per_second << _invisible_degree << _speeddown;
		return true;
	}

	invisible_filter(){}
public:
	DECLARE_SUBSTANCE(invisible_filter);
	invisible_filter(object_interface parent,int time,int mana_per_second, int invisible_degree, int speeddown)
		:filter(parent,MASK),_protected_timeout(5),_timeout(time),_mana_per_second(mana_per_second),_invisible_degree(invisible_degree),_speeddown(speeddown)
	{
		_filter_id = FILTER_INDEX_INVISIBLE;
	}

	virtual void Heartbeat(int tick);	
	virtual void TranslateSendAttack(const XID & target,attack_msg & msg);
	virtual void TranslateSendEnchant(const XID & target,enchant_msg & msg);
	virtual void DoEnchant(const XID & attacker,enchant_msg & msg);
	virtual void AdjustDamage(damage_entry&,  const XID &, const attack_msg&,float);

	virtual void OnAttach(); 

	virtual void OnRelease();

};

//gm ÒþÉí+ÎÞµÐfilter
class gm_invisible_filter : public filter
{
protected:
	enum
	{
		MASK = FILTER_MASK_UNIQUE | FILTER_MASK_HEARTBEAT | FILTER_MASK_TRANSLATE_RECV_MSG | FILTER_MASK_TRANSLATE_ENCHANT 
	};
	
	int _timeout;

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _timeout;
		return true;
	}

	virtual bool Save(archive & ar)
	{
		filter::Save(ar);	
		ar << _timeout;
		return true;
	}

	gm_invisible_filter(){}
public:
	DECLARE_SUBSTANCE(gm_invisible_filter);
	gm_invisible_filter(object_interface parent,int time=-1, int mask=0)
		:filter(parent,MASK|mask),_timeout(time)
	{
		_filter_id = FILTER_INDEX_GM_INVISIBLE;
	}

	virtual void Heartbeat(int tick);	
	virtual void TranslateRecvAttack(const XID & attacker,attack_msg & msg);
	virtual void TranslateEnchant(const XID & attacker,enchant_msg & msg);

	virtual void OnAttach(); 
	virtual void OnRelease();
};
#endif

