#ifndef __ONLINEGAME_GS_PLAYER_INVADE_H__
#define __ONLINEGAME_GS_PLAYER_INVADE_H__

#include <common/types.h>
#include <common/base_wrapper.h>
#include "config.h"
#include "staticmap.h"
#include "property.h"
#include "faction.h"

class gplayer_imp;
class world;
class player_invade 
{
public:
	typedef abase::vector<int, abase::fast_alloc<> > DEFENDER_LIST;
	class player_invade_control
	{
		protected:
		private:
		virtual void BecomeInvader(player_invade *,const XID &who, int time) = 0;
		virtual void BecomePariah(player_invade *) = 0;
		virtual void Heartbeat(player_invade *) = 0;
		virtual void OnDeath(player_invade *, const XID & killer) = 0;
		virtual void ReducePariah(player_invade *,int t){}
		friend class player_invade;
	};

protected:

	friend class player_ctrl_normal;
	friend class player_ctrl_invade;
	friend class player_ctrl_pariah;
	friend class gplayer_imp;
	static player_invade_control * _invade_ctrl[4];	//总共是三个状态 第四个是空指针

	template <int n >
	void SetParentState(int state)
	{
		switch(state)
		{
			default:
			ASSERT(false);
			case gactive_imp::INVADER_LVL_0:
			((gplayer*)_imp->_parent)->object_state &= ~(gactive_object::STATE_INVADER|gactive_object::STATE_PARIAH);
			_imp->_faction &= ~FACTION_PARIAH;
			((gplayer*)_imp->_parent)->base_info.faction = _imp->_faction;
			break;

			case gactive_imp::INVADER_LVL_1:
			((gplayer*)_imp->_parent)->object_state &= ~gactive_object::STATE_PARIAH;
			((gplayer*)_imp->_parent)->object_state |= gactive_object::STATE_INVADER;
			break;
			
			case gactive_imp::INVADER_LVL_2:
			UpdatePariahState();
			((gplayer*)_imp->_parent)->object_state &= ~gactive_object::STATE_INVADER;
			((gplayer*)_imp->_parent)->object_state |= gactive_object::STATE_PARIAH;
			_imp->_faction |= FACTION_PARIAH;
			((gplayer*)_imp->_parent)->base_info.faction = _imp->_faction;
			((gplayer*)_imp->_parent)->pariah_state = _pariah_state;
			break;
		}
	}

	template <int n >
	void SetParentPariahState(int state)
	{
		_pariah_state = state;
		((gplayer*)_imp->_parent)->pariah_state = _pariah_state;
		_imp->_runner->pariah_rise();
	}

	template <int n>
	void SetInvaderState(int state)
	{
		_invader_state = state;
		_imp->_invader_state = state;
		SetParentState<n>(state);
	}

	gplayer_imp * _imp;
	DEFENDER_LIST _list;
	int _invader_state;
	int _invader_time;
	int _pariah_time;
	int _pariah_state;	//根据时间来决定不同红名状态
	int _kill_count;
public:
	void Swap(player_invade & rhs)
	{
		_list.swap(rhs._list);
		abase::swap(_invader_state,rhs._invader_state);
		abase::swap(_invader_time,rhs._invader_time);
		abase::swap(_pariah_time,rhs._pariah_time);
		abase::swap(_kill_count,rhs._kill_count);
		abase::swap(_pariah_state,rhs._pariah_state);
	}
	
	bool Load(archive & ar)
	{
		ar >> _invader_state >> _invader_time >> _pariah_time >> _pariah_state >> _kill_count >> _pariah_state;
		size_t size;
		ar >> size;
		for(size_t i = 0; i < size; i++)
		{
			int id;
			ar >> id;
			_list.push_back(id);
		}
		return true;
	}

	bool Save(archive & ar)
	{
		ar << _invader_state << _invader_time << _pariah_time << _pariah_state << _kill_count << _pariah_state;
		ar <<(size_t)_list.size();
		for(size_t i = 0; i < _list.size(); i ++)
		{
			ar << _list[i];
		}
		return true;
	}

public:
	player_invade():_imp(NULL),_invader_state(0),_invader_time(0),_pariah_time(0),_kill_count(0)
	{
		_list.reserve(MAX_INVADER_ENTRY_COUNT);
	}

	void Init(gplayer_imp * imp)
	{
		_imp = imp;
	}

	void SetState(int state, int invader_time, int pariah_time)
	{
		_invader_state = state;
		_invader_time = invader_time;
		_pariah_time = pariah_time;
		UpdatePariahState();
	}


	inline bool IsDefender(int id)
	{
		for(size_t i = 0; i < _list.size();i ++)
		{
			if(_list[i] == id)
			{	
				return true;
			}
		}
		return false;
	}

	inline void BecomeInvader(const XID & who, int time)
	{
		_invade_ctrl[_invader_state]->BecomeInvader(this,who,time);
	}

	inline void BecomePariah()
	{
		_invade_ctrl[_invader_state]->BecomePariah(this);
	}

	inline void OnHeartbeat()
	{
		_invade_ctrl[_invader_state]->Heartbeat(this);
	}

	inline void OnDeath(const XID & killer)
	{
		//考虑死亡损失
		_invade_ctrl[_invader_state]->OnDeath(this,killer);
	}

	inline void ReducePariah(int t)
	{
		_invade_ctrl[_invader_state]->ReducePariah(this,t);
	}

	void ClientGetPariahTime();

protected:
	void UpdatePariahState();
	inline 	void InsertDefender(const XID & who)
	{
		int id = who.id;
		for(size_t i = 0; i < _list.size();i ++)
		{
			if(_list[i] == id)
			{	
				return;
			}
		}
		if(_list.size() >= MAX_INVADER_ENTRY_COUNT)
		{ 
			_list.erase(_list.begin());
		}
		_list.push_back(id);
	}

	inline 	void ClearDefender()
	{
		_list.clear();
	}

};


#endif

