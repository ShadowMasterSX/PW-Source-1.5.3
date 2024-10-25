#ifndef __ONLINE_GAME_GS_PET_NPC_H__
#define __ONLINE_GAME_GS_PET_NPC_H__

#include "npc.h"
#include "aipolicy.h"
#include "cooldown.h"

class gplayer_imp;
class gpet_dispatcher: public gnpc_dispatcher
{
public:
	DECLARE_SUBSTANCE(gpet_dispatcher);
	virtual void on_inc_invisible(int prev_invi_degree, int cur_invi_degree);
	virtual void on_dec_invisible(int prev_invi_degree, int cur_invi_degree);
	virtual void enter_slice(slice * ,const A3DVECTOR &pos);
	virtual void leave_slice(slice * ,const A3DVECTOR &pos);
	virtual void move(const A3DVECTOR & target, int cost_time,int speed,unsigned char move_mode);
	virtual void stop_move(const A3DVECTOR & target, unsigned short speed,unsigned char dir,unsigned char move_mode);
	virtual void enter_world();
	virtual void disappear();
	virtual void enter_sanctuary();
	virtual void leave_sanctuary();
};

struct pet_leader_prop
{
	short is_pvp_enable;
	short pvp_combat_timer;
	int   mafia_id;
	int  team_count;
	int  team_efflevel;
	int  wallow_level;
	int  profit_level;
	XID  teamlist[TEAM_MEMBER_CAPACITY];
	int  team_id;
	int  team_seq;
	int  cs_index;		//leader 的 cs_index
	int  cs_sid;		//leader 的 cs_sid
	int  duel_target;	//决斗的目标对象
	int  task_mask;
	int  force_id;
	char invader_state;	//红粉名状态
	char free_pvp_mode;	
};

class gpet_imp : public  gnpc_imp
{
protected:
	typedef abase::static_multimap<int, int, abase::fast_alloc<> >  ENEMY_LIST;
	int _hp_notified;
	int _mp_notified;
	int _notify_master_counter;
	int _pet_stamp;
	int _pet_tid;
	int _honor_level;
	attack_judge _attack_hook;
	enchant_judge _enchant_hook;
	attack_fill   _attack_fill;
	enchant_fill  _enchant_fill;
	ENEMY_LIST _enemy_list;

	pet_leader_prop  _leader_data;
	cd_manager _cooldown;

	char	_leader_force_attack;
	char	_aggro_state;		//三种仇恨状态  0 被动 1 主动 2 发呆
	char	_stay_mode;		//两种跟随方式: 0 跟随，1　停留
	char 	_old_combat_state;	//原来的combat state
	char    _old_attack_monster;//原来的attack_monster
	int	_master_attack_target;
	
	int 	_peep_counter;

	struct 
	{
		int skill;
		int level;
	} _skills[16];

	abase::hash_map<int/*factionid*/,int> _leader_faction_alliance; 	//帮派同盟
	abase::hash_map<int/*factionid*/,int> _leader_faction_hostile;		//帮派敌对


public:
	void inline SetPetStamp(int stamp)
	{
		_pet_stamp = stamp;
	}

	void inline SetTID(int tid)
	{
		_pet_tid = tid;
	}

	void inline AddSkill(int skillid, int level)
	{
		for(size_t i = 0; i < 16; i ++)
		{
			if(_skills[i].skill == 0)
			{
				_skills[i].skill = skillid;
				_skills[i].level = level;
				break;
			}
		}
	}

	inline int GetSkillLevel(int skillid)
	{
		for(size_t i = 0; i <16; i ++)
		{
			if(_skills[i].skill == skillid) return _skills[i].level;
		}
		return -1;
	}

	inline void ClearSkill()
	{
		memset(_skills, 0, sizeof(_skills));
	}

	enum 
	{
		PET_AGGRO_DEFENSE = 0,
		PET_AGGRO_AUTO = 1,
		PET_AGGRO_PASSIVE = 2,
	};

	enum
	{
		PET_MOVE_FOLLOW = 0,
		PET_STAY_STAY = 1,
	};

	DECLARE_SUBSTANCE(gpet_imp);
public:
	virtual void InitSkill(){}
	virtual bool CheckCoolDown(int idx);
	virtual void SetCoolDown(int idx, int msec);
	virtual void NotifySetCoolDownToMaster(int idx, int msec);
	virtual void NotifySkillStillCoolDown(int cd_id);
	virtual bool DrainMana(int mana);
public:
	gpet_imp();
	virtual void Init(world * pPlane,gobject*parent);
	virtual void OnDeath(const XID & attacker,bool is_invader, char attacker_mode, int taskdead);
	virtual void NotifyDeathToMaster();	
	virtual int MessageHandler(world * pPlane, const MSG & msg);
	virtual int DoAttack(const XID & target,char force_attack);
	virtual void OnHeartbeat(size_t tick);
	virtual void NotifyMasterInHeartbeat();
	virtual void DispatchPlayerCommand(int target, const void * buf, size_t size);
	virtual void PetRelocatePos(bool is_disappear); 
	virtual void TryChangeInhabitMode(char leader_layer, const A3DVECTOR & leader_pos);
	virtual bool PetGetNearestTeammate(float range, XID & target);
	void AdjustDamage(const MSG & msg, attack_msg * attack,damage_entry & dmg, float & damage_adjust);
	virtual bool OI_IsPVPEnable();
	virtual bool OI_IsInPVPCombatState();
	virtual bool OI_IsInTeam();
	virtual int  OI_GetMafiaID();
	virtual bool OI_IsFactionAlliance(int fid);
	virtual bool OI_IsFactionHostile(int fid);
	virtual bool OI_IsMember(const XID & id);
	virtual void SendMsgToTeam(const MSG & msg,float range,bool exclude_self);
	virtual void FillAttackMsg(const XID & target, attack_msg & attack,int dec_arrow);
	virtual void FillEnchantMsg(const XID & target,enchant_msg & enchant);
	virtual bool OI_IsPet() { return true;}
	virtual int OI_GetTaskMask(){ return _leader_data.task_mask; }
	virtual int OI_GetForceID(){ return _leader_data.force_id; }
	virtual void AddAggroToEnemy(const XID & who,int rage);
	virtual void PeepEnemy();
	virtual void Notify_StartAttack(const XID & target,char force_attack);
	virtual void ClearInvisible();
	virtual void NotifyClearInvisibleToMaster();
	virtual void DrainLeaderHPMP(const XID & attacker, int hp, int mp);

	void SetAttackHook(attack_judge hook1,enchant_judge hook2, attack_fill fill1, enchant_fill fill2);
	void InitFromMaster(gplayer_imp * pImp);
	void SetHonorLevel(int honor_level);
	void SetAggroState(int aggro_state);
	void SetStayState(int stay_state);
	void SendPetAIState();
	bool TestSanctuary();
	
	friend class gpet_dispatcher;
};

class gpet_policy : public ai_policy
{
protected:
	CChaseInfo 	_chase_info;
	int 		_pathfind_result;	//寻路失败的记数
	int		_aggro_state;		//1代表主动寻敌
	int 		_stay_mode;		//0 跟随 1 停留
	int 		_auto_skill_id;
	int		_auto_skill_level;
	int		_auto_skill_type;
	A3DVECTOR 	_stay_pos;

protected:
	void RelocatePetPos(bool disappear = false);	//要求宠物主人重新定位宠物的位置 
	bool GatherTarget();

	bool CheckCoolDown(size_t idx);
	bool CheckMp(int skill_id, int skill_level);
	void AddAutoCastSkill(const XID & target)
	{
		AddPetSkillTask(_auto_skill_id,_auto_skill_level, target , _auto_skill_type);
	}
public:
	DECLARE_SUBSTANCE(gpet_policy);

	gpet_policy();

	void AddPetPrimaryTask(const XID & target, int strategy);
	virtual void UpdateChaseInfo(const CChaseInfo * pInfo);
	virtual void FollowMasterResult(int reason);
	virtual void ChangeAggroState(int);
	virtual void ChangeStayMode(int);
	virtual int GetInvincibleTimeout();
	virtual void SetPetAutoSkill(int skill_id, int skill_level, int range_type);
public:
	
	virtual void OnHeartbeat();
	virtual void DeterminePolicy(const XID & target);
	virtual void RollBack();
};

class gpet_imp_2 : public  gpet_imp
{
public:
	DECLARE_SUBSTANCE(gpet_imp_2);
	virtual int MessageHandler(world * pPlane, const MSG & msg);
	virtual void PeepEnemy(){}
};

class gpet_plant_imp : public gpet_imp
{
protected:
	int _suicide_skill_id;
	int _suicide_skill_level;

public:
	DECLARE_SUBSTANCE(gpet_plant_imp);	
	
public:
	virtual void InitSkill();
	virtual void NotifySetCoolDownToMaster(int idx, int msec){}
	virtual void NotifySkillStillCoolDown(int cd_id){}

public:
	gpet_plant_imp():_suicide_skill_id(0),_suicide_skill_level(0){}
	virtual void NotifyDeathToMaster();
	virtual int MessageHandler(world * pPlane, const MSG & msg);
	virtual void NotifyMasterInHeartbeat();
	virtual void DispatchPlayerCommand(int target, const void * buf, size_t size){}
	virtual void PetRelocatePos(bool is_disappear);
	virtual void MasterLayerChanged(){}
	virtual bool OI_IsPet() { return true;}	//暂时认为是吧
	virtual void NotifyClearInvisibleToMaster(){}

};

//植物的默认策略是STRATEGY_STUB
//停留状态是PET_STAY_STAY
//可接收GM_MSG_WATCHING_YOU
//攻击性植物,目标，玩家攻击目标或 其他怪或敌对阵营的人
//仇恨状态PET_AGGRO_AUTO
//防御性植物 目标，master或组队成员
//仇恨状态PET_AGGRO_PASSIVE
class gpet_plant_policy : public gpet_policy
{
public:
	DECLARE_SUBSTANCE(gpet_plant_policy);

	virtual void UpdateChaseInfo(const CChaseInfo * pInfo){}
	virtual void FollowMasterResult(int reason){}
	
public:
	virtual void OnHeartbeat();
	virtual void RollBack();
};

#endif

