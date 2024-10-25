#ifndef __ONLINEGAME_GS_POTION_FILTER_H__
#define __ONLINEGAME_GS_POTION_FILTER_H__
#include "filter.h"
#include "actobject.h"

class healing_potion_filter : public filter
{
	size_t _timeout;
	int _total_life;
	int _life_per_tick;
protected:
	healing_potion_filter(){}

public:
	DECLARE_SUBSTANCE(healing_potion_filter);
	healing_potion_filter(gactive_imp * imp,size_t timeout,int total_life)
		:filter(object_interface(imp),FILTER_MASK_HEARTBEAT|FILTER_MASK_MERGE|FILTER_MASK_REMOVE_ON_DEATH),
		_timeout(timeout),_total_life(total_life),_life_per_tick(1)
	{
		_filter_id = FILTER_INDEX_HEALING;
		ASSERT(timeout);
		if(timeout)
		{
			_life_per_tick = _total_life / timeout;
		}
	}

	virtual void OnAttach() {}
	
	virtual void Merge(filter * f) 
	{
		//ASSERT(!_is_deleted);
		if(f->GetGUID() == GetGUID())
		{
			healing_potion_filter * hpf = (healing_potion_filter *)f;
			_timeout += hpf->_timeout;
			_total_life +=  hpf->_total_life;
			_life_per_tick = _total_life / _timeout;
		}
		else
		{
			ASSERT(false);
		}
	}
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _timeout << _total_life << _life_per_tick;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _timeout >> _total_life >> _life_per_tick;
		return true;
	}

private:
	virtual void Heartbeat(int tick)
	{
		_parent.HealByPotion(_life_per_tick);
		_total_life -= _life_per_tick;
		_timeout -= tick;
		if(_timeout <=0) _is_deleted = true;
	}
	
};

class mana_potion_filter : public filter
{
	size_t _timeout;
	int _total_mana;
	int _mana_per_tick;
protected:
	mana_potion_filter(){}

public:
	DECLARE_SUBSTANCE(mana_potion_filter);
	mana_potion_filter(gactive_imp * imp,size_t timeout,int total_mana)
		:filter(object_interface(imp),FILTER_MASK_HEARTBEAT|FILTER_MASK_MERGE|FILTER_MASK_REMOVE_ON_DEATH),
		_timeout(timeout),_total_mana(total_mana),_mana_per_tick(1)
	{
		_filter_id = FILTER_INDEX_MANA_REGEN;
		ASSERT(timeout);
		if(timeout)
		{
			_mana_per_tick = _total_mana / timeout;
		}
	}
	
	virtual void OnAttach() {}
	virtual void Merge(filter * f) 
	{
		//ASSERT(!_is_deleted);
		if(f->GetGUID() == GetGUID())
		{
			mana_potion_filter * hpf = (mana_potion_filter *)f;
			_timeout += hpf->_timeout;
			_total_mana +=  hpf->_total_mana;
			_mana_per_tick = _total_mana / _timeout;
		}
		else
		{
			ASSERT(false);
		}
		
	}
	virtual bool Save(archive & ar)
	{
		filter::Save(ar);
		ar << _timeout << _total_mana << _mana_per_tick;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		filter::Load(ar);
		ar >> _timeout >> _total_mana >> _mana_per_tick;
		return true;
	}

private:
	virtual void Heartbeat(int tick)
	{
		_parent.InjectMana(_mana_per_tick);
		_total_mana -= _mana_per_tick;
		_timeout -= tick;
		if(_timeout <=0) _is_deleted = true;
	}
	
};
#endif

