#ifndef __ONLINEGAME_NPC_AI_H__
#define __ONLINEGAME_NPC_AI_H__

#include "amemobj.h"
#include "aggrolist.h"
#include <common/message.h>
#include "config.h"
#include "aipolicy.h"


class gnpc_controller;
/*
	aggro_policy 负责控制仇恨策略，它提供了一个仇恨控制的接口
*/
enum
{
	AGGRO_POLICY_NORMAL 	= 0,
	AGGRO_POLICY_1		= 1,
	AGGRO_POLICY_2		= 2,
	AGGRO_POLICY_3		= 3,
	AGGRO_POLICY_BOSS	= 4,
	AGGRO_POLICY_BOSS_MINOR	= 5,
	AGGRO_POLICY_PET	= 6,
	AGGRO_POLICY_TURRET	= 7,

};

struct  aggro_param
{	
	int   aggro_policy;
	float aggro_range;
	float sight_range;
	int   aggro_time;
	int   enemy_faction;
	int   faction_ask_help;
	int   faction_accept_help;
	int   faction;
};
class aggro_policy : public substance 
{
protected:
	aggro_list _alist;
	float _aggro_range;
	int _aggro_time;	//以秒为单位
	int _cur_time;
	int _enemy_faction;
	int _faction;
	int _aggro_state;
	friend class gnpc_ai;
public:
	enum 
	{
		STATE_NORMAL,
		STATE_FREEZE,
	};
public:
	bool Save(archive & ar)
	{
		ar << _aggro_range << _aggro_time << _cur_time << _enemy_faction << _faction << _aggro_state;
		size_t size = _alist.Size();
		ar << size;
		for(size_t i=0; i < size; i ++)
		{
			XID target;
			int rage = _alist.GetEntry(i,target);
			ar << target.type << target.id << rage;
		}
		return true;
	}
	
	bool Load(archive & ar)
	{
		ar >> _aggro_range >> _aggro_time >> _cur_time >> _enemy_faction >> _faction >> _aggro_state;
		size_t size;
		ASSERT(_alist.Size() == 0);
		_alist.Clear();
		ar >> size;
		for(size_t i=0; i < size; i ++)
		{
			XID target;
			int rage;
			ar >> target.type >> target.id >> rage;
			_alist.AddRage(target,rage);
		}
		return true;
	}

DECLARE_SUBSTANCE(aggro_policy);
	aggro_policy():_alist(MAX_AGGRO_ENTRY),_aggro_range(30.f),_aggro_time(20),_enemy_faction(0),_faction(0),_aggro_state(0){}
	virtual ~aggro_policy(){}
	void Init(const aggro_param & aggp)
	{
		_aggro_range = aggp.aggro_range;
		_aggro_time = aggp.aggro_time;
		_enemy_faction = aggp.enemy_faction;
		_faction = aggp.faction;
	}

	int GetAggroState()
	{
		return _aggro_state;
	}

	void SetAggroState(int state)
	{
		_aggro_state = state;
	}

	float GetAggroRange() { return _aggro_range;}

	bool GetFirst(XID & target)
	{
		return _alist.GetFirst(target);
	}
	
	int GetEntry(int index, XID & target)
	{
		return _alist.GetEntry(index,target);
	}

	void GetAll(abase::vector<XID> & list)
	{
		return _alist.GetAll(list);
	}

	size_t Size()
	{
		return _alist.Size();
	}

	void Clear()
	{
		_cur_time = 0;
		return _alist.Clear();
	}

	void Remove(const XID & id)
	{
		if(_alist.Remove(id) == 0)
		{
			_cur_time = _aggro_time;
		}
	}
	
	bool IsEmpty(){ return _alist.IsEmpty();}

	void RefreshTimer(const XID & target)
	{
		if(_alist.IsFirst(target))
		{
			_cur_time = _aggro_time;
		}
	}

	void RegroupAggro()
	{
		_alist.RegroupAggro();
	}

	void SwapAggro(size_t index1,size_t index2)
	{
		_alist.SwapAggro(index1,index2);
	}

	void BeTaunted(const XID & target)
	{
		BeTaunted(target,1);
	}

	void FadeTarget(const XID & target)
	{
		_alist.AddToLast(target);
	}

	void AggroFade()
	{
		_alist.Fade();
	}

	int AddToFirst(const XID & target, int addon_rage)
	{
		_cur_time = _aggro_time;
		return _alist.AddToFrist(target,addon_rage);
	}
	
public:

	virtual int AddAggro(const XID & id, int rage)
	{
		if(_alist.IsEmpty())
		{
			_cur_time = _aggro_time;
		}
		return _alist.AddRage(id,rage);
	}

	virtual int AddAggro(const XID & id, int rage, int max_rage)
	{
		if(_alist.IsEmpty())
		{
			_cur_time = _aggro_time;
		}
		return _alist.AddRage(id,rage,max_rage);
	}

	virtual void OnDeath(const XID & attacker)
	{
		Clear();
	}

	virtual int AggroGen(const XID &who , int rage)
	{
		if(_alist.AddRage(who,rage) == 0)
		{
			_cur_time = _aggro_time;
		}
		return rage;
	}

    virtual int AggroRemove(const XID& src, const XID& dest, float ratio)
    {
        if (_alist.RemoveRage(src, dest, ratio) == 0)
        {
            _cur_time = _aggro_time;
            return 0;
        }

        return -1;
    }

	virtual int BeTaunted(const XID & who , int rage)
	{
		if(_alist.IsEmpty()) 
		{
			_alist.AddRage(who,rage);
		}
		else
		{
			_alist.AddToFrist(who,rage);
		}
		_cur_time = _aggro_time;
		return rage;
	}
	
	virtual int AggroGen(const MSG& msg)
	{
		ASSERT(msg.content_length == sizeof(msg_aggro_info_t));
		ASSERT(msg.source.type != -1);
		if(msg.content_length != sizeof(msg_aggro_info_t)) return 0;
		const msg_aggro_info_t * pInfo = (const msg_aggro_info_t *)msg.content;
		ASSERT(msg.source == pInfo->source);
		if(_alist.AddRage(pInfo->source,pInfo->aggro) == 0)
		{
			_cur_time = _aggro_time;
		}
		return pInfo->aggro;
	}

	virtual void OnHeartbeat()
	{
		if(_cur_time)
		{
			if(--_cur_time <= 0)
			{
				//.....
				_alist.RemoveFirst();
				//这是是否应该处理一下？？？
				XID target;
				if(_alist.GetFirst(target))
					_cur_time = _aggro_time;
				else
					_cur_time = 0;
			}
		}
	}
	
	virtual bool AggroWatch(const MSG & msg)
	{
		ASSERT(msg.content_length == sizeof(msg_watching_t));
		if(msg.content_length != sizeof(msg_watching_t)) return false;
		const msg_watching_t * pInfo = (const msg_watching_t *)msg.content;
		if(_alist.IsEmpty() && (pInfo->faction & _enemy_faction))
		{
			_alist.AddRage(msg.source,1,1);
			_cur_time = _aggro_time;
			return true;
		}
		return false;
	}


	virtual void AggroTransfer(const MSG & msg)
	{
		//do nothing
	}
	
	virtual void AggroAlarm(const MSG & msg)
	{
		
		//do nothing
	}
	virtual void AggroWakeUp(const MSG & msg)
	{
		//do nothing
	}
	
	virtual void AggroTest(const MSG & msg)
	{
		//do nothing
	}
};

class pet_aggro_policy : public aggro_policy 
{
public:

DECLARE_SUBSTANCE(pet_aggro_policy);
	pet_aggro_policy(){}

	virtual ~pet_aggro_policy(){}

	

	bool GetFirst(XID & target)
	{
		return _alist.GetFirst(target);
	}
	
	int GetEntry(int index, XID & target)
	{
		return _alist.GetEntry(index,target);
	}

	void GetAll(abase::vector<XID> & list)
	{
		return _alist.GetAll(list);
	}

	size_t Size()
	{
		return _alist.Size();
	}

	void Clear()
	{
		_cur_time = 0;
		return _alist.Clear();
	}

	void Remove(const XID & id)
	{
		if(_alist.Remove(id) == 0)
		{
			_cur_time = _aggro_time;
		}
	}
	
	bool IsEmpty(){ return _alist.IsEmpty();}

	void RefreshTimer(const XID & target)
	{
		if(_alist.IsFirst(target))
		{
			_cur_time = _aggro_time;
		}
	}

public:
	virtual int AddAggro(const XID & id, int rage)
	{
		switch(_aggro_state)
		{
			case STATE_NORMAL:
				return aggro_policy::AddAggro(id,rage);
			case STATE_FREEZE:
				return 0;
		}
		return 0;
	}

	virtual int AddAggro(const XID & id, int rage, int max_rage)
	{
		switch(_aggro_state)
		{
			case STATE_NORMAL:
				return aggro_policy::AddAggro(id,rage,max_rage);
			case STATE_FREEZE:
				return 0;
		}
		return 0;
	}

	virtual void OnDeath(const XID & attacker)
	{
		Clear();
	}

	virtual int AggroGen(const XID &who , int rage)
	{
		switch(_aggro_state)
		{
			case STATE_NORMAL:
				return aggro_policy::AggroGen(who,rage);
			case STATE_FREEZE:
				return 0;
		}
		return 0;
	}
	
	virtual int BeTaunted(const XID & who , int rage)
	{
		switch(_aggro_state)
		{
			case STATE_NORMAL:
				return aggro_policy::BeTaunted(who,rage);
			case STATE_FREEZE:
				return 0;
		}
		return 0;
	}
	
	virtual int AggroGen(const MSG& msg)
	{
		switch(_aggro_state)
		{
			case STATE_NORMAL:
				return aggro_policy::AggroGen(msg);
			case STATE_FREEZE:
				return 0;
		}
		return 0;
	}

	virtual void OnHeartbeat()
	{
		//仇恨永远不衰减....
		//不过距离参数会生效的
	}
	
	virtual bool AggroWatch(const MSG & msg)
	{
		switch(_aggro_state)
		{
			case STATE_NORMAL:
				return aggro_policy::AggroWatch(msg);
			case STATE_FREEZE:
				return 0;
		}
		return false;
	}


	virtual void AggroTransfer(const MSG & msg)
	{
		//do nothing
	}
	
	virtual void AggroAlarm(const MSG & msg)
	{
		
		//do nothing
	}
	virtual void AggroWakeUp(const MSG & msg)
	{
		//do nothing
	}
	
	virtual void AggroTest(const MSG & msg)
	{
		//do nothing
	}
};

class turret_aggro_policy : public aggro_policy 
{
public:

DECLARE_SUBSTANCE(turret_aggro_policy);
	turret_aggro_policy(){}

	virtual ~turret_aggro_policy(){}

public:
	virtual int AddAggro(const XID & id, int rage)
	{
		if(id.IsPlayerClass()) return 0;
		return aggro_policy::AddAggro(id,rage);
	}

	virtual int AddAggro(const XID & id, int rage, int max_rage)
	{
		if(id.IsPlayerClass()) return 0;
		return aggro_policy::AddAggro(id,rage,max_rage);
	}

	virtual int AggroGen(const XID &who , int rage)
	{
		if(who.IsPlayerClass()) return 0;
		return aggro_policy::AggroGen(who,rage);
	}
	
	virtual int BeTaunted(const XID & who , int rage)
	{
		return 0;
	}
	
	virtual int AggroGen(const MSG& msg)
	{
		if(msg.source.IsPlayerClass()) return 0;
		return aggro_policy::AggroGen(msg);
	}

	virtual bool AggroWatch(const MSG & msg)
	{
		return 0;
	}
};
	

/**
	这个接口是npc对象实现的一些封装,是一个proxy类
	主要供ai策略进行数据取用和操作
*/
class  ai_npcobject : public ai_actobject
{
	gnpc_controller *_ctrl;
	aggro_policy *_aggro;
	int _hate_count;
public:
	ai_npcobject(gactive_imp * imp,gnpc_controller * ctrl,aggro_policy * aggro)
			:ai_actobject(imp),_ctrl(ctrl),_aggro(aggro),_hate_count(0)
	{}

	//destructor
	virtual ai_object * Clone() const
	{
		return new ai_npcobject(*this);
	}
	virtual ~ai_npcobject()
	{
	}

public:
	//ai control
	virtual float GetIgnoreRange()
	{
		return _aggro->GetAggroRange();
	}

	virtual const XID & GetLeaderID();
	virtual void  SetLeaderID(const XID & leader_id);
	virtual const XID & GetTargetID();
	virtual void  SetTargetID(const XID & target_id);
	virtual int GetPetMaster(const XID& target);
	
public:
	//property
	virtual int GetState();

	//aggro operation
	virtual size_t GetAggroCount()
	{
		return _aggro->Size();
	}
	
	virtual void ClearAggro()
	{
		return _aggro->Clear();
	}

	virtual int GetAggroEntry(size_t index, XID & id)
	{
		return _aggro->GetEntry(index, id);
	}
	
	virtual void RemoveAggroEntry(const XID & id)
	{
		return _aggro->Remove(id);
	}

	virtual float GetSightRange();
	
	virtual void ChangeAggroEntry(const XID & id, int rage)
	{
		_aggro->AddAggro(id,rage);
	}

	virtual void RegroupAggro()
	{
		_aggro->RegroupAggro();
	}

	virtual void SwapAggro(int index1,int index2)
	{
		_aggro->SwapAggro(index1,index2);
	}

	virtual void BeTaunted(const XID & target)
	{
		_aggro->BeTaunted(target);
	}

	virtual void FadeTarget(const XID & target)
	{
		_aggro->FadeTarget(target);
	}
	
	virtual void AggroFade()
	{
		_aggro->AggroFade();
	}

	virtual void ForwardFirstAggro(const XID & id,int rage);
	virtual void HateTarget(const XID & target)
	{
		if(!_hate_count) SendMessage(target,GM_MSG_HATE_YOU);
		_hate_count = (_hate_count + 1) & 0x01;
	}

	virtual void ActiveInvisible(bool invisible)
	{
		if(invisible)
			((gnpc_imp*)_imp)->SetInvisible(1);
		else
			((gnpc_imp*)_imp)->ClearInvisible();
	}

	virtual void ClearDamageList() 
	{
		((gnpc_imp*)_imp)->ClearDamageList();
	}
	
	virtual const A3DVECTOR & GetBirthPlace()
	{
		return ((gnpc_imp*)_imp)->_birth_place;
	}
	virtual int GetInhabitType();

	virtual bool IsReturnHome(A3DVECTOR & pos, float offset_range);
	virtual void BeHurt(int hp);
	virtual void ReturnHome(const A3DVECTOR & pos,float range);
	virtual void AddInvincibleFilter(const XID & who);
	virtual bool CanRest();
	virtual bool IsInIdleHeartbeat();
	virtual void GetPatrolPos(A3DVECTOR & pos);
	virtual void GetPatrolPos(A3DVECTOR & pos,float range);
	virtual bool CheckWorld();
	virtual void Say(const XID & target, const void * msg, size_t size, int level, bool anonymous);
	virtual void BattleFactionSay(const void * msg, size_t size);
	virtual void BattleSay(const void * msg, size_t size);
	virtual void BroadcastSay(const void * msg, size_t size, bool is_system);
	virtual void SendClientTurretNotify(int id);
	virtual void PetRelocatePos(bool is_disappear);
	virtual bool PetGetNearestTeammate(float range, XID & target);
	virtual int GetLastDamage();
	virtual XID  GetChiefGainer();
	virtual XID  GetMafiaID();
	virtual void FestiveAward(int fa_type,int type,const XID & target);
	virtual void InstanceSay(const void * msg, size_t size, bool middle = false);	
	virtual void SummonMonster(int mod_id, int count, const XID& target, int target_distance, int remain_time, char die_withwho, int path_id);	
	virtual void StartPlayAction(char action_name[128], int play_times, int action_last_time, int interval_time);
	virtual void StopPlayAction();
	virtual void SetTargetCache(const XID& target);
};


class gnpc_ai : public abase::ASmallObject 
{
protected:
	gnpc_controller * _commander;
	aggro_policy	* _aggro;
	ai_policy	* _core;
	float _squared_sight_range;
	float _sight_range;
	int   _faction_ask_help;
	int   _faction_accept_help;
public:
	gnpc_ai():_commander(0),_aggro(NULL),_core(NULL),_squared_sight_range(0.f),_faction_ask_help(0){}

	bool Init(gnpc_controller * pControl,const aggro_param & aggp, const ai_param & aip);
	//int policy_class,int primary_strategy);
	virtual ~gnpc_ai()
	{
		if(_core) delete _core;
		if(_aggro) delete _aggro;
	}

	float GetSightRange() {return _sight_range;}

	void Heartbeat();
	void OnDeath(const XID & attacker)
	{
		_aggro->OnDeath(attacker);
		_core->OnDeath(attacker);
	}
	void OnDamage()
	{
		_core->OnDamage();
	}
	
	inline aggro_policy * GetAggroCtrl()
	{	
		return _aggro;
	}

	inline ai_policy * GetAICtrl()
	{
		return _core;
	}
	inline gnpc_controller * GetCommander()
	{
		return _commander;
	}
	void SetIdleMode(bool isIdle)
	{
		_core->SetIdleMode(isIdle);
	}

	void SetSealMode(int seal_flag)
	{
		_core->SetSealMode(seal_flag);
	}

	inline int GetFactionAskHelp()
	{
		return _faction_ask_help;
	}

	inline void SetLife(int life)
	{
		_core->SetLife(life);
	}

	//inline void SetDieWithLeader(bool val)
	//{
	//	_core->SetDieWithLeader(val);
	//}
	
	inline void SetDieWithWho(char val)
	{
		_core->SetDieWithWho(val);
	}
public:
	bool Save(archive & ar);
	bool Load(archive & ar);
	void ReInit(gnpc_controller * pControl)
	{
		_commander = pControl;
		if(_core)
		{
			_core->ReInit(ai_npcobject((gactive_imp*)_commander->_imp,_commander,_aggro));
		}
	}
	
	void SessionStart(int task_id, int session_id)
	{
		_core->SessionStart(task_id,session_id);
	}

	void SessionEnd(int task_id, int session_id, int retcode)
	{
		_core->SessionEnd(task_id,session_id,retcode);
	}
	void SessionUpdateChaseInfo(int task_id,const void * buf ,size_t size)
	{
		_core->SessionUpdateChaseInfo(task_id, buf, size);
	}
	
public:
	void SessionTerminate()
	{
	}

	void RefreshAggroTimer(const XID & target)
	{
		_aggro->RefreshTimer(target);
	}

	void Reborn()
	{
		_aggro->Clear();
		_core->Reborn();
	}
	
	
//仇恨度函数转发
	void AddAggro(const XID & id, int rage)
	{
		if(_aggro->AddAggro(id,rage) == 0) //第一位的才调用OnAggro
		{
			_core->OnAggro();
		}
	}

	void RawAddAggro(const XID & id, int rage)
	{
		_aggro->AddAggro(id,rage);
		_core->EnableCombat(true,true);
	}
	
	void AggroGen(const MSG& msg)
	{
		int rst;
		if((rst =_aggro->AggroGen(msg)) > 2 && _faction_ask_help)
		{
			//需要进行求救，考虑是否进行
			_commander->CryForHelp(msg.source,_faction_ask_help,_sight_range);
		}
		_core->OnAggro();
	}

	inline void TryCryForHelp(const XID & attacker)
	{
		if(_faction_ask_help)
		{
			_commander->CryForHelp(attacker,_faction_ask_help,_sight_range);
		}
	}

	
	void AggroGen(const XID & who, int rage)
	{
		int rst;
		if((rst =_aggro->AggroGen(who,rage)) > 2 && _faction_ask_help)
		{
			//需要进行求救，考虑是否进行
			_commander->CryForHelp(who,_faction_ask_help,_sight_range);
		}
		_core->OnAggro();
	}

    void AggroRemove(const XID& src, const XID& dest, float ratio)
    {
        if (_aggro->AggroRemove(src, dest, ratio) == 0)
        {
            _core->OnAggro();
        }
    }

	void BeTaunted(const XID & who,int rage)
	{
		int rst;
		if((rst =_aggro->BeTaunted(who,rage)) > 2 && _faction_ask_help)
		{
			//需要进行求救，考虑是否进行
			_commander->CryForHelp(who,_faction_ask_help,_sight_range);
		}
		_core->OnAggro();
	}
	
	void AggroWatch(const MSG & msg)
	{
		if( !_aggro->Size() && 
				_commander->_imp->_parent->pos.squared_distance(msg.pos) < _squared_sight_range)
		{
			if(_aggro->AggroWatch(msg)) 
			{
				if(_faction_ask_help)
				{
					_commander->CryForHelp(msg.source,_faction_ask_help,_sight_range);
				}
				_core->OnAggro();
			}
		}
	}

	void AggroHelp(const XID & attacker, int lamb_faction)
	{
		if((_faction_accept_help & lamb_faction) &&
				_aggro->Size() <= 1)
		{
			//最大只加两点仇恨
			if(_aggro->AddAggro(attacker,2,2) == 0)
			{
				_core->OnAggro();
			}
		}
	}
	
	void AggroTransfer(const MSG & msg)
	{
		_aggro->AggroTransfer(msg);
		_core->OnAggro();
	}
	
	void AggroAlarm(const MSG & msg)
	{
		ASSERT(msg.content_length == sizeof(msg_aggro_alarm_t));
		msg_aggro_alarm_t * agg = (msg_aggro_alarm_t*)msg.content;
		if((_faction_accept_help & agg->target_faction) && (agg->faction & _aggro->_enemy_faction))
		{
			_aggro->AggroGen(agg->attacker, agg->rage);
			_core->OnAggro();
		}
	}

	void AggroWakeUp(const MSG & msg)
	{
		_aggro->AggroWakeUp(msg);
	}
	
	void AggroTest(const MSG & msg)
	{
		_aggro->AggroTest(msg);
	}

	void AggroClear(const XID & id)
	{
		_aggro->Remove(id);	
		_core->OnAggro();
	}

	void KillTarget(const XID & target)
	{
		_core->KillTarget(target);
	}

	bool ChangeTurretMaster(const XID & master)
	{
		return _core->ChangeTurretMaster(master);
	}
	
	void ClearTurretMaster()
	{
		_core->ClearTurretMaster();
	}

	void SetFastRegen(bool b)
	{
		return _core->SetFastRegen(b);
	}

	void CheckNPCData()
	{
		_core->CheckNPCData();
	}
};

#endif
