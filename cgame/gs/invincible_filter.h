#ifndef __ONLINEGAME_GS_INVINCIBLE_FILTER_H__
#define __ONLINEGAME_GS_INVINCIBLE_FILTER_H__
#include "filter.h"

class invincible_filter : public filter
{
protected:
	bool _isAggressive;
	char _immune;
	short _timeout;
	enum 
	{
		MASK = FILTER_MASK_HEARTBEAT | FILTER_MASK_TRANSLATE_RECV_MSG | FILTER_MASK_TRANSLATE_ENCHANT
			| FILTER_MASK_UNIQUE | FILTER_MASK_REMOVE_ON_DEATH
	};
public:
	DECLARE_SUBSTANCE(invincible_filter);
	invincible_filter(gactive_imp * imp,int filter_id,short timeout = -1, char immune = false)
		:filter(object_interface(imp),MASK),_immune(immune),_timeout(timeout)
	{
		_filter_id = filter_id;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease(); 
	virtual void TranslateRecvAttack(const XID & attacker,attack_msg & msg);
	virtual void TranslateEnchant(const XID & attacker,enchant_msg & msg);
	virtual void Heartbeat(int tick);
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _isAggressive << _immune << _timeout;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _isAggressive >> _immune >> _timeout;
		return true;
	}
	
	invincible_filter() {}
};

class invincible_filter_2 : public invincible_filter
{
public:
	DECLARE_SUBSTANCE(invincible_filter_2);
	invincible_filter_2(gactive_imp * imp,int filter_id,short timeout = -1, char immune = false)
		: invincible_filter(imp,filter_id,timeout,immune) {}
	
protected:
	virtual void OnAttach();
	virtual void OnRelease(); 
	invincible_filter_2() {}
};

class invincible_filter_to_spec_id : public timeout_filter
{
	XID  _who;
public:
	DECLARE_SUBSTANCE(invincible_filter_to_spec_id);
	invincible_filter_to_spec_id(gactive_imp * imp,int filter_id, int timeout,const XID & who)
		:timeout_filter(object_interface(imp), timeout,FILTER_MASK_TRANSLATE_RECV_MSG|FILTER_MASK_TRANSLATE_ENCHANT|FILTER_MASK_REMOVE_ON_DEATH|FILTER_MASK_HEARTBEAT),_who(who)
	{
		_filter_id = filter_id;
	}

protected:
	virtual void OnAttach() {}
	virtual void TranslateRecvAttack(const XID & attacker,attack_msg & msg);
	virtual void TranslateEnchant(const XID & attacker,enchant_msg & msg);
	virtual bool Save(archive & ar)
	{
		timeout_filter::Save(ar);
		ar << _who;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		timeout_filter::Load(ar);
		ar >> _who;
		return true;
	}
	
	invincible_filter_to_spec_id() {}
};

class invincible_banish_filter : public filter
{
	int _timeout;
	enum 
	{
		MASK = FILTER_MASK_HEARTBEAT | FILTER_MASK_TRANSLATE_RECV_MSG | FILTER_MASK_TRANSLATE_ENCHANT
			| FILTER_MASK_UNIQUE | FILTER_MASK_REMOVE_ON_DEATH
	};
public:
	DECLARE_SUBSTANCE(invincible_banish_filter);
	invincible_banish_filter(gactive_imp * imp,int filter_id,int timeout = -1)
		:filter(object_interface(imp),MASK),_timeout(timeout)
	{
		_filter_id = filter_id;
	}

protected:
	virtual void OnAttach();
	virtual void OnRelease(); 
	virtual void TranslateRecvAttack(const XID & attacker,attack_msg & msg);
	virtual void TranslateEnchant(const XID & attacker,enchant_msg & msg);
	virtual void Heartbeat(int tick);
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _timeout;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _timeout;
		return true;
	}
	
	invincible_banish_filter() {}
};
#endif

