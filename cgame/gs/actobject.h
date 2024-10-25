#ifndef __ONLINEGAME_GS_ACTIVEOBJ_H__
#define __ONLINEGAME_GS_ACTIVEOBJ_H__

#include "config.h"
#include "gimp.h"
#include "object.h"
#include "attack.h"
#include "property.h"
#include "filter_man.h"
#include "aipolicy.h"
#include "skillwrapper.h"
#include <timer.h>
#include <arandomgen.h>
#include <common/protocol.h>
#include <glog.h>
#include "sfilterdef.h"
#include "moving_action_env.h"

//lgc
#pragma pack(1)
struct elf_enhance
{
	short str_point;
	short agi_point;
	short vit_point;
	short eng_point;

	short str_scale;
	short agi_scale;
	short vit_scale;
	short eng_scale;
};
#pragma pack()

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const struct elf_enhance & en)
{
	wrapper.push_back(&en, sizeof(en));
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, struct elf_enhance & en)
{
	wrapper.pop_back(&en, sizeof(en));
	return wrapper;
}



namespace GDB { struct itemdata;}
/*
*	对象所处位置的逻辑判断结构（临时）
*/

enum
{
	//这里的数值和顺序其他地方已经使用了 
	LAYER_GROUND,
	LAYER_AIR,
	LAYER_WATER,
	LAYER_INVALID,
};

enum IMMUNE_MASK
{
	IMMUNE_PHYSIC = 0,
	IMMUNE_GOLD,
	IMMUNE_WOOD,
	IMMUNE_WATER,
	IMMUNE_FIRE,
	IMMUNE_EARTH,

	IMMUNE_MASK_PHYSIC	= 0x0001,
	IMMUNE_MASK_GOLD	= 0x0002,
	IMMUNE_MASK_WOOD	= 0x0004,
	IMMUNE_MASK_WATER	= 0x0008,
	IMMUNE_MASK_FIRE	= 0x0010,
	IMMUNE_MASK_EARTH	= 0x0020,

};

struct object_layer_ctrl
{
	enum 
	{
		MODE_GROUND,
		MODE_FLY,
		MODE_FALL,
		MODE_SWIM,
	};

	char _layer;		//对象处于什么层面，0 地面，  1 天上  2 水中,改为由服务器端控制
	char move_mode;		//玩家的移动模式 

	char GetLayer() { return _layer;}
	char GetMode() { return move_mode;}
	bool CanSitDown() { return move_mode == MODE_GROUND; }
	bool IsFalling() { return move_mode == MODE_FALL;}
	bool IsOnGround() { return _layer == LAYER_GROUND;}
	bool IsOnAir()
	{
		return _layer == LAYER_AIR || move_mode == MODE_FALL;
	}

	bool IsFlying()
	{
		return move_mode == MODE_FLY;
	}

	bool CheckAttack()
	{
		return move_mode != MODE_FALL;
	}

	//起飞
	void TakeOff()
	{
		move_mode = MODE_FLY;
		_layer = LAYER_AIR;
	}

	//降落
	void Landing()
	{
		move_mode = MODE_FALL;
		_layer = LAYER_AIR;
	}

	void Swiming()
	{
		move_mode = MODE_SWIM;
		_layer = LAYER_WATER;
	}

	//降落完成
	void Ground()
	{
		move_mode = MODE_GROUND;
		_layer = LAYER_GROUND;
	}

	void UpdateMoveMode(int mode)
	{
		move_mode = MODE_GROUND;
		if(mode & C2S::MOVE_MASK_SKY) 
			move_mode = MODE_FLY;
		else if(mode & C2S::MOVE_MASK_WATER) 
			move_mode = MODE_SWIM;

		switch(mode & 0x3F)
		{
			case C2S::MOVE_MODE_FALL:
			case C2S::MOVE_MODE_SLIDE:
			case C2S::MOVE_MODE_FLY_FALL:
				move_mode = MODE_FALL;
			break;
		}
	}


	void UpdateStopMove(int mode)
	{
		//假设这里已经通过了验证
		move_mode = MODE_GROUND;
		if(mode & C2S::MOVE_MASK_SKY) 
			move_mode = MODE_FLY;
		else if(mode & C2S::MOVE_MASK_WATER) 
			move_mode = MODE_SWIM;
	}

	void UpdateLayer(int layer)	//仅用于player调用
	{
		_layer = layer;
	}
};

template <typename WRAPPER> WRAPPER &  operator >>(WRAPPER & ar, object_layer_ctrl & ctrl)
{
	ar.pop_back(&ctrl,sizeof(ctrl)); return ar;
}

template <typename WRAPPER> WRAPPER & operator <<(WRAPPER & ar, object_layer_ctrl & ctrl)
{
	ar.push_back(&ctrl,sizeof(ctrl)); return ar;
}


class act_session;
/**
 *	能够运动的对象的基础类实现
 */
class gactive_imp : public gobject_imp 
{
	int	_session_id;
protected:
	int _switch_dest;
	A3DVECTOR _switch_pos;
	int _server_notify_timestamp;		//向其他服务器发送存在的计时器

	bool _team_visible_state_flag;		//保存组队状态是否刷新的标志
	bool _visiable_state_flag;		//保存了可见状态的改变
	unsigned char _subscibe_timer;
	abase::vector<unsigned short, abase::fast_alloc<> > _visible_team_state; //保存组队时可见的状态
	abase::vector<int, abase::fast_alloc<> > _visible_team_state_param; 	//保存组队时可见的状态参数
	abase::vector<int, abase::fast_alloc<> > _visible_state_list; 		//保存可见状态的引用计数列表
	abase::vector<link_sid, abase::fast_alloc<> > _subscibe_list;		 //订阅列表
	abase::vector<link_sid, abase::fast_alloc<> > _second_subscibe_list; //第二订阅列表
	abase::static_multimap<int,int, abase::fast_alloc<> >  _set_addon_map;
	abase::vector<int, abase::fast_alloc<> > _idle_seal_mode_counter;	//保存idle_mode和seal_mode的引用计数lgc

	int  _cur_form;		//当前变身状态
	int _idle_mode_flag;		//睡眠模式标志
	int _seal_mode_flag;		//封印模式标志
	A3DVECTOR _direction;		//面朝的方向

public:
	GNET::SkillWrapper _skill;	//技能结构
	unsigned int _faction;		//自己的阵营
	unsigned int _enemy_faction;	//敌人的阵营
	filter_man _filters;		//filter管理器
	basic_prop _basic;		//基本属性值
	extend_prop _base_prop;		//基础属性值，刨除装备影响的基础值 同时这些数据也是保存在数据库中
	extend_prop _cur_prop;		//当前属性值
	item_prop  _cur_item;		//物品所产生的属性(本体)
	property_rune _cur_rune;	//当前的两种攻击优化符
	int _damage_reduce;		//伤害减免	 或者增加（负数为增加伤害，表示百分比中的整数）
	int _magic_damage_reduce[MAGIC_CLASS];//法术伤害减免	 或者增加（负数为增加伤害，表示百分比中的整数）
	int _crit_rate;			//重击概率	 表示重击发生的百分比
	int _base_crit_rate;		//敏捷重击率加成
	int _crit_damage_bonus;	//额外的暴击伤害加成
	int _crit_damage_reduce;//暴击伤害减免
	int _crit_resistance;	//爆击抗性
	int _exp_addon;			//经验加成，仅对player有效
	int _immune_state;		//天生免疫的效果
	int _immune_state_adj;		//后来附加的免疫效果
	abase::vector<int, abase::fast_alloc<> > _immune_state_adj_counter;//保存后来附加的免疫效果的  引用计数
	enhanced_param _en_point;	//按点数增强的属性
	scale_enhanced_param _en_percent;//按百分比增强的属性
	object_layer_ctrl _layer_ctrl;	//位置控制结构
	struct elf_enhance _elf_en;	//小精灵技能或其他对小精灵的属性增强//lgc

	bool _combat_state; 		//是否对象处于战斗状态 
	bool _refresh_state;		//刷新状态，当对象的属性有变化时，这个值应该true
	char _invader_state;		//红粉名状态
	bool _lock_equipment;		//装备锁定标志
	bool _lock_inventory;		//包裹锁定标志
	char _bind_to_ground;		//无法飞行 这个现在是计数了
	char _deny_all_session;		//禁止加入session操作 这个现在是个计数
	int __at_attack_state;		//临时保存一些攻击的状态  无需保存 
	int __at_defense_state;		//临时保存防御时的一些状态，这个状态与攻击状态重叠  无需保存
	int _session_state;		//当前对象的状态(这个state保存的是session state)
	act_session * _cur_session;	//当前的session
	int _hp_gen_counter;		//记录加血的计数值，用于少量加血
	int _mp_gen_counter;		//记录加血的计数值，用于少量加血
	abase::vector<act_session *, abase::fast_alloc<> > _session_list;	//考虑以后用队列实现
	int _expire_item_date;          //是否存在有期限的物品，这里面保存最近一次到期的到期时间
	XID _last_attack_target;        //最后一次攻击的目标，供反沉迷判断使用
	int _attack_degree;		//攻击等级
	int _defend_degree;		//防御等级
	int _invisible_passive;			//被动技能提升的隐身级别,刺客专用
	int _invisible_active;			//主动技能提升的隐身级别		 ;对于怪物来说保存的是模板中定义的隐身级别
	int _anti_invisible_passive;	//被动技能提升的反隐级别,刺客专用
	int _anti_invisible_active;		//技能物品提升的反隐级别		 ;对于怪物来说保存的是模板中定义的反隐级别
	int _damage_dodge_rate;			//伤害闪避概率
	int _debuff_dodge_rate;			//状态闪避概率
	int _hp_steal_rate;				//吸血百分比，以实际造成伤害为准
	int _heal_cool_time_adj;		//使用红符的冷却时间额外增加减少值单位毫秒
	int _skill_enhance;				//增强使用技能的伤害，只有目标是npc才生效
	int _penetration;				//增强伤害，只有目标是npc才生效
	int _resilience;				//减少伤害，只有攻击者为npc才生效
	bool _attack_monster;			//标志玩家或者宠物是否在打怪
	int _vigour_base;				// 气魄由境界等级带来的基础值
	int _vigour_en;					// 气魄在技能和装备中获得的增强值
	int _skill_enhance2;			// 技能伤害加深对所有物体
	float _near_normal_dmg_reduce;	// 近距离普攻伤害减少
	float _near_skill_dmg_reduce;	// 近距离技能伤害减少
	float _far_normal_dmg_reduce;	// 远距离普攻伤害减少
	float _far_skill_dmg_reduce;	// 远距离技能伤害减少
	float _mount_speed_en;			// 骑乘速度增加值

	moving_action_env _moving_action_env;	//移动中可执行的action,仅Player会使用

	enum 
	{
		MAX_HP_DEC_DELAY = 4
	};
	struct damage_delay_t
	{
		XID who;
		int damage;
		bool orange_name;
	};
	
	enum
	{
		INVADER_LVL_0,		//白名
		INVADER_LVL_1,		//粉名
		INVADER_LVL_2,		//红名
	};

	enum
	{
		AT_STATE_ATTACK_RUNE1  	= 0x0001,		//physic attack rune
		AT_STATE_ATTACK_RUNE2  	= 0x0002,		//magic attack rune
		AT_STATE_DEFENSE_RUNE1 	= 0x0004,		//physic defense rune
		AT_STATE_DEFENSE_RUNE2 	= 0x0008,		//magic defense rune
		AT_STATE_ATTACK_CRIT  	= 0x0010,		//crit
		AT_STATE_ATTACK_RETORT	= 0x0020,		//返震攻击
		AT_STATE_EVADE			= 0x0040,		//无效攻击
		AT_STATE_IMMUNE			= 0x0080,		//免疫此次攻击
		AT_STATE_ENCHANT_FAILED	= 0x0100,		//实际只在技能捉宠时使用了
		AT_STATE_ENCHANT_SUCCESS= 0x0200,		//实际只在技能捉宠时使用了
		AT_STATE_DODGE_DAMAGE	= 0x0400,		//伤害躲闪
		AT_STATE_DODGE_DEBUFF	= 0x0800,		//状态躲闪
		AT_STATE_ATTACK_AURA	= 0x1000,		//光环攻击
		AT_STATE_REBOUND        = 0x2000,		//反弹
		AT_STATE_BEAT_BACK      = 0x4000,		//反击
		AT_STATE_CRIT_FEEDBACK  = 0x8000,       //暴击反馈
	};

	enum
	{
		STATE_SESSION_IDLE,
		STATE_SESSION_MOVE,
		STATE_SESSION_ATTACK,
		STATE_SESSION_USE_SKILL,
		STATE_SESSION_GATHERING,
		STATE_SESSION_TRAHSBOX,
		STATE_SESSION_EMOTE,
		STATE_SESSION_OPERATE,
		STATE_SESSION_TRADE,
		STATE_SESSION_ZOMBIE,
		STATE_SESSION_SKILL,
		STATE_SESSION_COSMETIC,
		STATE_SESSION_GENERAL,
	};

	enum 
	{
		SEAL_MODE_NULL		= 0x00,
		SEAL_MODE_ROOT		= 0x01,
		SEAL_MODE_SILENT 	= 0x02,

		IDLE_MODE_NULL		= 0x00,
		IDLE_MODE_SLEEP		= 0x01,
		IDLE_MODE_STUN		= 0x02,
	};
	//lgc
	enum	//seal/idle mode 在_idle_seal_mode_counter中计数的索引
	{
		MODE_INDEX_SLEEP	= 0,
		MODE_INDEX_STUN,	
		MODE_INDEX_ROOT,
		MODE_INDEX_SILENT,
	};

	enum	//notify_root,dispel_root协议中的定身类型
	{
		ROOT_TYPE_SLEEP = 0,
		ROOT_TYPE_STUN,
		ROOT_TYPE_ROOT,
	};

	inline void UpdateDataToParent()		//将自己的基本数据更新的_parent中
	{
		gactive_object * pObj  = (gactive_object*)_parent;
		pObj->base_info.faction = GetFaction();	//阵营可能变化
		pObj->base_info.level = _basic.level;
		pObj->base_info.hp = _basic.hp;
		pObj->base_info.max_hp = _cur_prop.max_hp;
		pObj->base_info.mp = _basic.mp;
	}

	inline void SetAttackMonster(bool flag) {_attack_monster = flag;}
	inline bool IsAttackMonster() {return _attack_monster;}
	inline void ActiveCombatState(bool state) {_combat_state = state; if(!_combat_state) _attack_monster = false;} 
	inline bool IsCombatState() {return _combat_state;} 
	inline bool GetRefreshState() {return _refresh_state;}
	inline void SetRefreshState() {_refresh_state = true;} 
	inline void ClearRefreshState() {_refresh_state = false;} 
	inline void RecalcDirection() {_parent->dir = a3dvector_to_dir(_direction);} 
	inline void SetDirection(unsigned char dir) {_parent->dir = dir;}
	inline void CheckVisibleDataForTeam() {if(_team_visible_state_flag) SendTeamDataToMembers();}
	inline int ActivateSetAddon(int addon_id)
	{
		addon_id &= 0xFFFF;
		return ++_set_addon_map[addon_id];
	}

	inline int DeactivateSetAddon(int addon_id)
	{
		addon_id &= 0xFFFF;
		return --_set_addon_map[addon_id];
	}
	inline int  OI_GetInvaderState() { return _invader_state;}

	inline void UpdateExpireItem(int date)
	{
		if(_expire_item_date <= 0)
		{
			_expire_item_date = date;
		}
		else if(_expire_item_date > date)
		{
			_expire_item_date = date;
		}
	}

public:
	typedef bool (*attack_judge)(gactive_imp * __this , const MSG & msg, attack_msg & amsg);
	typedef bool (*enchant_judge)(gactive_imp * __this, const MSG & msg, enchant_msg & emsg);
	typedef void (*attack_fill)(gactive_imp * __this, attack_msg & attack);
	typedef void (*enchant_fill)(gactive_imp * __this, enchant_msg & enchant);
public:

	gactive_imp();
	~gactive_imp();
	virtual void Init(world * pPlane,gobject*parent);
	virtual void ReInit();
public:
	void SaveAllState(archive &ar);
	void SaveSetAddon(archive &ar);
	void LoadAllState(archive &ar);
	void LoadSetAddon(archive &ar);
	bool StartSession();
	bool EndCurSession();
	void TryStopCurSession();	//试图停止当前的session，不一定成功 ,同时会试图开始
	bool AddSession(act_session * ses);
	act_session * GetCurSession() { return _cur_session;}
	act_session * GetNextSession() { if(HasNextSession()) return _session_list[0]; else return NULL;}
	bool HasNextSession() { return _session_list.size();}
	bool HasSession() { return _cur_session || _session_list.size();}
	bool InNonMoveSession();	//是否正在进行非移动session
	void SaveSessionList(archive & ar);
	bool LoadSessionList(archive & ar);
	void ClearSession();
	void ClearMoveSession();
	void ClearAttackSession();
	void ClearSpecSession(int exclusive_mask);
	void ResetSession();	//cur_session不以endsession的方式结束
	void ClearNextSession();
	bool CurSessionValid(int id);
	int GetCurSessionID() { return _session_id;}
	inline int GetNextSessionID() 
	{
		_session_id++;
		_session_id &= 0x7FFFFFFF;
		return _session_id;
	}
	
	inline moving_action * GetAction(){ return _moving_action_env.GetAction(); }
	inline bool StartAction(moving_action * pAction){ return _moving_action_env.StartAction(pAction); }
	inline void TryBreakAction(){ return _moving_action_env.TryBreakAction(); } 
	inline void RestartAction(){ return _moving_action_env.RestartAction(); } 
	inline void ClearAction(){ return _moving_action_env.ClearAction(); } 
	inline void ReleaseAction(){ return _moving_action_env.ReleaseAction(); } 
	inline bool ActionOnAttacked(int action_id)
	{
		if(!_moving_action_env.ActionValid(action_id)) return true;
		return _moving_action_env.ActionOnAttacked(); 
	}


public:
//虚函数
	virtual bool Save(archive & ar);
	virtual bool Load(archive & ar);
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual int DoAttack(const XID & target,char force_attack);
	virtual bool CanAttack(const XID & target) { return true;}
	virtual bool CheckLevitate(){ return false;}
	virtual void PhaseControlInit(){}
	virtual const XID & GetCurTarget(){ ASSERT(false); return _parent->ID;}
	virtual const XID & GetLeaderID(){ ASSERT(false); return _parent->ID;}
	virtual int GetAmmoCount() { return 0;}
	virtual void OnHeal(const XID & healer, int life){}
	virtual int TakeOutItem(int item_id) { return -1; }
	virtual void TakeOutItem(int item_id, size_t count){}//lgc
	virtual void TakeOutItem(const int * id_list, size_t list_count, size_t count){}
	virtual bool CheckItemExist(int item_id, size_t count) {return false;}
	virtual bool CheckItemExist(int inv_index, int item_id, size_t count) {return false;}
	virtual bool CheckItemExist(const int * id_list, size_t list_count, size_t count) {return false;}
	virtual int CheckItemPrice(int inv_index, int item_id) { return 0;}
	virtual void DropSpecItem(bool isProtected, const XID & owner){}
	virtual size_t GetMoneyAmount() { return 0;}
	virtual void DecMoneyAmount(size_t money) {}
	virtual void DropMoneyAmount(size_t money, bool isProtected){}
	virtual bool UseProjectile(int count) { return true;}
	virtual bool OI_IsMember(const XID & member) { return false;}
	virtual bool OI_IsInTeam() { return false;}
	virtual bool OI_IsTeamLeader() { return false;}
	virtual int SpendFlyTime(int tick) {return 1;}
	virtual int GetFlyTime() {return 0;}
	virtual bool Resurrect(float exp_reduce) {return false;}
	virtual void EnterResurrectReadyState(float exp_reduce, float hp_factor, float mp_factor) {}
	virtual void KnockBack(const XID & target, const A3DVECTOR & source, float distance,int time,int stun_time) {}
	virtual void PullOver(const XID & target, const A3DVECTOR & source, float distance, int time){}
	virtual void Teleport(const A3DVECTOR & pos, int time, char mode){}
	virtual void Teleport2(const A3DVECTOR & pos, int time, char mode){}
	virtual void KnockUp(float distance, int time) {}
	virtual bool DrainMana(int mana) { return true; }
	virtual void SendDataToSubscibeList() = 0;	//发送自身数据给玩家
	virtual void SendTeamDataToSubscibeList() = 0;	//发送组队可见信息给选中对象
	virtual void SendTeamDataToMembers(){} ;	//发送组队可见信息给队友
	virtual void SetIdleMode(bool sleep, bool stun) { _idle_mode_flag = (sleep?IDLE_MODE_SLEEP:0) | (stun?IDLE_MODE_STUN:0); } 
	virtual void SetSealMode(bool silent,bool root) { _seal_mode_flag = (silent?SEAL_MODE_SILENT:0) | (root?SEAL_MODE_ROOT:0); }
	virtual void SendAttackMsg(const XID & target, attack_msg & attack);
	virtual void SendDelayAttackMsg(const XID & target, attack_msg & attack, size_t delay_tick);
	virtual int GetCSIndex() { return -1;}
	virtual int  GetCSSid() { return -1;}
	virtual void SendEnchantMsg(int message,const XID & target, enchant_msg & attack);
	virtual void SendDelayEnchantMsg(int message,const XID & target, enchant_msg & attack, size_t delay_tick);
	virtual void SendMsgToTeam(const MSG & msg,float range,bool exclude_self){}
	virtual void AddNPCAggro(const XID & who,int rage){}
    virtual void RemoveNPCAggro(const XID& src, const XID& dest, float ratio) {}
	virtual void BeTaunted(const XID & who,int rage){}
	virtual void AddAggroToEnemy(const XID & who,int rage){}
	virtual void ClearAggroToEnemy(){}
	virtual void SetCombatState() {}
	virtual bool CheckInvaderAttack(const XID & who) {return false;}	//检查自己是否这个目标的橙名
	virtual void FillAttackMsg(const XID & target, attack_msg & attack, int dec_arrow = 0);
	virtual void FillEnchantMsg(const XID & target, enchant_msg & enchant);
	virtual int GetObjectClass() { return -1;}			//取得对象的职业
	virtual bool CheckCoolDown(int idx) { return true;}
	virtual void SetCoolDown(int idx, int msec) {}
	virtual void NotifySkillStillCoolDown(int cd_id){}
	virtual int GetMonsterFaction() { return 0;}
	virtual int GetFactionAskHelp() { return 0;}
	virtual void SetLifeTime(int life) {} //设置寿命，只有NPC有这个相应
	virtual void EnhanceBreathCapacity(int value) {}
	virtual void ImpairBreathCapacity(int value) {}
	virtual void InjectBreath(int value) {}
	virtual void EnableEndlessBreath(bool bVal) {}
	virtual void AdjustBreathDecPoint(int offset) {}
	virtual void EnableFreePVP(bool bVal) {}
	virtual void ObjReturnToTown() {}
	virtual void AddEffectData(short effect) {}  		//只给 player 用
	virtual void RemoveEffectData(short effect) {}		//只给 player 用
	virtual void AddMultiObjEffect(const XID& target, char type);
	virtual void RemoveMultiObjEffect(const XID& target, char type);
	virtual void EnterCosmeticMode(unsigned short inv_index,int cos_id) {}		//只给 player 用
	virtual void LeaveCosmeticMode(unsigned short inv_index) {}			//只给 player 用
	virtual void SetPerHitAP(int ap_per_hit){}
	virtual void ModifyPerHitAP(int delta){}
	virtual bool IsPlayerClass() { return 0;}
	virtual bool IsEquipWing() { return false;}
	virtual int GetLinkIndex() { return -1;}
	virtual int GetLinkSID() { return -1;}
	virtual int SummonMonster(int mod_id, int count, const XID& target, int target_distance, int remain_time=0, char die_with_who=0, int path_id=0){ return -1; }
	virtual int SummonNPC(int npc_id, int count, const XID& target, int target_distance, int remain_time=0){ return -1;}
	virtual int SummonMine(int mine_id, int count, const XID& target, int target_distance, int remain_time=0){ return -1;}
	virtual bool UseSoulItem(int type, int level, int power) {return false;}	//只给 player 用
	virtual void IncAntiInvisiblePassive(int val){}//只给 player 用
	virtual void DecAntiInvisiblePassive(int val){}//只给 player 用
	virtual void IncAntiInvisibleActive(int val){}
	virtual void DecAntiInvisibleActive(int val){}
	virtual void IncInvisiblePassive(int val){}//只给 player 用
	virtual void DecInvisiblePassive(int val){}//只给 player 用
	virtual void SetInvisible(int invisible_degree){}//对于npc来说参数无效
	virtual void ClearInvisible(){}
	virtual int GetSoulPower(){ return 0; }
	virtual void EnhanceSoulPower(int val){}
	virtual void ImpairSoulPower(int val){}
	virtual void UpdateMinAddonExpireDate(int addon_expire){}
	virtual void SetGMInvisible(){}
	virtual void ClearGMInvisible(){}
	virtual bool ActivateSharpener(int id, int equip_index){ return false; }
	virtual void TransferSpecFilters(int filter_mask, const XID & target, int count);
	virtual void AbsorbSpecFilters(int filter_mask, const XID & target, int count);
	virtual bool SummonPet2(int pet_egg_id, int skill_level, int life_time){ return false; }
	virtual bool SummonPlantPet(int pet_egg_id, int skill_level, int life_time, const XID & target, char force_attack){ return false; }
	virtual bool CalcPetEnhance(int skill_level, extend_prop& prop, int& attack_degree, int& defend_degree,int& vigour){ return false; }
	virtual bool PetSacrifice(){ return false; }
	virtual bool PlantSuicide(float distance, const XID & target, char force_attack){ return false; }
	virtual void InjectPetHPMP(int hp, int mp){}
	virtual void DrainPetHPMP(const XID & pet_id, int hp, int mp){}
	virtual void DrainLeaderHPMP(const XID & attacker, int hp, int mp){}
	virtual void LongJumpToSpouse(){}
	virtual void WeaponDisabled(bool disable){}
	virtual void DetectInvisible(float range){}
	virtual void ForceSelectTarget(const XID & target){}
	virtual void ExchangePosition(const XID & target){}
	virtual void SetSkillAttackTransmit(const XID & target){}
	virtual void CallUpTeamMember(const XID& member){}
	virtual void QueryOtherInventory(const XID& target){}
	virtual void IncPetHp(int inc){}
	virtual void IncPetMp(int inc){}
	virtual void IncPetDamage(int inc){}
	virtual void IncPetMagicDamage(int inc){}
	virtual void IncPetDefense(int inc){}
	virtual void IncPetMagicDefense(int inc){}
	virtual void IncPetAttackDegree(int inc){}
	virtual void IncPetDefendDegree(int inc){}
	virtual void IncAggroOnDamage(const XID & attacker, int val){}
	virtual void DecAggroOnDamage(const XID & attacker, int val){}
	virtual void FestiveAward(int fa_type,int type,const XID& target){}
	virtual void AdjustLocalControlID(int& cid){}
	virtual int  GetMazeRoomIndex() { return 0;}
	virtual void ReduceResurrectExpLost(int value){}
	virtual void IncreaseResurrectExpLost(int value){}
	virtual void SetPlayerLimit(int index, bool b){}
	virtual bool GetPlayerLimit(int index){ return false;}
	virtual void DenyAttackCmd(){}
	virtual void AllowAttackCmd(){}
	virtual void DenyElfSkillCmd(){}
	virtual void AllowElfSkillCmd(){}
	virtual void DenyUseItemCmd(){}
	virtual void AllowUseItemCmd(){}
	virtual void DenyNormalAttackCmd(){}
	virtual void AllowNormalAttackCmd(){}
	virtual void DenyPetCmd(){}
	virtual void AllowPetCmd(){}
	virtual void TurretOutOfControl(){}
	virtual void EnterNonpenaltyPVPState(bool b){}
	virtual int GetHistoricalMaxLevel(){ return _basic.level; }
	virtual int GetAvailLeadership(){ return 0; }	
	virtual void OccupyLeadership(int v){}
	virtual void RestoreLeadership(int v){}
	virtual size_t OI_GetInventorySize() { return 0;}
	virtual size_t OI_GetEmptySlotSize() { return 0;}
	virtual int OI_GetInventoryDetail(GDB::itemdata * list, size_t size) { return -1;}
	virtual size_t OI_GetMallOrdersCount() { return 0;}
	virtual int OI_GetMallOrders(GDB::shoplog * list, size_t size) { return 0;}
	virtual int TradeLockPlayer(int get_mask,int put_mask) { return -1;}
	virtual int TradeUnLockPlayer() { return -1;}
	virtual void OnDuelStart(const XID & target);
	virtual void OnDuelStop();
	virtual void Die(const XID & attacker, bool is_pariah, char attacker_mode, int taskdead);	//taskdead=0非任务死亡=1任务有损死亡=2任务无损死亡
	virtual void ActiveMountState(int mount_id, unsigned short mount_color) {};
	virtual void DeactiveMountState() {};
	virtual bool AddPetToSlot(void * data, int inv_index) { return false;}
	virtual bool FeedPet(int food_mask, int hornor) { return false;}
	virtual bool OI_IsMafiaMember() { return false;}
	virtual int OI_GetMafiaID() { return 0;}
	virtual char OI_GetMafiaRank() { return 0;}
	virtual bool OI_IsMafiaMaster() { return false;}
	virtual bool OI_IsFactionAlliance(int fid) { return false;}
	virtual bool OI_IsFactionHostile(int fid) { return false;}
	virtual int OI_GetSpouseID(){ return 0; }
	virtual int OI_GetReputation(){ return 0; }
	virtual bool CheckGMPrivilege() { return false;}
	virtual int GetFaction() { return _faction;}
	virtual int GetEnemyFaction() { return _enemy_faction;}
	virtual size_t OI_GetTrashBoxCapacity(int where) { return 0;}
	virtual int OI_GetTrashBoxDetail(int where, GDB::itemdata * list, size_t size) { return -1;}
	virtual bool OI_IsTrashBoxModified() {return false;}
	virtual bool OI_IsEquipmentModified() {return false;}
	virtual size_t OI_GetTrashBoxMoney() {return 0;}
	virtual int OI_GetEquipmentDetail(GDB::itemdata * list, size_t size) { return -1;}
	virtual size_t OI_GetEquipmentSize() { return 0;}
	virtual int OI_GetDBTimeStamp() { return 0;}
	virtual int OI_InceaseDBTimeStamp() { return 0;}
	virtual bool CheckWaypoint(int point_index, int & point_domain) { return false;}
	virtual bool ReturnWaypoint(int point) { return false;}
	virtual attack_judge GetPetAttackHook() { return NULL;}
	virtual enchant_judge GetPetEnchantHook() { return NULL;}
	virtual attack_fill GetPetAttackFill() { return NULL;}
	virtual enchant_fill GetPetEnchantFill() { return NULL;}
	virtual bool OI_IsPVPEnable() { return false;}
	virtual char OI_GetForceAttack() { return 0;}
	virtual bool OI_IsInPVPCombatState() { return  false;}
	virtual bool OI_IsInventoryFull() { return true;}
	virtual bool OI_IsPet() { return false;}
	virtual int OI_GetPetEggID() { return 0;}
	virtual XID OI_GetPetID() { return XID(-1,-1);}
	virtual void OI_ResurrectPet() {} 
	virtual void OI_RecallPet() {}
	virtual void OI_TransferPetEgg(const XID & who, int pet_egg){}
	virtual void OI_Disappear() {}
	virtual void OI_Die();
	virtual void Notify_StartAttack(const XID & target,char force_attack) {}
	virtual bool OI_GetMallInfo(int & cash, int & cash_used, int &cash_delta,  int &order_id) { return false;}
	virtual bool OI_IsCashModified() { return false;}
	virtual void ActivePetNoFeed(bool feed) {}
	virtual bool OI_TestSafeLock() { return false;}
	virtual size_t OI_GetPetsCount() { return 0; }
	virtual size_t OI_GetPetSlotCapacity() { return 0; }
	virtual pet_data * OI_GetPetData(size_t index){ return NULL; }
	virtual void OI_TryCancelPlayerBind(){}
	virtual int OI_GetTaskMask(){ return 0; }
	virtual int OI_GetForceID(){ return 0; }
	virtual void UpdateMallConsumptionShopping(int id, unsigned int proc_type, int count, int total_price){};
	virtual void UpdateMallConsumptionBinding(int id, unsigned int proc_type, int count){}
	virtual void UpdateMallConsumptionDestroying(int id, unsigned int proc_type, int count){}
	virtual bool CalcPetEnhance2(const pet_data *pData, extend_prop& prop, int& attack_degree, int& defend_degree, int& vigour){ return false; }
	virtual	void GetNatureSkill(int nature,int &skill1,int &skill2) {}
	virtual int OI_GetRealm() { return 0; }
	virtual int GetLocalVal(int index) { return 0;}
	virtual void SetLocalVal(int index,int val) {}	
	virtual void DeliverTaskToTarget(const XID& target, int taskid) {}
	virtual int ChangeVisibleTypeId(int tid) { return -1;}
    virtual void SetHasPVPLimitFilter(bool has_pvp_limit_filter) {}
	virtual void SetTargetCache(const XID& target) {}
	virtual void DispatchTaskToDmgList(int taskid, int count, int dis) {}
	virtual void EnhanceMountSpeedEn(float sp) {}
	virtual void ImpairMountSpeedEn(float sp) {}
	virtual float GetMountSpeedEnhance() const { return _mount_speed_en;}

public:
	inline void TranslateAttack(const XID & target , attack_msg & attack)
	{
		_filters.EF_TransSendAttack(target,attack);
	}

	inline void TranslateEnchant(const XID & target , enchant_msg & enchant)
	{
		_filters.EF_TransSendEnchant(target,enchant);
	}
	
//inlnie 逻辑操作
	inline void TakeOff()	//起飞
	{
		_layer_ctrl.TakeOff();
		((gactive_object*)_parent)->object_state |= gactive_object::STATE_FLY;
		_runner->takeoff();
	}

	inline void Landing()
	{
		_layer_ctrl.Landing();
		((gactive_object*)_parent)->object_state &= ~gactive_object::STATE_FLY;
		_runner->landing();
	}

	inline void UpdateStopMove(int move_mode) { _layer_ctrl.UpdateStopMove(move_mode); } 
	inline void UpdateMoveMode(int move_mode) { _layer_ctrl.UpdateMoveMode(move_mode); }
	inline void DecSkillPoint(int sp)
	{
		ASSERT(sp > 0 && sp <= _basic.skill_point);
		_basic.skill_point -= sp;
		_runner->cost_skill_point(sp);
		GLog::log(GLOG_INFO,"用户%d消耗了sp %d",_parent->ID.id,sp);
	}
	inline void Heal(const XID & healer, int life)
	{
		if(_parent->IsZombie()) return;
		int newhp = _basic.hp + life;
		if(newhp >= _cur_prop.max_hp)
		{
			newhp = _cur_prop.max_hp;
		}
		life = newhp - _basic.hp;
		_basic.hp = newhp;
		SetRefreshState();
		OnHeal(healer,life);
	}
	inline void Heal(int life)
	{
		if(_parent->IsZombie()) return;
		if(_basic.hp <_cur_prop.max_hp)
		{
			_basic.hp += life;
			if(_basic.hp >= _cur_prop.max_hp)
			{
				_basic.hp = _cur_prop.max_hp;
			}
			SetRefreshState();
		}
	}
	inline void HealBySkill(const XID & healer, int life)
	{
		_filters.EF_AdjustHeal(life,1);
		if(life > 0) Heal(healer,life);	
	}
	inline void HealBySkill(int life)
	{
		_filters.EF_AdjustHeal(life,1);
		if(life > 0) Heal(life);
	}
	inline void HealByPotion(int life)
	{
		_filters.EF_AdjustHeal(life,0);
		if(life > 0) Heal(life);
	}
	inline void InjectMana(int mana)
	{
		if(_basic.mp < _cur_prop.max_mp)
		{
			_basic.mp += mana;	
			if(_basic.mp > _cur_prop.max_mp)
			{
				_basic.mp = _cur_prop.max_mp;
			}
			SetRefreshState();
		}
	}

	//检查是否能移动
	bool CheckMove(int usetime,int move_mode)
	{
		if(usetime  < 80 || usetime > 1000) return -1;
		if((move_mode & C2S::MOVE_MASK_SKY) && !_layer_ctrl.IsFlying())
		{
			return false;
		}

		//这里是不是太严格?
		if(!(move_mode & C2S::MOVE_MASK_SKY) && _layer_ctrl.IsFlying())
		{
			return false;
		}
		return true;
	}

	/*float GetSpeedByMode(int mode)
	{
		float speed[]={_cur_prop.run_speed,_cur_prop.flight_speed,_cur_prop.swim_speed,_cur_prop.run_speed};
		int index = (mode & (C2S::MOVE_MASK_SKY|C2S::MOVE_MASK_WATER)) >> 6; //0 ground 1, sky,2 water, 3 other
		return speed[index];
	}*/

	bool CheckStopMove(const A3DVECTOR & target,int usetime,int move_mode)
	{
		return true;
	}

	inline int TestInvader(bool & orange_name,char force_attack,const XID & attacker)
	{
		orange_name = false;
		if(_invader_state == INVADER_LVL_1)
		{
			//粉名 无视其他标志
			bool self_is_invader = CheckInvaderAttack(attacker);
			if(!self_is_invader)
			{
				//自己不是攻击者
				if(!force_attack)
				{	
					//非主动攻击而且自己不是攻击者
					return -1;
				}
				else 
				{
					//变粉 需要回馈过去一个消息 且变橙色
					orange_name = true;
				}
			}
			//自己是攻击者，所以什么都不必说了
			//就要挨打
		}
		else
		{
			if(_invader_state == INVADER_LVL_0 )
			{
				//变粉 需要回馈过去一个消息 且变橙色
				orange_name = true;
			}
			else
			{
				//肯定是红名 是合法攻击，不需要做任何事情
				//但是有可能攻击消息不匹配，所以可能返回错误
				if(!force_attack) return 1;
			}
		}
		return 0;
	}
	inline void ObjectSitDown()
	{
		((gactive_object*)_parent)->object_state |= gactive_object::STATE_SITDOWN;
	}

	inline void ObjectStandUp()
	{
		((gactive_object*)_parent)->object_state &= ~gactive_object::STATE_SITDOWN;
	}

	inline void ChangeShape(int shape) 
	{ 
		gactive_object * pObj = (gactive_object *)_parent;
		pObj->shape_form = shape & 0xFF;
		pObj->object_state &= ~gactive_object::STATE_SHAPE;
		if(shape) pObj->object_state |= gactive_object::STATE_SHAPE;

		_cur_form = (shape&0xC0)>>6; 
	}

	inline int GetForm() { return _cur_form; }

	//修改长期的表情策略
	inline void SetEmoteState(unsigned char emote)
	{
		gactive_object * pObj = (gactive_object *)_parent;
		pObj->emote_form = emote;
		pObj->object_state |= gactive_object::STATE_EMOTE;
	}
	inline void ClearEmoteState()
	{
		gactive_object * pObj = (gactive_object *)_parent;
		pObj->emote_form = 0;
		pObj->object_state &= ~gactive_object::STATE_EMOTE;
	}

	inline void LockEquipment(bool is_lock)
	{
		_lock_equipment = is_lock;
	}

	inline void LockInventory(bool is_lock)
	{
		_lock_inventory = is_lock;
	}
	
	
	inline void BindToGound(bool is_bind)
	{
		_bind_to_ground += is_bind?1:-1;
	}

	inline bool IsBindGound()
	{
		return _bind_to_ground;
	}

	inline void ForbidBeSelected(bool b)
	{
		gactive_object * pObj = (gactive_object *)_parent;
		if(b)
		{
			pObj->object_state |= gactive_object::STATE_FORBIDBESELECTED;
		}
		else
		{
			pObj->object_state &= ~gactive_object::STATE_FORBIDBESELECTED;
		}
		_runner->forbid_be_selected(b);
	}

public:
//装备影响的函数系列
	template <typename BASE_DATA , typename SCALE_DATA>
		inline void WeaponItemEnhance(short weapon_type,short weapon_delay,int weapon_class,int weapon_level,int attack_speed,float attack_range, float attack_short_range,
				const BASE_DATA & base, const SCALE_DATA & scale)
		{
			_cur_item.weapon_type 		= weapon_type;
			_cur_item.weapon_delay		= weapon_delay;
			_cur_item.weapon_class		= weapon_class;
			_cur_item.weapon_level 		= weapon_level;
			_cur_item.attack_speed		= attack_speed;
			_cur_item.attack_range		= attack_range;
			_cur_item.short_range		= attack_short_range;
			_cur_item.damage_low		= base.damage_low;
			_cur_item.damage_high		= base.damage_high;
			_cur_item.damage_magic_low	= base.magic_damage_low;
			_cur_item.damage_magic_high	= base.magic_damage_high;

			_en_point.defense 		+= base.defense;
			_en_point.armor			+= base.armor;

			_en_percent.defense 		+= scale.defense;
			_en_percent.armor		+= scale.armor;
			_skill.EventWield(this,weapon_class);
		}

	template <typename BASE_DATA , typename SCALE_DATA>
		inline void WeaponItemImpair(const BASE_DATA & base, const SCALE_DATA & scale)
		{
			_skill.EventUnwield(this,_cur_item.weapon_class);
			_cur_item.weapon_type 		= 0;
			_cur_item.weapon_delay		= UNARMED_ATTACK_DELAY;
			_cur_item.weapon_class		= 0;
			_cur_item.weapon_level		= 0;
			_cur_item.damage_low		= 0;
			_cur_item.damage_high		= 0;
			_cur_item.damage_magic_low	= 0;
			_cur_item.damage_magic_high	= 0;
			_cur_item.attack_speed		= 0;
			_cur_item.attack_range		= 0;
			_cur_item.short_range		= 0;

			_en_point.defense 		-= base.defense;
			_en_point.armor			-= base.armor;

			_en_percent.defense 		-= scale.defense;
			_en_percent.armor		-= scale.armor;
		}

	template <typename BASE_DATA , typename SCALE_DATA>
		inline void ArmorEnhance(const BASE_DATA & base, const SCALE_DATA & scale,int hp, int mp)
		{
			_en_point.defense		+= base.defense;
			_en_point.armor			+= base.armor;

			_en_point.damage_low		+= base.damage_low;
			_en_point.damage_high		+= base.damage_high;
			_en_point.magic_dmg_low		+= base.magic_damage_low;
			_en_point.magic_dmg_high	+= base.magic_damage_high;

			_en_percent.damage		+= scale.damage;
			_en_percent.magic_dmg		+= scale.magic_damage;

			_en_point.max_hp += hp;
			_en_point.max_mp += mp;
		}

	template <typename BASE_DATA , typename SCALE_DATA>
		inline void ArmorImpair(const BASE_DATA & base, const SCALE_DATA & scale,int hp, int mp)
		{
			_en_point.defense 		-= base.defense;
			_en_point.armor			-= base.armor;

			_en_point.damage_low		-= base.damage_low;
			_en_point.damage_high		-= base.damage_high;
			_en_point.magic_dmg_low		-= base.magic_damage_low;
			_en_point.magic_dmg_high	-= base.magic_damage_high;

			_en_percent.damage		-= scale.damage;
			_en_percent.magic_dmg		-= scale.magic_damage;

			_en_point.max_hp -= hp;
			_en_point.max_mp -= mp;
		}

	template <typename BASE_DATA , typename SCALE_DATA>
		inline void NormalEnhance(const BASE_DATA & base, const SCALE_DATA & scale)
		{
			_en_point.damage_low		+= base.damage_low;
			_en_point.damage_high		+= base.damage_high;
			_en_point.defense 		+= base.defense;
			_en_point.armor			+= base.armor;
			_en_point.magic_dmg_low		+= base.magic_damage_low;
			_en_point.magic_dmg_high	+= base.magic_damage_high;

			_en_percent.defense 		+= scale.defense;
			_en_percent.armor		+= scale.armor;
			_en_percent.damage		+= scale.damage;
			_en_percent.magic_dmg		+= scale.magic_damage;
		}

	template <typename BASE_DATA , typename SCALE_DATA>
		inline void NormalImpair(const BASE_DATA & base, const SCALE_DATA & scale)
		{
			_en_point.damage_low		-= base.damage_low;
			_en_point.damage_high		-= base.damage_high;
			_en_point.defense 		-= base.defense;
			_en_point.armor			-= base.armor;
			_en_point.magic_dmg_low		-= base.magic_damage_low;
			_en_point.magic_dmg_high	-= base.magic_damage_high;

			_en_percent.defense 		-= scale.defense;
			_en_percent.armor		-= scale.armor;
			_en_percent.damage		-= scale.damage;
			_en_percent.magic_dmg		-= scale.magic_damage;
		}
		
	inline void EnhanceResistance(int res[MAGIC_CLASS])
	{
		_en_point.resistance[0] += res[0];
		_en_point.resistance[1] += res[1];
		_en_point.resistance[2] += res[2];
		_en_point.resistance[3] += res[3];
		_en_point.resistance[4] += res[4];
	}

	inline void ImpairResistance(int res[MAGIC_CLASS])
	{
		_en_point.resistance[0] -= res[0];
		_en_point.resistance[1] -= res[1];
		_en_point.resistance[2] -= res[2];
		_en_point.resistance[3] -= res[3];
		_en_point.resistance[4] -= res[4];
	}

	inline void EnhanceAllResistance(int res)
	{
		_en_point.resistance[0] += res;
		_en_point.resistance[1] += res;
		_en_point.resistance[2] += res;
		_en_point.resistance[3] += res;
		_en_point.resistance[4] += res;
	}

	inline void ImpairAllResistance(int res)
	{
		_en_point.resistance[0] -= res;
		_en_point.resistance[1] -= res;
		_en_point.resistance[2] -= res;
		_en_point.resistance[3] -= res;
		_en_point.resistance[4] -= res;
	}

	inline void EnhanceScaleAllResistance(int res)
	{
		_en_percent.resistance[0] += res;
		_en_percent.resistance[1] += res;
		_en_percent.resistance[2] += res;
		_en_percent.resistance[3] += res;
		_en_percent.resistance[4] += res;
	}

	inline void ImpairScaleAllResistance(int res)
	{
		_en_percent.resistance[0] -= res;
		_en_percent.resistance[1] -= res;
		_en_percent.resistance[2] -= res;
		_en_percent.resistance[3] -= res;
		_en_percent.resistance[4] -= res;
	}
	
	inline void EnhanceAllMagicDamageReduce(int mdr)
	{
		_magic_damage_reduce[0] += mdr;	
		_magic_damage_reduce[1] += mdr;	
		_magic_damage_reduce[2] += mdr;	
		_magic_damage_reduce[3] += mdr;	
		_magic_damage_reduce[4] += mdr;	
	}

	inline void ImpairAllMagicDamageReduce(int mdr)
	{
		_magic_damage_reduce[0] -= mdr;	
		_magic_damage_reduce[1] -= mdr;	
		_magic_damage_reduce[2] -= mdr;	
		_magic_damage_reduce[3] -= mdr;	
		_magic_damage_reduce[4] -= mdr;	
	}

	inline void TitleEnhance(int phyd,int magicd,int phydf,int magicdf,int attack,int armor)
	{
		_en_point.damage_low	+= phyd;
		_en_point.damage_high	+= phyd;
		_en_point.magic_dmg_low	+= magicd;
		_en_point.magic_dmg_high+= magicd;
		_en_point.attack 		+= attack;
		_en_point.armor			+= armor;
		_en_point.defense		+= phydf;
		EnhanceAllResistance(magicdf);
	}

	inline void TitleImpair(int phyd,int magicd,int phydf,int magicdf,int attack,int armor)
	{
		_en_point.damage_low	-= phyd;
		_en_point.damage_high	-= phyd;
		_en_point.magic_dmg_low	-= magicd;
		_en_point.magic_dmg_high-= magicd;
		_en_point.attack 		-= attack;
		_en_point.armor			-= armor;
		_en_point.defense		-= phydf;
		ImpairAllResistance(magicdf);
	}

	inline void EnhanceVigour(int vigour)
	{
		_vigour_en += vigour;
	}
	
	inline void ImpairVigour(int vigour)
	{
		_vigour_en -= vigour;
	}

	inline void GeneralCardEnhance(int max_hp, int damage_low, int damage_high, int damage_magic_low, int damage_magic_high, int defense, int res[5], int vigour)
	{
		_en_point.max_hp 			+= max_hp;
		_en_point.damage_low        += damage_low;
		_en_point.damage_high       += damage_high;
		_en_point.magic_dmg_low     += damage_magic_low;
		_en_point.magic_dmg_high    += damage_magic_high;
		_en_point.defense       	+= defense;
		_en_point.resistance[0] 	+= res[0];
		_en_point.resistance[1] 	+= res[1];
		_en_point.resistance[2] 	+= res[2];
		_en_point.resistance[3] 	+= res[3];
		_en_point.resistance[4] 	+= res[4];
		_vigour_en 					+= vigour;
	}

	inline void GeneralCardImpair(int max_hp, int damage_low, int damage_high, int damage_magic_low, int damage_magic_high, int defense, int res[5], int vigour)
	{
		_en_point.max_hp 			-= max_hp;
		_en_point.damage_low        -= damage_low;
		_en_point.damage_high       -= damage_high;
		_en_point.magic_dmg_low     -= damage_magic_low;
		_en_point.magic_dmg_high    -= damage_magic_high;
		_en_point.defense       	-= defense;
		_en_point.resistance[0] 	-= res[0];
		_en_point.resistance[1] 	-= res[1];
		_en_point.resistance[2] 	-= res[2];
		_en_point.resistance[3] 	-= res[3];
		_en_point.resistance[4] 	-= res[4];
		_vigour_en 					-= vigour;
	}

public:
//攻击函数系列
	inline int GetMagicRuneDamage()
	{
		if(_cur_rune.rune_level_min)
		{
			//这里来进行相应操作
			if(_cur_rune.rune_type)
			{
				if(_cur_rune.rune_level_max >= _cur_item.weapon_level 
						&& _cur_rune.rune_level_min <= _cur_item.weapon_level)
				{
					__at_attack_state |= AT_STATE_ATTACK_RUNE2;
					int enh = _cur_rune.rune_enhance;
					//减少优化符
					OnUseAttackRune();
					return enh;
				}
			}
		}
		return 0;
	}
	
	inline int GetPhysicRuneDamage()
	{
		if(_cur_rune.rune_level_min)
		{
			//这里来进行相应操作
			if(!_cur_rune.rune_type)
			{
				if(_cur_rune.rune_level_max >= _cur_item.weapon_level 
						&& _cur_rune.rune_level_min <= _cur_item.weapon_level)
				{
					__at_attack_state |= AT_STATE_ATTACK_RUNE1;
					int enh = _cur_rune.rune_enhance;
					OnUseAttackRune();
					return enh;
				}
			}
		}
		return 0;
	}
	
	inline int GenerateEquipPhysicDamage()
	{
		//取得基础攻击力
		int low = _base_prop.damage_low  + _en_point.damage_low  + _cur_item.damage_low;
		int high= _base_prop.damage_high + _en_point.damage_high + _cur_item.damage_high;
		low = abase::Rand(low,high);
		return low;
	}
	
	inline int GenerateEquipMagicDamage()
	{
		//取得基础攻击力
		int low = _base_prop.damage_magic_low  + _en_point.magic_dmg_low  + _cur_item.damage_magic_low;
		int high= _base_prop.damage_magic_high + _en_point.magic_dmg_high + _cur_item.damage_magic_high;
		low = abase::Rand(low,high);
		return low;
	}

	inline int GeneratePhysicDamage(int scale_damage, int point_damage)
	{
		//取得基础攻击力
		int low = _base_prop.damage_low  + _en_point.damage_low  + _cur_item.damage_low;
		int high= _base_prop.damage_high + _en_point.damage_high + _cur_item.damage_high;
		low = abase::Rand(low,high);

		//计算优化符效果
		low += GetPhysicRuneDamage();
		
		//计算比例加成
		float scale = 0.01f*(float)(100 + _en_percent.damage + _en_percent.base_damage + scale_damage);
		low = (int)(low * scale);

		//返回攻击
		low = low + point_damage;
		if(low < 0) low = 0;
		return low;
	}

	inline int GenerateMaigicDamage2(int scale_damage, int point_damage)
	{
		//取得基础攻击力
		int low = _base_prop.damage_magic_low  + _en_point.magic_dmg_low  + _cur_item.damage_magic_low;
		int high= _base_prop.damage_magic_high + _en_point.magic_dmg_high + _cur_item.damage_magic_high;
		low = abase::Rand(low,high);
		
		//计算优化符效果
		low += GetMagicRuneDamage();

		//计算比例加成
		float scale = 0.01f*(float)(100 + _en_percent.magic_dmg + _en_percent.base_magic + scale_damage);
		low = (int)(low * scale);

		//返回攻击
		low = low + point_damage;
		if(low < 0) low = 0;
		return low;
	}

	inline int GenerateAttackDamage()
	{
		int damage = abase::Rand(_cur_prop.damage_low,_cur_prop.damage_high);
		damage += GetPhysicRuneDamage();
		return damage;
	}

	inline int GenerateMagicDamage()
	{
		int damage = abase::Rand(_cur_prop.damage_magic_low,_cur_prop.damage_magic_high);
		damage += GetMagicRuneDamage();
		return damage;
	}

	inline int GeneratePhysicDamageWithoutRune(int scale_damage, int point_damage)
	{
		//取得基础攻击力
		int low = _base_prop.damage_low  + _en_point.damage_low  + _cur_item.damage_low;
		int high= _base_prop.damage_high + _en_point.damage_high + _cur_item.damage_high;
		low = abase::Rand(low,high);

		//计算比例加成
		float scale = 0.01f*(float)(100 + _en_percent.damage + _en_percent.base_damage + scale_damage);
		low = (int)(low * scale);

		//返回攻击
		low = low + point_damage;
		if(low < 0) low = 0;
		return low;
	}

	inline int GenerateMaigicDamage2WithoutRune(int scale_damage, int point_damage)
	{
		//取得基础攻击力
		int low = _base_prop.damage_magic_low  + _en_point.magic_dmg_low  + _cur_item.damage_magic_low;
		int high= _base_prop.damage_magic_high + _en_point.magic_dmg_high + _cur_item.damage_magic_high;
		low = abase::Rand(low,high);
		
		//计算比例加成
		float scale = 0.01f*(float)(100 + _en_percent.magic_dmg + _en_percent.base_magic + scale_damage);
		low = (int)(low * scale);

		//返回攻击
		low = low + point_damage;
		if(low < 0) low = 0;
		return low;
	}

	inline int GenerateAttackDamageWithoutRune()
	{
		int damage = abase::Rand(_cur_prop.damage_low,_cur_prop.damage_high);
		return damage;
	}

	inline int GenerateMagicDamageWithoutRune()
	{
		int damage = abase::Rand(_cur_prop.damage_magic_low,_cur_prop.damage_magic_high);
		return damage;
	}

	//设置下次攻击的类别为反震攻击，一次有效
	inline void SetRetortState()
	{
		__at_attack_state = AT_STATE_ATTACK_RETORT;
	}

	inline bool GetRetortState()
	{
		return __at_attack_state & AT_STATE_ATTACK_RETORT;
	}

	inline void SetAuraAttackState()
	{
		__at_attack_state = AT_STATE_ATTACK_AURA;	
	}
	
	inline bool GetAuraAttackState()
	{
		return __at_attack_state & AT_STATE_ATTACK_AURA;	
	}

	inline void SetReboundState()
	{
		__at_attack_state = AT_STATE_REBOUND;	
	}

	inline void SetBeatBackState()
	{
		__at_attack_state = AT_STATE_BEAT_BACK;	
	}

	inline void DoDamageReduce(float & damage)
	{
		if(_damage_reduce)
		{
			int dr= _damage_reduce;
			if(dr > 75) dr = 75;
			damage *= (100.f -  dr)*0.01f;
		}
	}
	
	inline void DoMagicDamageReduce(size_t cls, float & damage)
	{
		if(_magic_damage_reduce[cls])
		{
			int dr= _magic_damage_reduce[cls];
			if(dr > 90) dr = 90;
			damage *= (100.f -  dr)*0.01f;
		}
	}
	int MakeAttackMsg(attack_msg & attack,char force_attack);
	bool CheckAttack(const XID & target,bool report_err=true);
	bool CheckAttack(const XID & target,int * flag,float * pDis ,A3DVECTOR & pos);

public:
//消息发送函数
	template<int>
		void SendTo(int message,const XID & target, int param) 
		{
			MSG msg;
			BuildMessage(msg,message,target,_parent->ID,_parent->pos,param);
			_plane->PostLazyMessage(msg);
		}

	template<int>
		void LazySendTo(int message,const XID & target, int param, int latancy) 
		{
			MSG msg;
			BuildMessage(msg,message,target,_parent->ID,_parent->pos,param);
			_plane->PostLazyMessage(msg,latancy);
		}

	template<int>
		void LazySendTo(int message,const XID & target, int param, int latancy,const void * buf, size_t len) 
		{
			MSG msg;
			BuildMessage(msg,message,target,_parent->ID,_parent->pos,param,buf,len);
			_plane->PostLazyMessage(msg,latancy);
		}

	template<int>
		void SendTo(int message,const XID & target, int param,const void * buf, size_t len) 
		{
			MSG msg;
			BuildMessage(msg,message,target,_parent->ID,_parent->pos,param,buf,len);
			_plane->PostLazyMessage(msg);
		}

	void SendTeamVisibleStateToOther(int user_id,int cs_index, int cs_sid);


	inline void SetMaxAP(int max_ap)
	{
		ASSERT(max_ap >=0);
		_cur_prop.max_ap += max_ap - _base_prop.max_ap;  
		_base_prop.max_ap = max_ap;  
		SetRefreshState();
	}
	
	inline void ModifyAP(int ap)
	{
		ap += _basic.ap;
		if(ap < 0)
		{
			ap = 0;
		}
		else if(ap > _cur_prop.max_ap) 
		{
			ap = _cur_prop.max_ap;
		}
		if(_basic.ap != ap)
		{
			_basic.ap = ap;
			SetRefreshState();
		}
	}

    inline void ModifyHP(int hp)
    {
        hp += _basic.hp;
        if (hp < 0)
        {
            hp = 0;
        }
        else if (hp > _cur_prop.max_hp)
        {
            hp = _cur_prop.max_hp;
        }

        if (_basic.hp != hp)
        {
            _basic.hp = hp;
            SetRefreshState();
        }
    }

    inline void ModifyScaleHP(int hp)
    {
        if (hp > 0)
        {
            ModifyHP(_cur_prop.max_hp * hp / 100);
        }
    }

	void ClearSubscibeList();
protected:

	void Swap(gactive_imp * rhs);
	void InsertInfoSubscibe(const XID & target, const link_sid & ld);
	void RemoveInfoSubscibe(const XID & target)
	{
		link_sid * last = _subscibe_list.end();
		link_sid * first = _subscibe_list.begin();
		for(;last != first;)
		{
			--last;
			if(target.id == last->user_id)
			{
				_subscibe_list.erase_noorder(last);
				if(_subscibe_list.empty() && _second_subscibe_list.empty()) 
					_subscibe_timer = 0;
				return;
			}
		}
	}
	void InsertInfoSecondSubscibe(const XID & target, const link_sid & ld);
	void RemoveInfoSecondSubscibe(const XID & target)
	{
		link_sid * last = _second_subscibe_list.end();
		link_sid * first = _second_subscibe_list.begin();
		for(;last != first;)
		{
			--last;
			if(target.id == last->user_id)
			{
				_second_subscibe_list.erase_noorder(last);
				if(_subscibe_list.empty() && _second_subscibe_list.empty()) 
					_subscibe_timer = 0;
				return;
			}
		}
	}
	void RefreshSubscibeList();			//刷新订阅列表 要求在清空组队可见状态和自身标志前调用
	void NotifyTargetChange(XID& target);          //通知订阅列表 自己目标改变

	inline void IncVisibleState(unsigned short state)
	{
		int & counter= _visible_state_list[state];
		ASSERT(counter >= 0);
		if(!counter) _visiable_state_flag= true;
		counter ++;

	}
	
	inline void DecVisibleState(unsigned short state)
	{
		int & counter= _visible_state_list[state];
		//ASSERT(counter > 0);
		if(counter > 0)
		{
			if(counter == 1) _visiable_state_flag= true;
			counter --; 
		}
		else
		{
			ASSERT(false);
			GLog::log(GLOG_ERR,"FATALERROR: DECVEISBLESTATE state %d,counter:%d\n:",state,counter);
		}
	}

	inline void ClearVisibleState(unsigned short state)
	{
		int & counter= _visible_state_list[state];
		ASSERT(counter >= 0);
		if(counter) 
		{
			_visiable_state_flag= true;
			counter = 0;
		}	
	}
	inline unsigned short __TVSGetState(unsigned short s){ return s & 0x3FFF; }
	inline size_t __TVSGetParamCnt(unsigned short s){ return (s & 0xC000) >> 14; }
	inline void InsertTeamVisibleState(unsigned short state, int * param, size_t param_count)
	{
		ASSERT(param_count >= 0 && param_count <= 3);
		size_t count = _visible_team_state.size();
		for(size_t i = count ; i > 0; i--) 
		{
			size_t index = i - 1;
			if(__TVSGetState(_visible_team_state[index]) == state)
			{
				return;
			}
		}
		if(param_count) state |= ((param_count & 0x03) << 14);
		_visible_team_state.push_back(state);
		for(size_t m=0; m<param_count; m++)
			_visible_team_state_param.push_back(param[m]);
			
		_team_visible_state_flag = true;
	}

	inline void RemoveTeamVisibleState(unsigned short state)
	{
		size_t param_index = _visible_team_state_param.size();
		size_t count = _visible_team_state.size();
		for(size_t i = count ; i > 0; i--) 
		{
			size_t index = i - 1;
			size_t param_count = __TVSGetParamCnt(_visible_team_state[index]);
			if(param_count) param_index -= param_count;
			if(__TVSGetState(_visible_team_state[index]) == state)
			{
				_visible_team_state.erase(_visible_team_state.begin() + index);
				if(param_count)	_visible_team_state_param.erase(_visible_team_state_param.begin() + param_index, _visible_team_state_param.begin() + param_index + param_count);
				
				_team_visible_state_flag = true;
			}
		}
	}

	inline void ModifyTeamVisibleState(unsigned short state, int * param, size_t param_count)
	{
		ASSERT(param_count >= 1 && param_count <= 3);
		size_t param_index = _visible_team_state_param.size();
		size_t count = _visible_team_state.size();
		for(size_t i = count ; i > 0; i--) 
		{
			size_t index = i - 1;
			size_t pcount = __TVSGetParamCnt(_visible_team_state[index]);
			if(pcount) param_index -= pcount;
			if(__TVSGetState(_visible_team_state[index]) == state)
			{
				ASSERT(param_count == pcount);
				for(size_t m=0; m<param_count; m++)
					_visible_team_state_param[param_index + m] = param[m];
				
				_team_visible_state_flag = true;
				return;	
			}
		}
	}
	
	void UpdateVisibleState();
	//lgc
	inline void IncIdleSealMode(unsigned char mode)
	{
		ASSERT(mode < 4);
		int & counter= _idle_seal_mode_counter[mode];
		ASSERT(counter >= 0);
		if(!counter)
		{
			if(mode == MODE_INDEX_SLEEP)
			{
				bool sleep, stun;
				GetIdleMode(sleep, stun);
				SetIdleMode(true, stun);
				_runner->notify_root(ROOT_TYPE_SLEEP);
			}
			else if(mode == MODE_INDEX_STUN)
			{
				bool sleep, stun;
				GetIdleMode(sleep, stun);
				SetIdleMode(sleep, true);
				_runner->notify_root(ROOT_TYPE_STUN);
			}
			else if(mode == MODE_INDEX_ROOT)
			{
				bool silent, root;
				GetSealMode(silent, root);
				SetSealMode(silent, true);
				_runner->notify_root(ROOT_TYPE_ROOT); 
			}
			else if(mode == MODE_INDEX_SILENT)
			{
				bool silent, root;
				GetSealMode(silent, root);
				SetSealMode(true, root);
			}
		}
		counter ++;
	}
	
	inline void DecIdleSealMode(unsigned char mode)
	{
		ASSERT(mode < 4);
		int & counter= _idle_seal_mode_counter[mode];
		if(counter > 0)
		{
			if(counter == 1)
			{
			if(mode == MODE_INDEX_SLEEP)
			{
				bool sleep, stun;
				GetIdleMode(sleep, stun);
				SetIdleMode(false, stun);
				_runner->dispel_root(ROOT_TYPE_SLEEP);
			}
			else if(mode == MODE_INDEX_STUN)
			{
				bool sleep, stun;
				GetIdleMode(sleep, stun);
				SetIdleMode(sleep, false);
				_runner->dispel_root(ROOT_TYPE_STUN);
			}
			else if(mode == MODE_INDEX_ROOT)
			{
				bool silent, root;
				GetSealMode(silent, root);
				SetSealMode(silent, false);
				_runner->dispel_root(ROOT_TYPE_ROOT); 
			}
			else if(mode == MODE_INDEX_SILENT)
			{
				bool silent, root;
				GetSealMode(silent, root);
				SetSealMode(false, root);
			}
			}
			counter --; 
		}
		else
		{
			ASSERT(false);
			GLog::log(GLOG_ERR,"FATALERROR: DECIDLESEALMODE mode %d,counter:%d\n:",mode,counter);
		}
	} 

	void IncImmuneMask(int mask)
	{
		int i=0;
		while(mask)
		{
			if(mask & 1)
			{
				if(_immune_state_adj_counter[i] == 0)
					_immune_state_adj |= 1<<i;
				_immune_state_adj_counter[i] ++;
			}
			i++;
			mask >>= 1;
		}
	}

	void DecImmuneMask(int mask)
	{
		int i=0;
		while(mask)
		{
			if(mask & 1)
			{
				if(_immune_state_adj_counter[i] == 1)
					_immune_state_adj &= ~(1<<i);
				_immune_state_adj_counter[i] --;
			}
			i++;
			mask >>= 1;
		}
	}

	friend class object_interface;
	friend class ai_actobject;
public:
	inline void DecHP(int hp)
	{
		if(_parent->IsZombie()) return;
		DoDamage(hp);
		if(_basic.hp <=0)
		{
			_basic.hp = 0;
			Die(XID(-1,-1),false,0,0);
		}
	}
	//因为某种原因受到特定的伤害 这个函数主要由filter来调用
	template<typename ATTACK_INFO>
	void BeHurt(const XID & who,const ATTACK_INFO & info,int damage,bool invader, char attacker_mode)
	{	
		if(_parent->IsZombie()) return;
		if(damage <= 0) damage = 1;
		OnHurt(who,info,damage,invader);
		DoDamage(damage);
		if(_basic.hp <=0)
		{
			_basic.hp = 0;
			Die(info.attacker,invader,attacker_mode,0);
		}
		else if (invader)
		{
			SendTo<0>(GM_MSG_PLAYER_BECOME_INVADER,who,60);
		}
	}


	inline void GetIdleMode(bool & sleep, bool & stun)
	{
		sleep = _idle_mode_flag & IDLE_MODE_SLEEP;
		stun  = _idle_mode_flag & IDLE_MODE_STUN ;
	}

	inline void GetSealMode(bool & silent,bool & root)
	{
		silent = _seal_mode_flag & SEAL_MODE_SILENT;
		root  = _seal_mode_flag & SEAL_MODE_ROOT;
	}
	
	inline int GetSealMode()
	{ 
		return _seal_mode_flag;
	}

	inline bool IsRootMode()
	{
		return _seal_mode_flag & SEAL_MODE_ROOT;
	}

	inline int GetVigour()
	{
		return _vigour_base + _vigour_en;
	}

	inline void SetVigourBase(int vb)
	{
		_vigour_base = vb;
	}

protected:
//受到攻击和技能的判定
	int HandleAttackMsg(world * pPlane,const MSG & msg, attack_msg * );
	int HandleEnchantMsg(world * pPlane,const MSG & msg, enchant_msg * );
	bool AttackJudgement(attack_msg * attack,damage_entry &dmg,bool is_short_range,float dist );
	inline int DoDamage(int damage)
	{
		if(damage > _basic.hp) damage = _basic.hp;
		if(damage <= 0) damage = 1;
		_basic.hp -= damage;
		SetRefreshState();
		return damage;
	}
	int InsertDamageEntry(int damage,int delay,const XID & target, bool orange_name, char attacker_mode);

protected:
//杂项
	void MH_query_info00(const MSG & msg);
	void DoHeartbeat(size_t tick);
	inline void IncHP(int hp_gen)
	{
		int tmp = _basic.hp + hp_gen;
		if(tmp > _cur_prop.max_hp) tmp = _cur_prop.max_hp;
		if(tmp != _basic.hp)
		{
			_basic.hp = tmp;
			SetRefreshState();
		}
	}

	struct func
	{
		//这个是每八秒增加传入的数值
		static inline int  Update(int & base, int & counter,int offset,int max)
		{    
			counter += offset;
			int xo = counter & 0x07;
			base += counter >> 3;
			counter = xo;
			if(base > max) 
				base = max;
			else if(base < 0)
				base = 0;
			return base;
		}
	}; 

	inline int GenHP(int hp_gen)
	{
		int tmp = _basic.hp;
		if( tmp != func::Update(_basic.hp,_hp_gen_counter,hp_gen,_cur_prop.max_hp))
		{
			SetRefreshState();
		}
		return _basic.hp;
	}
	
	inline void GenHPandMP(int hp_gen,int mp_gen)
	{
		int tmp = _basic.hp;
		if( tmp != func::Update(_basic.hp,_hp_gen_counter,hp_gen,_cur_prop.max_hp))
		{
			SetRefreshState();
		}

		tmp = _basic.mp;
		if( tmp != func::Update(_basic.mp,_mp_gen_counter,mp_gen,_cur_prop.max_mp))
		{
			SetRefreshState();
		}
	}

	template <int foo>
	bool CheckServerNotify()
	{
		int tick = g_timer.get_tick();
		if(tick - _server_notify_timestamp > 0)
		{
			_server_notify_timestamp = tick + 25*20;
			return true;
		}
		return false;
	}

private:
//私有虚函数组
	virtual void OnHeartbeat(size_t tick) = 0;
	virtual void OnDamage(const XID & attacker,int skill_id, const attacker_info_t&info,int damage,int at_state,char speed,bool orange,unsigned char section)=0;
	virtual void OnHurt(const XID & attacker,const attacker_info_t&info,int damage,bool invader)=0;
	virtual void OnDeath(const XID & lastattack,bool is_invader, char attacker_mode, int taskdead) {};
	virtual void OnAttacked(world *pPlane,const MSG & msg, attack_msg * attack,damage_entry & dmg,bool hit) = 0;
	virtual void AdjustDamage(const MSG & msg, attack_msg * attack,damage_entry & dmg,float & damage_adjust) = 0;
	virtual bool CheckInvader(world * pPlane, const XID & source){ return false;} //检测粉名攻击是否有效
	virtual void OnPickupMoney(size_t money,int drop_id){}
	virtual void OnPickupItem(const A3DVECTOR &pos,const void * data, size_t size,bool isTeam, int drop_id) {}
	virtual void OnUseAttackRune() {}

public:
//技能的所有接口
	inline int  CheckSkillCondition(unsigned int skill,const XID * target, int size)
	{
		object_interface obj_if(this);
		return _skill.Condition(skill,obj_if,target,size);
	}
	
	inline int NPCStartSkill(unsigned int skill_id, int level, const XID & target,int & next_interval)
	{
		return _skill.NpcStart(skill_id,this,level,&target,1,next_interval);
	}
	
	inline void NPCEndSkill(unsigned int skill_id,int level,const XID & target)
	{
		_skill.NpcEnd(skill_id,this,level,&target,1);
	}

	inline bool NPCSkillOnAttacked(unsigned int skill_id,int level)
	{
		return _skill.NpcInterrupt(skill_id, object_interface(this),level);
	}

	inline float NPCSkillRange(unsigned int skill_id,int level)
	{
		return _skill.NpcCastRange(skill_id,level);
	}
	

	inline int StartSkill(SKILL::Data & data, const XID * target,int size,int & next_interval)
	{
		return _skill.StartSkill(data,object_interface(this),target,size,next_interval);
	}

	inline int RunSkill(SKILL::Data & data, const XID * target,int size,int & next_interval)
	{
		return _skill.Run(data, object_interface(this),target,size,next_interval);
	}

	inline int StartSkill(SKILL::Data & data, const A3DVECTOR &pos, const XID * target,int size,int & next_interval)
	{
		return _skill.StartSkill( data, object_interface(this), pos, target,size,next_interval);
	}

	inline int RunSkill(SKILL::Data & data, const A3DVECTOR & pos, const XID * target,int size,int & next_interval)
	{
		return _skill.Run( data, object_interface(this), pos, target,size, next_interval);
	}
	
	inline int ContinueSkill(SKILL::Data & data,const XID * target,int size,int & next_interval,int elapse_time) 
	{
		return _skill.Continue(data, object_interface(this),target,size,next_interval,elapse_time);
	}

	inline int CastInstantSkill(SKILL::Data & data, const XID * target,int size)
	{
		return _skill.InstantSkill(data,object_interface(this),target,size);
	}

	inline bool SkillOnAttacked(SKILL::Data & data)
	{
		return _skill.Interrupt(data, object_interface(this));
	}

	inline bool CancelSkill(SKILL::Data & data)
	{
		return _skill.Cancel(data, object_interface(this));
	}

	inline int GetSkillLevel(int skill_id)
	{
		return _skill.GetSkillLevel(skill_id);
	}

	inline void IncSkillAbility(int skill_id, int inc = 1)
	{
		_skill.IncAbility(object_interface(this),skill_id,inc);
	}

	inline int GetSkillAbility(int skill_id)
	{
		return _skill.GetAbility(skill_id);
	}

	inline float GetSkillAbilityRatio(int skill_id)
	{
		return _skill.GetAbilityRatio(skill_id);
	}
	
	inline void IncSkillAbilityRatio(int id, float ratio)
	{
		_skill.IncAbilityRatio(object_interface(this),id,ratio);	
	}

	inline int RemoveSkill(int skill_id)
	{
		return _skill.Remove(skill_id);
	}

	inline bool CastRune(int skill_id, int skill_level)
	{
		SKILL::Data sd((unsigned int)skill_id);
		return _skill.CastRune(sd, object_interface(this),skill_level) == 0;
	}
	
	inline int StartRuneSkill(SKILL::Data & data, const XID * target,int size,int & next_interval,int level)
	{
		return _skill.StartRuneSkill(data,object_interface(this),target,size,next_interval,level);
	}

	inline int RunRuneSkill(SKILL::Data & data, const XID * target,int size,int & next_interval, int level)
	{
		return _skill.RunRuneSkill(data, object_interface(this),target,size,next_interval,level);
	}
	
	inline int ContinueRuneSkill(SKILL::Data & data,const XID * target,int size,int & next_interval,int elapse_time,int level) 
	{
		return _skill.ContinueRuneSkill(data, object_interface(this),target,size,next_interval,elapse_time,level);
	}

	inline bool RuneSkillOnAttacked(SKILL::Data & data,int level)
	{
		return _skill.InterruptRuneSkill(data, object_interface(this),level);
	}

	inline bool CancelRuneSkill(SKILL::Data & data,int level)
	{
		return _skill.CancelRuneSkill(data, object_interface(this),level);
	}

	inline int CastRuneInstantSkill(SKILL::Data & data, const XID * target,int size,int level)
	{
		return _skill.RuneInstantSkill(data,object_interface(this),target,size,level);
	}

	inline int GetProduceSkillLevel(int id)
	{
		return _skill.GetProduceSkillLevel(id);	
	}

	inline void ActivateDynSkill(int id, int counter)
	{
		return _skill.ActivateDynSkill(id, counter);
	}

	inline void DeactivateDynSkill(int id, int counter)
	{
		return _skill.DeactivateDynSkill(id, counter);
	}
	
public:	
	//lgc
	virtual bool IsElfEquipped(){return false;}
	virtual void UpdateCurElfInfo(unsigned int id, short refine_level, short str, short agi, short vit, short eng, const char * skilldata, int cnt){}
	virtual void ClearCurElfInfo(){}
	virtual void ClearCurElfVigor(){}
	virtual void UpdateElfProp(){}
	virtual void UpdateElfVigor(){}
	virtual void UpdateMinElfStatusValue(int value){}
	virtual void TriggerElfRefineEffect(){}
	virtual bool IsElfRefineEffectActive(){return false;}
	virtual void UpdateStallInfo(int id, int max_sell, int max_buy, int max_name){}
	virtual void ClearStallInfo(){}
	virtual void OnStallCardTakeOut(){}
	
	//obj_interface接口	
	virtual bool OI_GetElfProp(short& level, short& str, short& agi, short& vit, short& eng){return false;}
	virtual int OI_GetElfVigor(){return -1;}
	virtual int OI_GetElfStamina(){return -1;}
	virtual bool OI_DrainElfVigor(int dec){return false;}
	virtual bool OI_DrainElfStamina(int dec){return false;}
};


/*	active object 的对AI的包装借口*/

class  ai_actobject : public ai_object
{
	protected:
		gactive_imp * _imp;
	public:
		ai_actobject(gactive_imp * imp):_imp(imp)
		{}

		//destructor
		virtual ~ai_actobject()
		{
		}

		virtual gactive_imp * GetImpl()
		{
			return _imp;
		}

		//property
		virtual void GetID(XID & id)
		{
			id = _imp->_parent->ID;
		}

		virtual void GetPos(A3DVECTOR & pos)
		{
			pos = _imp->_parent->pos;
		}

		//virtual int GetState() = 0;

		virtual int GetHP()
		{
			return _imp->_basic.hp;
		}

		virtual int GetMaxHP()
		{
			return _imp->_cur_prop.max_hp;
		}

		virtual int GenHP(int hp)
		{
			return _imp->GenHP(hp);
		}

		virtual int GetMP()
		{
			return _imp->_basic.mp;
		}

		virtual float GetAttackRange()
		{
			return _imp->_cur_prop.attack_range;
		}

		virtual float GetMagicRange(unsigned int id,int level)
		{	
			return _imp->NPCSkillRange(id,level);
		}
		
		virtual float GetBodySize()
		{
			return _imp->_parent->body_size;
		}

		virtual int GetFaction()
		{
			return _imp->_faction;
		}

		virtual int GetEnemyFaction()
		{
			return _imp->_enemy_faction;
		}

		virtual int GetAntiInvisibleDegree()
		{
			return ((gactive_object*)_imp->_parent)->anti_invisible_degree;	
		}

		//operation
		virtual void AddSession(act_session * pSession)
		{
			_imp->AddSession(pSession);
			_imp->StartSession();
		}

		virtual bool HasSession()
		{
			return _imp->_cur_session || _imp->_session_list.size();
		}

		virtual void ClearSession()
		{
			_imp->ClearSession();
		}

		virtual void SendMessage(const XID & id, int msg);
		virtual void ActiveCombatState(bool state)
		{
			_imp->ActiveCombatState(state);
		}

		virtual bool GetCombatState()
		{
			return _imp->IsCombatState();
		}

		virtual int GetAttackSpeed()
		{
			return _imp->_cur_prop.attack_speed;
		}
	public:

		virtual int QueryTarget(const XID & id, target_info & info);
		virtual int GetSealMode()
		{
			return _imp->GetSealMode();
		}
};
#endif
