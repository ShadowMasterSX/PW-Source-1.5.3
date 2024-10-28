#ifndef __ONLINE_GAME_GS_AI_POLICY_H__
#define __ONLINE_GAME_GS_AI_POLICY_H__

#include "substance.h"
#include <common/types.h>
#include <amemory.h>
#include <arandomgen.h>
#include "pathfinding/chaseinfo.h"
//#include "aitrigger.h"

class gnpc_ai;
class gnpc_controller;
class act_session;
class gactive_imp;
class ai_policy;
namespace ai_trigger
{
	class policy;
};

/*	class ai_object
	表示了一个具有ai行为的对象接口
	对所有对象的各种判定和操作所需的各种数据，
	都需要根据这个接口来完成
	
*/
class ai_object	: public abase::ASmallObject
{
public:
	//destructor
	virtual ai_object * Clone() const = 0;
	virtual ~ai_object() {}
public:
	virtual gactive_imp * GetImpl() = 0;
public:
	//ai control
	virtual float GetIgnoreRange() = 0;
public:
	enum
	{
		STATE_NORMAL = 0x00,
		STATE_ZOMBIE = 0x01,
	};
	//property
	virtual void GetID(XID & id) = 0;
	virtual void GetPos(A3DVECTOR & id) = 0;
	virtual int GetState() = 0;
	virtual int GetHP() = 0;
	virtual int GetMaxHP() = 0;
	virtual int GenHP(int hp) = 0;
	virtual int GetMP() = 0;
	virtual void BeHurt(int hp) = 0; //减少血值
	virtual float GetAttackRange() = 0;
	virtual float GetMagicRange(unsigned int skill, int level ) = 0;
	virtual float GetBodySize() = 0;
	virtual int GetFaction() = 0;
	virtual int GetEnemyFaction() = 0;
	virtual int GetAntiInvisibleDegree() = 0;

	//aggro operation
	virtual size_t GetAggroCount() = 0;
	virtual void ClearAggro() = 0;
	virtual int GetAggroEntry(size_t index, XID & id) = 0;
	virtual void RemoveAggroEntry(const XID & id) = 0;
	virtual void ChangeAggroEntry(const XID & id, int rage) = 0;
	virtual void ForwardFirstAggro(const XID & id,int rage) = 0;
	virtual void RegroupAggro() = 0;			//将所有敌人的仇恨度设成一样的值
	virtual void SwapAggro(int index1,int index2) = 0;	//仇恨表中交换两个位置
	virtual void BeTaunted(const XID & target) = 0;		//将某人置于仇恨列表第一位
	virtual void FadeTarget(const XID & target) = 0;	//将某人置于仇恨列表最后
	virtual void AggroFade() = 0;				//仇恨衰减50%
	virtual int GetInhabitType() = 0;

	
	virtual void HateTarget(const XID & target) = 0;
	virtual void ActiveCombatState(bool state) = 0;
	virtual bool GetCombatState() = 0;
	virtual void ActiveInvisible(bool invisible) = 0;
	virtual void ClearDamageList() {}
	virtual bool CanRest() = 0;
	virtual bool IsInIdleHeartbeat() = 0;	//是否附近无人
	virtual void GetPatrolPos(A3DVECTOR & pos) = 0;
	virtual void GetPatrolPos(A3DVECTOR & pos,float range) = 0;
	virtual const XID & GetLeaderID() = 0;
	virtual void SetLeaderID(const XID & id) = 0;
	virtual const XID & GetTargetID() = 0;
	virtual void SetTargetID(const XID & id) = 0;
	virtual float GetSightRange() = 0;
	virtual const A3DVECTOR & GetBirthPlace() = 0;
	virtual bool IsReturnHome(A3DVECTOR & pos, float offset_range) = 0;
	virtual void PetRelocatePos(bool disappear) = 0;
	virtual bool PetGetNearestTeammate(float range, XID & target) = 0;
	virtual int GetLastDamage() = 0;
	virtual XID  GetChiefGainer() = 0;
	virtual XID  GetMafiaID() = 0;
	virtual void FestiveAward(int fa_type,int type,const XID & target) = 0;
public:
	//operation
	virtual void AddSession(act_session * pSession)  = 0;
	virtual void ClearSession() = 0;
	virtual bool HasSession() = 0;
	virtual void SendMessage(const XID & id, int msg) = 0;
	virtual void ReturnHome(const A3DVECTOR & pos,float range) = 0;
	virtual void AddInvincibleFilter(const XID & who) = 0;
	virtual int GetAttackSpeed() = 0;
	virtual void Say(const XID & target, const void * msg, size_t size, int level, bool anonymous = false) = 0;
	virtual void BattleFactionSay(const void * msg, size_t size) = 0;
	virtual void BattleSay(const void * msg, size_t size) = 0;
	virtual void BroadcastSay(const void * msg, size_t size, bool is_system = false) = 0;
	virtual void SendClientTurretNotify(int id) = 0;
	//add middle to notify client whether show the msg in the middle of the screen or not
	virtual void InstanceSay(const void * msg, size_t size, bool middle = false) = 0;	
	virtual void SummonMonster(int mod_id, int count, const XID& target, int target_distance, int remain_time,char die_with_who, int path_id) = 0;	
	virtual void StartPlayAction(char action_name[128], int play_times, int action_last_time, int interval_time) = 0;
	virtual void StopPlayAction() = 0;
	virtual void SetTargetCache(const XID& target) = 0;
	virtual int GetPetMaster(const XID& target) = 0;
public:
	//global function
	struct  target_info
	{
		A3DVECTOR pos;
		int race;
		int faction;
		int level;
		int hp;
		int mp;
		float body_size;
		int max_hp;
		int invisible_degree;
	};

	enum
	{
		TARGET_STATE_NORMAL,
		TARGET_STATE_ZOMBIE,
		TARGET_STATE_NOT_FOUND,
	};

	virtual int QueryTarget(const XID & id, target_info & inf) = 0;
	virtual bool CheckWorld() = 0;

	enum
	{
		//这几个枚举的主定义在 actobject.h
		SEAL_MODE_NULL 		= 0x00,
		SEAL_MODE_ROOT 		= 0x01,
		SEAL_MODE_SILENT 	= 0x02,
	};
	virtual int GetSealMode() = 0;

	static ai_trigger::policy * CreatePolicy(int id);
};

/**
	class ai_task
	类ai_task描述了一个ai的行为模式，
	这个类的操作会执行对应模式的各种行为
*/

class ai_task : public substance
{
protected:
	ai_object * _self;
//	int padding1;
	ai_policy *_apolicy;
//	int padding2;
	int _session_id;
	CChaseInfo _chase_info;

	virtual bool Save(archive & ar)
	{
		ar << _session_id;
		ar.push_back(&_chase_info, sizeof(_chase_info));
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ar >> _session_id;
		ar.pop_back(&_chase_info, sizeof(_chase_info));
		return true;
	}

	void ClearChaseInfo()
	{
		_chase_info.HaveDispersed() = false;
	}
	friend class ai_policy;
public:
	DECLARE_SUBSTANCE(ai_task);
	enum
	{
		SE_OUT_OF_RANGE,
		SE_TARGET_DEAD,
		SE_TARGET_DISAPPEAR,
		SE_COMPLETE,
	};

	enum 
	{
		AI_TASK_TYPE_NULL,
		AI_TASK_TYPE_PET_SKILL,
	};
	ai_task():_self(0),_apolicy(0),_session_id(-1){}
	void Init(ai_object * self,ai_policy * aipolicy)
	{
		_self = self;
		_apolicy = aipolicy;
	}
	
	virtual ~ai_task(){}

	virtual int GetTaskType() { return AI_TASK_TYPE_NULL;}
	virtual void OnHeartbeat() {ASSERT(false);}
	virtual void OnAggro()
	{
		if(_self->GetAggroCount())
		{
			XID id;
			_self->GetAggroEntry(0, id);
			ChangeTarget(id);
		}
	}
	
	virtual bool StartTask() {return true;}
	virtual bool EndTask();
	virtual bool CancelTask() {return true;}
	virtual bool ChangeTarget(const XID & target)
	{
		return false;
	}
	virtual bool ChangePos(const A3DVECTOR & pos)
	{
		return false;
	}
	virtual bool DetermineTarget(XID & target);
	void SetChaseInfo(const CChaseInfo * pInfo)
	{
		if(pInfo) 
		{
			_chase_info = *pInfo;
		}
	}
public:
	//供session回调的函数
	virtual void OnSessionStart(int session_id)
	{
		_session_id = session_id;
	}
	virtual void OnSessionEnd(int session_id, int reason) {}
	virtual void OnSessionChaseInfo(const void * buf,size_t size) 
	{
		if(size == sizeof(_chase_info))
		{
			_chase_info = *(const CChaseInfo *)buf;
		}
	}
	virtual void OnDeath() {}
};

class ai_target_task : public ai_task
{
protected:
	XID _target;
	ai_target_task(){}
public:
	ai_target_task(const XID & target):_target(target){}
	virtual bool StartTask();
	virtual bool EndTask();
	virtual bool ChangeTarget(const XID & target);
	virtual void OnSessionEnd(int session_id, int reason);
	virtual void OnHeartbeat();
public:
	virtual void Execute() = 0;
	virtual bool Save(archive & ar)
	{
		ai_task::Save(ar);
		ar << _target;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_task::Load(ar);
		ar >> _target;
		return true;
	}
};

class ai_melee_task : public ai_target_task
{
protected:
	ai_melee_task(){}
public:
	ai_melee_task(const XID & target):ai_target_task(target){}
public:
	DECLARE_SUBSTANCE(ai_melee_task);
	virtual void Execute();
};

class ai_fix_melee_task : public ai_target_task
{
protected:
	ai_fix_melee_task(){}
public:
	ai_fix_melee_task(const XID & target):ai_target_task(target){}
public:
	DECLARE_SUBSTANCE(ai_fix_melee_task);
	virtual void Execute();
}; 

class ai_fix_magic_task : public ai_target_task
{
protected:
	ai_fix_magic_task(){}
public:
	ai_fix_magic_task(const XID & target):ai_target_task(target){}
public:
	DECLARE_SUBSTANCE(ai_fix_magic_task);
	virtual void Execute();
}; 

#define ST_KO_COUNT 2
class ai_range_task : public ai_target_task
{
	int _state;
	int _ko_count;
protected:
	ai_range_task():_ko_count(ST_KO_COUNT){}
public:
	DECLARE_SUBSTANCE(ai_range_task);
	ai_range_task(const XID & target):ai_target_task(target),_state(1),_ko_count(ST_KO_COUNT){}
public:
	virtual void Execute();
	virtual bool Save(archive & ar)
	{
		ai_target_task::Save(ar);
		ar << _state;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_target_task::Load(ar);
		ar >> _state;
		return true;
	}
}; 

class ai_magic_task : public ai_target_task
{
	int _skill;
	int _skill_level;
	int _state;
	float _skill_range;
	int _ko_count;
	int _skill_type;	//0 祝福 1 诅咒 2 攻击
	enum
	{
		STATE_START,
		STATE_TRACE,
		STATE_DODGE,
		STATE_MAGIC,
	};
protected:
	ai_magic_task():_ko_count(ST_KO_COUNT){}
public:
	DECLARE_SUBSTANCE(ai_magic_task);
	ai_magic_task(const XID & target):ai_target_task(target),_skill(0),
							    _state(0),_skill_range(1.f),_ko_count(ST_KO_COUNT)
	{}

	virtual bool StartTask();

public:
	virtual void Execute();
	virtual bool Save(archive & ar)
	{
		ai_target_task::Save(ar);
		ar << _skill << _skill_level << _state << _skill_range << _skill_type;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_target_task::Load(ar);
		ar >> _skill >> _skill_level >> _state >> _skill_range >> _skill_type;
		return true;
	}
};

class ai_magic_melee_task : public ai_target_task
{
	int _skill;
	int _skill_level;
	int _state;
	float _skill_range;
	int _ko_count;
	int _skill_type;
	enum
	{
		STATE_START, STATE_TRACE, STATE_DODGE, STATE_MAGIC, STATE_PHYSC,
	};
	ai_magic_melee_task():_ko_count(ST_KO_COUNT) {}
public:
	DECLARE_SUBSTANCE(ai_magic_melee_task);
	ai_magic_melee_task(const XID & target):ai_target_task(target),_skill(0),
							    _state(0),_skill_range(1.f),_ko_count(ST_KO_COUNT)
	{}

	virtual bool StartTask();

public:
	virtual void Execute();
	virtual bool Save(archive & ar)
	{
		ai_target_task::Save(ar);
		ar << _skill << _skill_level << _state << _skill_range << _skill_type;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_target_task::Load(ar);
		ar >> _skill >> _skill_level >> _state >> _skill_range >> _skill_type;
		return true;
	}
};

class ai_follow_master : public ai_target_task		//这个follow操作是专门处理跟随队长的
{
	
	ai_follow_master(){}
public:
	DECLARE_SUBSTANCE(ai_follow_master);
	ai_follow_master(const XID & target):ai_target_task(target)
	{}

public:
	virtual bool StartTask(){ return true;}
	virtual void Execute();
	virtual void OnHeartbeat();

	virtual void OnAggro();
	virtual bool EndTask();

	virtual void OnSessionEnd(int session_id, int reason);
};

class ai_follow_target : public ai_target_task	//给攻城车用的跟随操作 
{
	ai_follow_target();
public:
	DECLARE_SUBSTANCE(ai_follow_target);
	ai_follow_target(const XID & target);
	virtual ~ai_follow_target();

public:
	virtual bool StartTask();
	virtual void Execute();
	virtual void OnHeartbeat();

	virtual void OnAggro();
	virtual bool EndTask();

	virtual void OnSessionEnd(int session_id, int reason);
};

class ai_pet_follow_master : public ai_target_task		//这个follow操作是给宠物跟随主人使用的
{
	
	ai_pet_follow_master(){}
public:
	DECLARE_SUBSTANCE(ai_pet_follow_master);
	ai_pet_follow_master(const XID & target):ai_target_task(target)
	{}

public:
	virtual bool StartTask();
	virtual void Execute();
	virtual void OnHeartbeat();

	virtual void OnAggro();
	virtual bool EndTask();

	virtual void OnSessionEnd(int session_id, int reason);
};


/**
*	这类 task 以后要去除， 现在只是临时方案
*/
class ai_runaway_task : public ai_target_task
{
protected:
	ai_runaway_task(){}
public:
	DECLARE_SUBSTANCE(ai_runaway_task);
	ai_runaway_task(const XID & target):ai_target_task(target){}
	virtual bool StartTask();
	virtual bool ChangeTarget(const XID & target);
	virtual void OnSessionEnd(int session_id, int reason);
public:
	virtual void Execute() {}
};


class ai_skill_task : public ai_target_task
{
	int _skill;
	int _level;
	ai_skill_task(){}
public:
	DECLARE_SUBSTANCE(ai_skill_task);
	ai_skill_task(const XID & target,int skill, int level):ai_target_task(target),_skill(skill),_level(level){}
	virtual bool StartTask();
	virtual bool ChangeTarget(const XID & target);
	virtual void OnSessionEnd(int session_id, int reason);
public:
	virtual void Execute() {}
	virtual bool Save(archive & ar)
	{
		ai_target_task::Save(ar);
		ar << _skill << _level;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_target_task::Load(ar);
		ar >> _skill >> _level;
		return true;
	}
};

class ai_skill_task_2 : public ai_target_task
{
protected:
	int _skill;
	int _level;
	int _skill_range_type;
	float _skill_range;
	int _state;
	int _trace_count;
	ai_skill_task_2(){}
	enum
	{
		STATE_START, STATE_TRACE, STATE_DODGE, STATE_MAGIC, STATE_PHYSC,
	};
public:
	DECLARE_SUBSTANCE(ai_skill_task_2);
	ai_skill_task_2(const XID & target,int skill, int level):ai_target_task(target),_skill(skill),_level(level),_skill_range_type(-1),_skill_range(1.f),_state(STATE_START),_trace_count(3){}
	virtual bool StartTask();
	virtual bool ChangeTarget(const XID & target);
	virtual void OnSessionEnd(int session_id, int reason);
	virtual void Execute();
	virtual bool UseCoolDown() { return false;}
	virtual bool UseMp() { return false;}
	virtual void OnHeartbeat();
public:
	virtual bool Save(archive & ar)
	{
		ai_target_task::Save(ar);
		ar << _skill << _level << _skill_range_type << _skill_range << _state << _trace_count;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_target_task::Load(ar);
		ar >> _skill >> _level >> _skill_range_type >> _skill_range >> _state >> _trace_count;
		return true;
	}
};

class ai_silent_runaway_task : public ai_runaway_task
{
	ai_silent_runaway_task(){}
public:
	DECLARE_SUBSTANCE(ai_silent_runaway_task);
	ai_silent_runaway_task(const XID & target):ai_runaway_task(target){}
	virtual bool StartTask();
};


class ai_rest_task : public ai_task
{ 
	int _timeout;
public:
	DECLARE_SUBSTANCE(ai_rest_task);
	ai_rest_task():_timeout(16){}

	virtual void OnAggro();
	virtual bool StartTask();
	virtual bool EndTask();

	virtual void OnSessionEnd(int session_id, int reason);
	virtual void OnHeartbeat();
private:
	void Execute();
	virtual bool Save(archive & ar)
	{
		ai_task::Save(ar);
		ar << _timeout;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_task::Load(ar);
		ar >> _timeout;
		return true;
	}
};

class ai_silent_task : public ai_task 
{
	int _timeout;
public:
	DECLARE_SUBSTANCE(ai_silent_task);
	ai_silent_task():_timeout(5){}
	virtual void OnAggro(){}
	virtual bool StartTask(){return true;}
	virtual bool EndTask();
	virtual void OnSessionEnd(int session_id, int reason){}
	virtual void OnHeartbeat();
public:
	virtual bool Save(archive & ar)
	{
		ai_task::Save(ar);
		ar << _timeout;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_task::Load(ar);
		ar >> _timeout;
		return true;
	}
};

class ai_regeneration_task : public ai_task
{ 
	int _timer;
public:
	DECLARE_SUBSTANCE(ai_regeneration_task);
	ai_regeneration_task():_timer(20){}
	virtual void OnAggro();
	virtual bool StartTask();
	virtual bool EndTask();
	virtual void OnSessionEnd(int session_id, int reason);
	virtual void OnHeartbeat();
	virtual bool Save(archive & ar)
	{
		ai_task::Save(ar);
		ar << _timer;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_task::Load(ar);
		ar >> _timer;
		return true;
	}
};

class ai_patrol_task : public ai_task
{
	A3DVECTOR _target;
	A3DVECTOR _start;
	ai_patrol_task(){}
public:
	DECLARE_SUBSTANCE(ai_patrol_task);
	ai_patrol_task(const A3DVECTOR &pos):_target(pos){}

	virtual void OnAggro();
	virtual bool StartTask();
	virtual bool EndTask();

	virtual void OnSessionEnd(int session_id, int reason);
	virtual void OnHeartbeat();
private:
	void Execute();
	virtual bool Save(archive & ar)
	{
		ai_task::Save(ar);
		ar << _target << _start;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_task::Load(ar);
		ar >> _target >> _start;
		return true;
	}
};

class ai_returnhome_task : public ai_task
{
	A3DVECTOR _target;
	int _timeout;
	ai_returnhome_task(){}
public:
	DECLARE_SUBSTANCE(ai_returnhome_task);
	ai_returnhome_task(const A3DVECTOR &pos):_target(pos),_timeout(20){}

	virtual void OnAggro();
	virtual bool StartTask();
	virtual bool EndTask();

	virtual void OnSessionEnd(int session_id, int reason);
	virtual void OnHeartbeat();
private:
	virtual bool Save(archive & ar)
	{
		ai_task::Save(ar);
		ar << _target << _timeout;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_task::Load(ar);
		ar >> _target >> _timeout;
		return true;
	}
};

class ai_pet_skill_task : public ai_skill_task_2
{
	ai_pet_skill_task(){}
public:
	DECLARE_SUBSTANCE(ai_pet_skill_task);
	ai_pet_skill_task(const XID & target,int skill, int level,int range_type):ai_skill_task_2(target,skill,level)
	{
		_skill_range_type = range_type;
		_trace_count = 2;
	}
	virtual bool StartTask();
	virtual bool UseCoolDown() { return true;}
	virtual bool UseMp() { return true;}
	virtual int GetTaskType() { return AI_TASK_TYPE_PET_SKILL;}
public:
};

class ai_play_action_task : public ai_task 
{
	char _action_name[128];
	int _play_times;
	int _action_last_time;
	int _interval_time;
	int _timeout;
	ai_play_action_task(){}
public:
	DECLARE_SUBSTANCE(ai_play_action_task);
	ai_play_action_task(char action_name[128], int play_times, int action_last_time, int interval_time):
		_play_times(play_times),_action_last_time(action_last_time),_interval_time(interval_time)
	{
		 memcpy(_action_name,action_name,sizeof(_action_name));
		 _timeout = (play_times*(action_last_time+interval_time)-interval_time)/1000;
		 if(_timeout < 1) _timeout = 1;
	}
	virtual void OnAggro();
	virtual bool StartTask();
	virtual bool EndTask();
	virtual void OnSessionEnd(int session_id, int reason){}
	virtual void OnHeartbeat();
public:
	virtual bool Save(archive & ar)
	{
		ai_task::Save(ar);
		ar.push_back(_action_name,sizeof(_action_name));
		ar << _play_times << _action_last_time << _interval_time << _timeout;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_task::Load(ar);
		ar.pop_back(_action_name,sizeof(_action_name));
		ar >> _play_times >> _action_last_time >> _interval_time >> _timeout;
		return true;
	}
};

/**/
struct ai_param
{
	int trigger_policy;
	int policy_class;
	int event[3];
	int event_level[3];
	int primary_strategy;
	struct 
	{
		int as_count;
		int bs_count;
		int cs_count;
		struct
		{
			int id;
			int level;
		} attack_skills[8];

		struct
		{
			int id;
			int level;
		} bless_skills[8];

		struct
		{
			int id;
			int level;
		} curse_skills[8];
	}skills;

	int path_id;		 //巡逻的路线ID,如果小于等于0，则
	int path_type;		 //巡逻的类型 0 表示不循环，1表示：原路返回（循环）?表示到了终点又到开始点如此循环
	char patrol_mode;	 //是否随机巡游
	char speed_flag;	 //是否高速巡逻
	char no_auto_fight;		//不可进入战斗状态
	float max_move_range;	//最大移动范围
};


class patrol_agent : public substance 
{
public:
	DECLARE_SUBSTANCE(patrol_agent);
	virtual bool Init(int path_id,int path_type) = 0;
	virtual bool Reset() = 0;
	virtual int GetPathID() = 0;
	virtual bool GetFirstWayPoint(A3DVECTOR & pos) = 0;
	virtual bool GetNextWayPoint(A3DVECTOR & pos, bool & first_end) = 0;
	virtual bool GetCurWayPoint(A3DVECTOR & pos) = 0;
};

class ai_policy : public substance
{
protected:
	ai_object * _self;
	int _tick;
	int _task_id;
	ai_task * _cur_task;
	abase::vector<ai_task * ,abase::fast_alloc<> > _task_list;
	int _cur_event_hp;
	int _quarter_hp;
	int _event_list[3];	//75 50 25
	int _event_level[3];	//75 50 25
	int _primary_strategy;
	int _life;		//寿命按照时间计算
	struct __SKILLS_T
	{
		int count;
		int skill[8];
		int level[8];
	};
	struct __SKILLS_T _attack_skills;
	struct __SKILLS_T _bless_skills;
	struct __SKILLS_T _curse_skills;

	ai_trigger::policy * _at_policy;

	bool _idle_mode;
	bool _in_combat_mode;
	bool _patrol_mode;
	bool _fast_regen;
	bool _speed_flag;
	char _policy_flag;
	bool _no_auto_fight;
	float _max_move_range;
	patrol_agent  * _path_agent;
	
	static inline bool GetSelfSkill(const __SKILLS_T & s, int & skill, int & level)
	{
		if(s.count > 0)
		{
			int index = abase::Rand(0,s.count - 1);
			skill = s.skill[index];
			level = s.level[index];
			return true;
		}
		else
		{
			return false;
		}
	}
protected:
	virtual void  HaveRest();

public:
DECLARE_SUBSTANCE(ai_policy);

	ai_policy():_self(NULL),_tick(0),_task_id(0),_cur_task(0),_cur_event_hp(1),_quarter_hp(1),_primary_strategy(0),_idle_mode(false),_in_combat_mode(false),_patrol_mode(false),_fast_regen(true),_no_auto_fight(false),_max_move_range(0.f)
	{
		_policy_flag = 0;
		_path_agent = NULL;
		memset(_event_list,0,sizeof(int)*3);
		memset(&_attack_skills,0,sizeof(_attack_skills));
		memset(&_bless_skills,0,sizeof(_bless_skills));
		memset(&_curse_skills,0,sizeof(_curse_skills));
		_event_list[1] = 1;
		_life = 0;
		_at_policy = 0;
	}

	~ai_policy();

	void SetAITrigger(int policy_id);

	enum
	{
		STRATEGY_MELEE,
		STRATEGY_RANGE,
		STRATEGY_MAGIC,
		STRATEGY_MELEE_MAGIC,
		STRATEGY_FIX,
		STRATEGY_CRITTER,
		STRATEGY_STUB,
		STRATEGY_FIX_MAGIC,
		STRATEGY_MAX,
	};

	bool Save(archive & ar)
	{
		ar << _tick << _task_id << _cur_event_hp << _quarter_hp
		   << _event_list[0] << _event_list[1] << _event_list[2]
		   << _primary_strategy << _life;
		ar << _idle_mode <<_in_combat_mode << _patrol_mode << _fast_regen << _speed_flag;
		ar.push_back(&_attack_skills,sizeof(_attack_skills));
		ar.push_back(&_bless_skills,sizeof(_bless_skills));
		ar.push_back(&_curse_skills,sizeof(_curse_skills));

		if(_cur_task)
		{
			//保存当前的任务
			//不保存当前任务列表
			_cur_task->SaveInstance(ar);
		}
		else
			ar << -1;
		
		if(_path_agent)
		{
			_path_agent->SaveInstance(ar);
		}
		else
			ar << -1;

		//ai_task * _cur_task;
		//abase::vector<ai_task * ,abase::fast_alloc<> > _task_list;
		return true;
	}
	
	bool Load(archive & ar)
	{
		ar >> _tick >> _task_id >> _cur_event_hp >> _quarter_hp
			>> _event_list[0] >> _event_list[1] >> _event_list[2]
			>> _primary_strategy >> _life;
		ar >> _idle_mode >>_in_combat_mode >> _patrol_mode >> _fast_regen >> _speed_flag;
		ar.pop_back(&_attack_skills,sizeof(_attack_skills));
		ar.pop_back(&_bless_skills,sizeof(_bless_skills));
		ar.pop_back(&_curse_skills,sizeof(_curse_skills));

		ASSERT(_cur_task == NULL);
		ClearTask();

		int task_guid;
		ar >> task_guid;
		if(task_guid >= 0)
		{
			substance * pSub = substance::LoadInstance(task_guid,ar);
			_cur_task = substance::DynamicCast<ai_task>(pSub);
			if(!_cur_task)
			{
				delete pSub;
			}
		}

		int path_guid;
		ar >> path_guid;
		if(path_guid >= 0)
		{
			substance * pSub = substance::LoadInstance(path_guid,ar);
			if(pSub)
			{
				_path_agent = substance::DynamicCast<patrol_agent>(pSub);
				if(!_path_agent)
				{
					ASSERT(false);
					delete pSub;
				}
			}
		}

		return true;
	}
	

	virtual void Init(const ai_object & self, const ai_param & aip);
	void SetPathAgent(patrol_agent * agent)
	{
		if(_path_agent) delete _path_agent;
		_path_agent = agent;
	}
	void ChangePath(int path_id,int path_type,char speed_flag); 

	int GetPrimarySkill(int & skill, int & level)
	{
		switch(_policy_flag)
		{
			case 0:
			//第一次使用优先考虑祝福， 然后是诅咒
				if(_bless_skills.count)
				{
					GetSelfSkill(_bless_skills,skill,level);
					_policy_flag = 1;
					return 0;
				}
			case 1:
			//第二次使用优先考虑诅咒
				if(_curse_skills.count)
				{
					GetSelfSkill(_curse_skills,skill,level);
					_policy_flag = 2; 
					return 1;
				}
			case 2:
			//以后80%考虑普通技能，10%考虑诅咒 10%考虑祝福
				{
					_policy_flag = 2; 
					int r = abase::Rand(0,9);
					if(r == 8)
					{
						if(_bless_skills.count)
						{
							GetSelfSkill(_bless_skills,skill,level);
							return 0;
						}
					}
					if(_attack_skills.count == 0) r = 9;
					if(r == 9)
					{
						if(_curse_skills.count)
						{
							GetSelfSkill(_curse_skills,skill,level);
							return 1;
						}
					}
					if(!GetSelfSkill(_attack_skills,skill,level))
					{
						skill = 0;
						level = 0;
					}
				}
		}
		return 2;
	}
	
	void ReInit(const ai_object & self)
	{
		if(_self) delete _self;
		_self = self.Clone();
		if(_cur_task)
		{
			_cur_task->Init(_self,this);
		}
	}

	void SetIdleMode(bool isIdle)
	{
		//回到正常状态 无需操作
		_idle_mode = isIdle;
		if(isIdle)
		{
			//进入等待状态
			ClearTask();
		}
		else
		{
			//回到正常状态则试图进行一次攻击判定
			DeterminePolicy();
		}
	}

	void SetSealMode(int seal_flag)
	{
		//if(seal_flag & ai_object::SEAL_MODE_SILENT)
		if(seal_flag)	//定身也这样处理了
		{
			//应该清除当前任务，重新生成
			ClearTask();
		}
	}

public:
	inline bool InCombat() { return _in_combat_mode;} //policy层的检测是否在战斗状态
	inline bool IsFastRegen() { return _fast_regen;} //是否能够快速回血
	inline void SetFastRegen(bool b) { _fast_regen = b;}
	inline patrol_agent * GetPathAgent() { return _path_agent;}
	inline bool GetSpeedFlag() { return _speed_flag;}
	inline int GetTaskID()
	{
		return _task_id;
	}
	inline void SessionStart(int task_id, int session_id)
	{
		if(task_id == _task_id && _cur_task) _cur_task->OnSessionStart(session_id);
	}

	inline void SessionEnd(int task_id, int session_id,int retcode)
	{
		if(task_id == _task_id && _cur_task) _cur_task->OnSessionEnd(session_id,retcode);
	}

	inline void SessionUpdateChaseInfo(int task_id, const void * buf, size_t size)
	{
		if(task_id == _task_id && _cur_task) _cur_task->OnSessionChaseInfo(buf , size);
	}

	virtual void TaskEnd()
	{
		ASSERT(_cur_task);
		ClearCurTask();
		StartTask();
	}

	virtual bool KeepMagicCastRange()
	{
		return true;
	}
	
	inline void CancelTask()
	{
		if(_cur_task)
		{
			_cur_task->CancelTask();
			TaskEnd();
		}
	}
	
	inline void StartTask()
	{
		ASSERT(_cur_task == NULL);
		while(_task_list.size())
		{
			_cur_task = _task_list[0];
			_task_list.erase(_task_list.begin());
			_task_id ++;
			if(_cur_task->StartTask())
			{
				break;
			}
			ClearCurTask();
		}
	}
	inline bool HasNextTask()
	{
		return !_task_list.empty();
	}

	//task function
	void AddTask(ai_task * pTask)
	{
		bool rst = _task_list.empty();
		_task_list.push_back(pTask);
		if(rst) {
			if(_cur_task == NULL) StartTask();
		}
	}

	template <typename SESSION>
	void AddTask()
	{
		_self->SetTargetCache(XID(-1,-1));
		ai_task * pTask = new SESSION();
		pTask->Init(_self,this);
		AddTask(pTask);
	}

	template <typename SESSION>
	void AddTargetTask(const XID & target)
	{
		_self->SetTargetCache(target);	
		ai_task * pTask = new SESSION(target);
		pTask ->Init(_self,this);
		AddTask(pTask);
	}

	template <typename SESSION>
	void AddTargetTask(const XID & target, const CChaseInfo * pInfo)
	{
		_self->SetTargetCache(target);	
		ai_task * pTask = new SESSION(target);
		pTask ->Init(_self,this);
		pTask->SetChaseInfo(pInfo);
		AddTask(pTask);
	}

	template <typename SESSION>
	void AddPosTask(const A3DVECTOR &pos)
	{
		_self->SetTargetCache(XID(-1,-1));
		ai_task * pTask = new SESSION(pos);
		pTask ->Init(_self, this);
		AddTask(pTask);
	}

	void AddPrimaryTask(const XID & target, int strategy)
	{
		int seal_flag = _self->GetSealMode();
		if(seal_flag & ai_object::SEAL_MODE_SILENT)
		{
			ai_task * pTask = NULL;
			switch(strategy)
			{
				case STRATEGY_MELEE:
				case STRATEGY_RANGE:
				case STRATEGY_MAGIC:
				case STRATEGY_MELEE_MAGIC:
					pTask = new ai_silent_runaway_task(target);
					break;
				case STRATEGY_CRITTER:
					pTask = new ai_runaway_task(target);
					break;
				case STRATEGY_FIX:
				case STRATEGY_FIX_MAGIC:
					pTask = new ai_silent_task();
					break;
				case STRATEGY_STUB:
					//do nothing
					break;
				default:
					ASSERT(false);
			}
			if(pTask){
				pTask->Init(_self,this);
				AddTask(pTask);
			}
		}
		else
		{
			switch(strategy)
			{
				case STRATEGY_MELEE:
					AddTargetTask<ai_melee_task>(target);
					break;
				case STRATEGY_RANGE:
					AddTargetTask<ai_range_task>(target);
					break;
				case STRATEGY_MAGIC:
					AddTargetTask<ai_magic_task>(target);
					break;
				case STRATEGY_MELEE_MAGIC:
					AddTargetTask<ai_magic_melee_task>(target);
					break;
				case STRATEGY_CRITTER:
					AddTargetTask<ai_runaway_task>(target);
					break;
				case STRATEGY_FIX:
					AddTargetTask<ai_fix_melee_task>(target);
					break;
				case STRATEGY_STUB:
					//do nothing
					break;
				case STRATEGY_FIX_MAGIC:
					AddTargetTask<ai_fix_magic_task>(target);
					break;
				default:
					ASSERT(false);
			}
		}
	}
	
	void AddEventTask(const XID & target, int event_skill,int level)
	{
		ai_task * pTask = NULL;
		switch(event_skill)
		{
			case FLEE_SKILL_ID:
			//挑出逃跑事件
			pTask = new ai_runaway_task(target);
			break;

			default:
			//是使用一个技能
			pTask = new ai_skill_task(target, event_skill, level);
			break;
		}
		ASSERT(pTask);
		if(pTask)
		{
			pTask ->Init(_self, this);
			AddTask(pTask);
		}
	}

	bool IsFixStrategy()
	{
		return _primary_strategy == STRATEGY_FIX || _primary_strategy == STRATEGY_FIX_MAGIC;
	}

	void AddPetSkillTask(int skill,int level ,  const XID & who, int rangetype)
	{
		ai_task * pTask = NULL;
		pTask = new ai_pet_skill_task(who,skill,level,rangetype);
		pTask->Init(_self,this);
		AddTask(pTask);
	}

	void ClearTask()
	{
		ClearCurTask();
		clear_ptr_vector(_task_list);
	}

	void Clear()
	{
		ClearTask();
	}

	void ClearNextTask()
	{
		clear_ptr_vector(_task_list);
	}

	void CheckNPCData();
protected:
	inline void ClearCurTask()
	{
		if(_cur_task) 
		{
			delete _cur_task;
			_cur_task = NULL;
		}
	}

	bool TriggerEvent(int hp,XID & target)
	{
		size_t event = _cur_event_hp / _quarter_hp  - 1;
		while(hp < _cur_event_hp) 
		{
			_cur_event_hp -= _quarter_hp;
		}
		int rst;
		if(event < 3 && (rst = _event_list[event]))
		{
			//执行特定的策略
			if(target.id != -1)
			{
				AddEventTask(target, rst,_event_level[event]);
			}
			else
			{	
				if(DetermineTarget(target))
				{
					AddEventTask(target, rst,_event_level[event]);
				}
				else
				return false;
			}
			return true;
		}
		return false;
	}
	
	virtual void RollBack();

public:
	virtual void OnAggro();			//仇恨度变化事件
	virtual void OnHeartbeat();		//心跳
	virtual void Reborn()
	{
		Clear();
		EnableCombat(false,false);
		_cur_event_hp = _quarter_hp * 3;
		if(_path_agent)
		{
			_path_agent->Reset();
		}
	}

	virtual void OnDeath(const XID & attacker);
	virtual void OnDamage();
	virtual void SetLife(int life) { _life = life;}
	//virtual void SetDieWithLeader(bool val) {}
	virtual void SetDieWithWho(char val) {}
	virtual float GetReturnHomeRange() { return 10.f*10.f;}
	virtual float GetMaxMoveRange() { return _max_move_range*_max_move_range;}
 	void EnableCombat(bool is_combat,bool is_trigger);
	void KillTarget(const XID & target);
	virtual bool ChangeTurretMaster(const XID & target) {return false;}
	virtual void ClearTurretMaster() {}
	virtual void UpdateChaseInfo(const CChaseInfo * pInfo) {}
	virtual void FollowMasterResult(int reason) {}
	virtual void ChangeAggroState(int) {}
	virtual void ChangeStayMode(int) {}
	virtual int GetInvincibleTimeout() { return 22;}
	virtual void SetPetAutoSkill(int skill_id, int skill_level, int range_type) {}
public:
	//不应该在外部调用
	virtual void DeterminePolicy(const XID & target = XID(-1,-1));
	virtual bool DetermineTarget(XID & target);

	


};

class group_master_policy : public ai_policy 
{
public:
DECLARE_SUBSTANCE(group_master_policy);
	//现在的逻辑和aipolicy毫无区别
	
};

class group_minor_policy : public ai_policy 
{
	//bool _die_with_leader;
	char _die_with_who;
public:
	enum{
		DIE_WITH_LEADER = 0x01,	
		DIE_WITH_TARGET = 0x02,
		};
public:
DECLARE_SUBSTANCE(group_minor_policy);
	group_minor_policy():_die_with_who(0)//_die_with_leader(0)
	{}

	virtual float GetReturnHomeRange();
	virtual float GetMaxMoveRange();
	virtual void  HaveRest();
	//virtual void SetDieWithLeader(bool val) {_die_with_leader = val;}
	virtual void SetDieWithWho(char val) {_die_with_who = val;}
	virtual void OnHeartbeat();		//心跳
	
	bool Save(archive & ar)
	{
		ai_policy::Save(ar);
		//ar << _die_with_leader;
		ar << _die_with_who;
		return true;
	}

	bool Load(archive & ar)
	{
		ai_policy::Load(ar);
		//ar >> _die_with_leader;
		ar >> _die_with_who;
		return true;
	}
};

class group_boss_policy : public ai_policy 
{
protected:
	XID _enemy;
public:
DECLARE_SUBSTANCE(group_boss_policy);
	group_boss_policy():_enemy(-1,-1)
	{}

	virtual void OnHeartbeat();		//心跳
	void TryForwardAggro()
	{
		XID target;
		int rst = _self->GetAggroEntry(0,target);
		if(rst)
		{
			if(_enemy.id != target.id)
			{
				_enemy = target;
				_enemy.id = target.id;
			}
			else
				rst = 0;
			
		}
		else
		{
			if(_enemy.id != -1)
			{
				rst = 0;
				_enemy.id = -1;
				_enemy.type = -1;
			}
		}

		//发送消息 
		if(rst) _self->ForwardFirstAggro(_enemy,rst);
	}

	bool Save(archive & ar)
	{
		ai_policy::Save(ar); 
		ar << _enemy;
		return true;
	}
	
	bool Load(archive & ar)
	{
		ai_policy::Load(ar); 
		ar >> _enemy;
		return true;
	}

};


#endif

