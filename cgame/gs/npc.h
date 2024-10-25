#ifndef __ONLINEGAME_GS_NPC_H__
#define __ONLINEGAME_GS_NPC_H__

#include "aggrolist.h"
#include "actobject.h"
#include <common/types.h>
#include <hashmap.h>

struct aggro_param;
struct ai_param;
class world;
class gnpc_dispatcher : public  dispatcher
{
public:
DECLARE_SUBSTANCE(gnpc_dispatcher);

public:
	virtual ~gnpc_dispatcher(){}
	virtual void begin_transfer(){}
	virtual void end_transfer(){}
	virtual void enter_slice(slice * ,const A3DVECTOR &pos);
	virtual void leave_slice(slice * ,const A3DVECTOR &pos);
	virtual void get_base_info() { }	//NPC不会自己取得自己的信息，所以无效
	virtual void enter_world();
	virtual void leave_world() { }		//要告诉其他人自己离开了
	virtual void move(const A3DVECTOR & target, int cost_time,int speed,unsigned char move_mode);
	virtual void stop_move(const A3DVECTOR & target, unsigned short speed,unsigned char dir,unsigned char move_mode);
	virtual void notify_move(const A3DVECTOR &oldpos, const A3DVECTOR & newpos) {}
	virtual void on_death(const XID & ,bool);
	virtual void disappear();
	virtual void notify_root(unsigned char type);
	virtual void dodge_attack(const XID &attacker, int skill_id, const attacker_info_t & ainfo,int at_state,char speed,bool orange,unsigned char section);
	virtual void be_damaged(const XID & id, int skill_id, const attacker_info_t & ainfo,int damage,int dura_index,int at_state,char speed, bool orange,unsigned char section);
	virtual void be_hurt(const XID & id, const attacker_info_t &,int damage,bool invader);
	virtual void query_info00(const XID & target, int cs_index,int sid);
	virtual void cast_skill(const XID & target, int skill,unsigned short time, unsigned char level);
	virtual void skill_interrupt(char reason);
	virtual void takeoff();
	virtual void landing();
	virtual void query_info_1(int uid,int cs_index, int cs_sid);
	virtual void send_turrent_leader(int id);
	virtual void level_up();
	virtual void appear_to_spec(int invi_degree);
	virtual void disappear_to_spec(int invi_degree);
	virtual void on_inc_invisible(int prev_invi_degree, int cur_invi_degree);
	virtual void on_dec_invisible(int prev_invi_degree, int cur_invi_degree);
	virtual void start_play_action(char action_name[128],int play_times,int action_last_time,int interval_time);
	virtual void stop_play_action();
	virtual void forbid_be_selected(char b);
	virtual void add_multiobj_effect(int target, char type);
	virtual void remove_multiobj_effect(int target, char type);
	virtual void notify_visible_tid_change();

public:
	virtual bool Save(archive & ar) { return true;}
	virtual bool Load(archive & ar) { return true;}
protected:
};


class gnpc_ai;
class gnpc_imp;
class gnpc_controller : public controller
{
protected:
	friend class gnpc_imp;
	int 		_svr_belong;			//NPC属于哪个服务器，对于自由转移的NPC，这个值是-1
	int 		_extern_svr;			//当前NPC处于哪个外部服务器，在本机的话，值为-1
	gnpc_ai * 	_ai_core;
	int MH_MOVEOUT(world * pPlane, const MSG & msg);
	inline gnpc * GetParent()
	{
		return (gnpc*)_imp->_parent;
	}
public:
	int		_cry_for_help_timestamp;	//用于验证是否发送求救消息的时间戳
	float		_ignore_range;			//超过这个距离，npc就将目标的仇恨消除
	XID	_cur_target_cache;				//缓存最近一个有目标策略中的指向对象
	bool _cache_refresh_state; 			//目标对象是否需要通知刷新
	
public:
DECLARE_SUBSTANCE(gnpc_controller);
	gnpc_controller();
	virtual ~gnpc_controller();
	virtual void Init(gobject_imp * imp);
public:
//	virtual int DispatchControllerMessage(world * pPlane, const MSG & msg);
	virtual int MessageHandler(world * pPlane, const MSG & msg);
	virtual int CommandHandler(int cmd_type,const void * buf, size_t size);
	virtual int MoveBetweenSlice(gobject *obj,slice * src, slice * dest);
	virtual void Release(bool free_parent = true);
	virtual void SwitchSvr(int dest,const A3DVECTOR &oldpos,const A3DVECTOR & newpos,const instance_key *switch_key);
	void DoSwitch();
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	virtual void OnHeartbeat(size_t tick);
	virtual void OnDeath(const XID & attacker);
	virtual void OnDamage();
	virtual void Reborn();
	virtual void NPCSessionStart(int task_id, int session_id);
	virtual void NPCSessionEnd(int task_id,int session_id, int retcode);
	virtual void NPCSessionUpdateChaseInfo(int task_id,const void * buf ,size_t size);

	void CryForHelp(const XID & attacker,int faction_ask_help,float sight_range);
	void TryCryForHelp(const XID & attacker);
	void AddAggro(const XID & who, int rage);
    void RemoveAggro(const XID& src, const XID& dest, float ratio);
	void BeTaunted(const XID & who, int rage);
	int GetFactionAskHelp();
	void SetLifeTime(int life);
	//void SetDieWithLeader(bool val);
	void SetDieWithWho(char val);

	void CheckNPCData();
	
public:
	bool CreateAI(const aggro_param & aggp, const ai_param & aip);
	gnpc_ai * GetAI() { return _ai_core;}
	const XID & GetCurTarget() { return _cur_target_cache; }
	void SetTargetCache(const XID& target);
	void ReleaseAI();
	gactive_imp * GetImpl() { return (gactive_imp *) _imp;}
	
	template <typename IDTYPE>
	void RefreshAggroTimer(const IDTYPE & target)
	{
		if(_ai_core) _ai_core->RefreshAggroTimer(target);
	}

	template <typename BOOLTYPE>
	void SetIdleMode(BOOLTYPE isIdle)
	{
		if(_ai_core) _ai_core->SetIdleMode(isIdle);
	}
	
	template <typename INTTYPE>
	void SetSealMode(INTTYPE seal_flag)
	{
		if(_ai_core) _ai_core->SetSealMode(seal_flag);
	}

	void SetFastRegen(bool b);

protected:
	int user_move(const void * buf, size_t size);
	inline void ActiveCombatState(bool state)
	{
		((gactive_imp*)_imp)->ActiveCombatState(state);
	}

};

class gnpc_notify : public abase::ASmallObject 
{
public:
	virtual ~gnpc_notify (){}
	virtual void OnDeath(gnpc_imp * imp){}
	virtual void OnMove(gnpc_imp * imp){}
	virtual void OnHeartbeat(gnpc_imp * impl,size_t tick){}
	virtual void OnDisappear(gnpc_imp * imp){}
	virtual void ForwardFirstAggro(gnpc_imp * imp, const XID & id, int rage) {}
	virtual int MoveOutHeartbeat(gnpc_imp * impl,int tick){ return 0;}
	virtual void OnUpdate(gnpc_imp * impl) {} 
};

class npc_spawner;
class gnpc_imp : public gactive_imp
{
protected:
	enum
	{
		LOCAL_VAL_COUNT = 3,
	};

	struct hurt_entry
	{
		int team_id;
		int team_seq;
		int damage;
		int level;
		int wallow_level;
		int profit_level;
		hurt_entry():team_id(-1),team_seq(0),damage(0),level(1){}
	};

	friend class gnpc_controller;
	friend class gnpc_dispatcher;
	gnpc_notify *_native_notify;		//通知原生对象的结构
	int _npc_state;				//NPC的状态
	typedef abase::hash_map<XID,hurt_entry,XID_HashFunc, abase::fast_alloc<> > DAMAGE_MAP;
	DAMAGE_MAP _dmg_list;
	XID 	   _first_attacker;		//第一个攻击自己的人
	int		   _first_attacked_tick;//第一次被攻击的时间,tick为单位
	int		   _total_damage;		//总共受到的伤害值
	int		   _max_damage;			//受到的最大伤害值
	int		   _last_damage;		//上次受到的伤害值
	//abase::hash_map<XID,hurt_entry,XID_HashFunc, abase::fast_alloc<> > _dmg_list;
//	aggro_list 	_dmg_list;		//伤害值列表
	XID _aggro_adj_attacker;	//受到谁的伤害时增加或减少仇恨
	int _aggro_on_damage_adj;	//受到伤害时增加或减少的仇恨百分比
	XID		   _chief_gainer_id;    //死亡获益者 roleid  gnpc_imp:OnDeath 时设置 方便策略aipolicy:OnDeath 中策略接口调用
public:
	A3DVECTOR _birth_place;
	npc_spawner * 	_spawner;
	float _money_scale;
	float _drop_rate;
	int _native_server;			//原生对象的所在地
	XID _leader_id;				//leader的id
	XID _target_id;				//策略中使用的目标，用于该npc跟随对象和死亡跟随
	XID _owner_id;				//所有者id(只能是玩家), 如存在owner则怪物只能被owner攻击
	int _damage_delay;
	char _inhabit_type;			//栖息地
	char _inhabit_mode;			//栖息地模式 
	char _after_death;			//死亡后续
	char _birth_dir;			//出生时的面朝方向
	int  _corpse_delay;			//死亡时延迟时间
	int  _petegg_id;			//如果能驯服，驯服的宠物蛋ID
	int _no_accept_player_buff;
	int _fixed_direction;
	char _knocked_back;			//被击退的标志
	char _fast_regen;
	char _regen_spawn;			//用重新生成的方式来进行 reborn
	char _drop_no_protected;		//掉落不保护
	char _drop_no_profit_limit;		//掉落受收益时间限制
	bool _collision_actived;
	bool _record_dps_rank;		//被玩家杀死后是否记入dps排行榜,存在owner时才有效	
	float _drop_mine_prob;		//掉落矿物的概率
	int _drop_mine_group;		//掉落矿物的分组
	int _local_var[LOCAL_VAL_COUNT];			//策划用特征值

	template <int>
	inline char GetMoveModeByInhabitType()
	{
		//临时版
		static char mode[] = { 0,C2S::MOVE_MASK_SKY,C2S::MOVE_MASK_WATER,C2S::MOVE_MODE_RUN, 0,0,0,};
			return mode[(size_t)_inhabit_mode];
	}
	void SetInhabitMode(char inhabit_mode);

	inline bool TestKnockBackFlag()
	{
		return _knocked_back;
	}

	void SetFastRegen(bool b);

	void ActiveCollision(bool active);

DECLARE_SUBSTANCE(gnpc_imp);
public:
	enum
	{
		NPC_STATE_NORMAL,
		NPC_STATE_WAITING_SWITCH,
		NPC_STATE_MOVEOUT,		//原生NPC已经转移到外部
		NPC_STATE_SPAWNING,		//在生成器中
	};
	int GetState() { return _npc_state;}
	void ClearDamageList()
	{
		_dmg_list.clear();
		_dmg_list.resize(20);
		_first_attacker.type = -1;
		_first_attacker.id = -1;
		_first_attacked_tick = 0;
		_total_damage = 0;
		_max_damage = 0;
		_last_damage = 0;
	}

public:
	gnpc_imp();
	~gnpc_imp();
	virtual void Init(world * pPlane,gobject*parent);
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual int ZombieMessageHandler(world * pPlane ,const MSG & msg); 
	virtual int DispatchMessage(world * pPlane ,const MSG & msg);
	virtual int DoAttack(const XID & target,char force_attack);
	

	void ForwardFirstAggro(const XID & id,int rage);
	int GetNPCID() { return ((gnpc*)_parent)->tid; }
	virtual int GetSrcMonster() { return ((gnpc*)_parent)->tid; }	//怪物的src_monster就是自己
//	virtual void SwitchSvr(int dest,const A3DVECTOR &oldpos,const A3DVECTOR & newpos);

//	void SendInfo00(const XID & target);
	//void SetDieWithLeader(bool val) { ((gnpc_controller*)_commander)->SetDieWithLeader(val);}
	void SetDieWithWho(char val) { ((gnpc_controller*)_commander)->SetDieWithWho(val);}

	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	virtual void OnHeartbeat(size_t tick);
	virtual void SendDataToSubscibeList();
	virtual void KnockBack(const XID & target, const A3DVECTOR & source, float distance,int time,int stun_time);
	virtual void PullOver(const XID & target, const A3DVECTOR & source,float distance, int time);
	virtual void Teleport2(const A3DVECTOR & pos, int time, char mode);
	virtual void SendTeamDataToSubscibeList();
	virtual void Reborn();
	virtual bool StepMove(const A3DVECTOR &offset);
	virtual bool CanMove();
	virtual void SetIdleMode(bool sleep, bool stun);
	virtual void SetSealMode(bool silent,bool root);
	virtual void SetCombatState();
	virtual void AddNPCAggro(const XID & who, int rage);
    virtual void RemoveNPCAggro(const XID& src, const XID& dest, float ratio);
	virtual void BeTaunted(const XID & who, int rage);
	virtual int GetMonsterFaction();
	virtual int GetFactionAskHelp();
	virtual void SetLifeTime(int life);
	virtual void PetRelocatePos(bool is_disappear){}
	virtual bool PetGetNearestTeammate(float range, XID & target){ return false; }
	virtual int GetLastDamage(){ return _last_damage; }
	virtual int SummonMonster(int mod_id, int count, const XID& target, int target_distance, int remain_time, char die_withwho, int path_id);	
	virtual void IncAntiInvisibleActive(int val);
	virtual void DecAntiInvisibleActive(int val);
	virtual void SetInvisible(int);
	virtual void ClearInvisible();
	virtual int OI_GetPetEggID();
	virtual void OI_TransferPetEgg(const XID & who, int pet_egg);
	virtual void OI_Disappear();
	virtual const XID & GetLeaderID(){ return _leader_id; }
	virtual void IncAggroOnDamage(const XID & attacker, int val){ _aggro_adj_attacker = attacker; _aggro_on_damage_adj += val; }
	virtual void DecAggroOnDamage(const XID & attacker, int val){ _aggro_adj_attacker = attacker; _aggro_on_damage_adj -= val; }
	virtual XID GetChiefGainer() const { return _chief_gainer_id;}
	virtual void FestiveAward(int fa_type,int type,const XID& target);
	virtual int OI_GetMafiaID();
	virtual int GetLocalVal(int index) { if(index < LOCAL_VAL_COUNT) return _local_var[index]; else return 0;}
	virtual void SetLocalVal(int index,int val) { if(index < LOCAL_VAL_COUNT) _local_var[index] = val;}	
	virtual void AdjustLocalControlID(int& cid);
	virtual int GetMazeRoomIndex();
	virtual void DeliverTaskToTarget(const XID& target, int taskid);	
	virtual int ChangeVisibleTypeId(int tid);
	virtual void SetTargetCache(const XID& target);
	virtual const XID & GetCurTarget();
	virtual void DispatchTaskToDmgList(int taskid, int count, int dis);
protected:
	virtual void DispatchExp(XID & owner, int &team,int &team_seq, int &own_level, XID & task_owner,int &max_wallow_level, int & min_profit_level);
	void DropItem(const XID & owner, int owner_level, int team_id,int team_seq, int wallow_level);
	void DropItemFromData(const XID & owner, int owner_level, int team_id,int team_seq, int wallow_level);
	bool DropItemFromGlobal(const XID & owner, int owner_level, int team_id, int team_seq, int wallow_level);//返回是否继续掉落
	void AddHurtEntry(const XID & attacker, int damage, int team,int seq,int level,int wallow_level, int profit_level);
	void  LifeExhaust();
	void AddAggroEntry(const XID & who , int faction, int level, int rage, size_t speed);
	void DropMine();

	gnpc_imp * TransformMob(int target_id);

private:
	virtual void OnDamage(const XID & attacker,int skill_id, const attacker_info_t&info,int damage,int at_state,char speed,bool orange,unsigned char section);
	virtual void OnHurt(const XID & attacker,const attacker_info_t&info,int damage,bool invader);
	
	virtual void OnDeath(const XID & lastattack,bool is_invader, char attacker_mode, int taskdead);
	virtual void OnAttacked(world *pPlane,const MSG & msg, attack_msg * attack, damage_entry & dmg,bool is_hit);

	virtual void AdjustDamage(const MSG & msg, attack_msg * attack,damage_entry &dmg,float & damage_adjust);
	virtual bool CheckInvader(world * pPlane, const XID & source);
	virtual void CheckNPCData()
	{	
		((gnpc_controller*)_commander)->CheckNPCData();
	}


};




#endif

/*
	AI policy
	Aggro Policy

**/

