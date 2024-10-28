#ifndef __SKILL_SKILLWRAPPER_H
#define __SKILL_SKILLWRAPPER_H

#include <map>
#include <vector>
#include <list>

#include "common/types.h"
#include "common/base_wrapper.h"
#include "obj_interface.h"

namespace SKILL
{
#pragma pack(1)
	struct Data
	{
		unsigned int id;

		char	forceattack;    // 传入参数，是否强制攻击
		bool	skippable;      // 传出参数，当前状态是否可提前结束
		int	stateindex;     // 当前状态ID
		int	nextindex;      // 下个状态ID
		int	warmuptime;     // 蓄力时间
		unsigned char section; // 技能段数
		int		lvalue;		// 策划用临时参数，state间传递状态时使用
		Data(unsigned int i) : id(i),forceattack(0),skippable(false),stateindex(-1),nextindex(-1),warmuptime(-1),section(0),lvalue(0)
		{ 
		}
	};
	struct elf_requirement
	{
		short elf_level;
		short str;
		short agi;
		short vit;
		short eng;
		short genius[5];
	};
#pragma pack()

}

namespace GNET
{
enum error_code{
	SKILL_SUCCESS = 0,
	SKILL_INVALIDID = 1,
	SKILL_WRONGPOSITION = 2,
	SKILL_OUTOFRANGE = 3,
	SKILL_WRONGWEAPON = 4,
	SKILL_UNUSABLE = 5,
};

enum WeaponClass {
	WEAPONCLASS_SWORD    = 1,
	WEAPONCLASS_SPEAR    = 5,
	WEAPONCLASS_AXE      = 9,
	WEAPONCLASS_BOW      = 13,
	WEAPONCLASS_BOXING   = 182,
	WEAPONCLASS_WAND	 = 292,
	WEAPONCLASS_DAGGER	 = 23749,
	WEAPONCLASS_TALISMAN = 25333,
	WEAPONCLASS_SCIMITAR = 44878,
	WEAPONCLASS_SCYTHE	 = 44879,
	WEAPONCLASS_FEATHER  = 65535
};

enum CashResurrectBuff
{
    GIANT,
    BLESSMAGIC,
    STONESKIN,
    INCRESIST,
    INCHP,
    IRONSHIELD,

    BUFF_COUNT,
};


#define IS_RANGE_WEAPON(wt)	(wt == WEAPONCLASS_BOW || wt == WEAPONCLASS_WAND || wt ==  WEAPONCLASS_TALISMAN || wt == WEAPONCLASS_SCYTHE)

class Skill;
class SkillWrapper
{
public:
	typedef unsigned int	ID;


protected:
	struct PersistentData
	{
		int 	ability;    // 熟练度
		int	level;      // 级别
		char	overridden; // 被高级技能覆盖

		PersistentData(long __t = 0, int __l = 1) : ability(__t), level(__l), overridden(0){ }
		int GetLevel() const { return level; }
	};

	struct DynamicPray
	{
		int   speed;
		int	  times;	
	};

	typedef std::map<ID,PersistentData>	StorageMap;
	typedef std::map<ID,DynamicPray> DynPrayMap;

	StorageMap	   map;
	StorageMap	   dyn_map;		//动态技能
	DynPrayMap	   dynpray_map; //动态修正吟唱	

	int prayspeed;  
	ID  asid;                      // 激活技能ID
	int aslevel;                   // 激活技能级别
	float  skillinc[MAGIC_CLASS];  // 0金 1木 2水 3火 4土
	int immune_buff_debuff;		//计数值, 0 不免疫 >=1 免疫所有状态,lgc
	int interrupt_prob;			//被攻击时技能的额外中断概率
	int ignore_interrupt;		// 忽略被攻击时中断
	int cd_adjust;				// cd调整 毫秒
	int cd_adjust_count;		// cd调整次数
	float pray_distance_adjust;	// 吟唱距离调整(米)

	struct ComboSkill
	{
		enum { MAX_COMBO_ARG = 3, TOLERANCE = 2};
		enum {
			CBREAK_ALL,				//所有技能都能中断的前置技准备
			CBREAK_IGNORE_NORMAL,	//只有前置技能才能中断的前置技准备
			CBREAK_IGNORE			//无法中断的前置技准备
		};

		ID  skillid;
		int timestamp;
		int breaktype;
		int expire;
		int arg[MAX_COMBO_ARG];
		bool Condition(ID id, int interval, int now)
		{
			return id == skillid && (timestamp + interval + TOLERANCE >= now);
		}
		void SetState(ID s, int st, int type, int ex) 
		{ 
			skillid = s; timestamp = st; breaktype = type; expire = ex;
		}
		void Clear()
		{
			skillid = timestamp = breaktype = expire = 0;
			memset(arg,0,sizeof(arg));
		}
		bool Break(int now, bool sflag)
		{
			if(skillid == 0 || breaktype == CBREAK_ALL) 
				return true;
			if(timestamp + expire <= now)
				return true;
			if(sflag && breaktype == CBREAK_IGNORE_NORMAL)
				return true;
			return false;
		}
		int	 GetArg(int i) { return i < MAX_COMBO_ARG ? arg[i] : 0;}
		bool SetArg(int i,int v) { if(i >= MAX_COMBO_ARG) return false; arg[i] = v; return true;}
		
		void Swap(ComboSkill& cs) 
		{ 
			std::swap(skillid,cs.skillid); 
			std::swap(timestamp,cs.timestamp); 
			std::swap(breaktype,cs.breaktype);
			std::swap(expire,cs.expire);
			for(int i = 0 ; i < MAX_COMBO_ARG; ++ i) std::swap(arg[i],cs.arg[i]);
		}
		void Save( archive & ar )
		{
			ar << skillid << timestamp << breaktype << expire;
			for(int i = 0 ; i < MAX_COMBO_ARG; ++ i) ar << arg[i];
		}
		void Load( archive & ar )
		{
			ar >> skillid >> timestamp >> breaktype >> expire;
			for(int i = 0 ; i < MAX_COMBO_ARG; ++ i) ar >> arg[i];
		}

	}combo_state;

	struct BlackWhiteBall
	{
		enum
		{
			BWB_BLACK = 1,
			BWB_WHITE = 5,
			BWB_VALUE_MAX = 15,
			BWB_COUNT_MAX = 3,
		};
		int UpdateVstate(int& oldv);
		bool Add(int type)	
		{
			if(type != BWB_BLACK && type != BWB_WHITE) return false;
			balls = (balls*10)%1000 + type;
			return true;
		}
		void Flip()
		{
			int ba[BWB_COUNT_MAX] = { balls%10,(balls%100)/10,balls/100 };
			for(int i = 0; i < BWB_COUNT_MAX; ++i)
			{
				switch(ba[i])
				{
					case BWB_BLACK:
						ba[i] = BWB_WHITE;	
						break;
					case BWB_WHITE:
						ba[i] = BWB_BLACK;
						break;
					default:
						ba[i] = 0;
						break;
				}
			}
			balls = ba[2]*100 + ba[1]*10 + ba[0];
		}
		int GetIndex() const	{ return balls/100+(balls%100)/10+(balls%10);	}

		void Swap(BlackWhiteBall& bw) 
		{ 
			std::swap(balls,bw.balls); 
			std::swap(vstate,bw.vstate); 
		}
		void Save( archive & ar )
		{
			ar << balls << vstate;
		}
		void Load( archive & ar )
		{
			ar >> balls >> vstate;
		}

		int balls; 
		int vstate;
	}black_white_ball;
private:
	int GetDynPrayspeed(ID id)
	{
		DynPrayMap::iterator iter = dynpray_map.find(id);
		int speed = 0;
		if(iter != dynpray_map.end())
		{
			speed = iter->second.speed;
			if(--iter->second.times <= 0) dynpray_map.erase(iter);
		}

		return speed;		
	}

public:
	SkillWrapper();

	// 技能类别 1.主动攻击，2.主动祝福，3.主动诅咒，4.召唤，5.被动，6.激活, 7.生活，8.瞬移, 9.生产
	static char GetType(ID id); 	
	// 技能攻击范围：0点 1线 2自身球 3目标球 4圆锥形 5自身
	static char RangeType(ID id);
	static bool Initialize();
	static bool IsInstant(ID id);		//是否瞬发
	static bool IsPosSkill(ID id);		//是否瞬移
	static bool IsMovingSkill(ID id);	//是否移动技能
	
	int Learn( ID id, object_interface player );	// 返回新的级别，错误返回-1
	int Learn( ID id, object_interface player, int level );
	int Remove(ID id);   // 0 成功， 1 未找到， 2 不能删除
	void GodEvilConvert(std::map<int,int>& convert_table, object_interface player, int weapon_class, int form, int worldtag);	//仙魔转换
	void ActivateDynSkill(ID id, int counter);
	void DeactivateDynSkill(ID id, int counter);
	int GetDynSkillCounter(ID id);

	int Condition( ID id, object_interface player, const XID * target, int size ); // 返回 error_code

	// 返回值为下次调用时间间隔(毫秒),-1表示结束
	int StartSkill( SKILL::Data & skilldata, object_interface player, const XID * target, int size, int & next_interval);
	int Run( SKILL::Data & skilldata, object_interface player, const XID * target, int size, int & next_interval );
	int StartSkill( SKILL::Data & skilldata, object_interface player, const A3DVECTOR& pos,const XID * target, int size, int & next_interval);
	int Run( SKILL::Data & skilldata, object_interface player, const A3DVECTOR& pos, const XID * target, int size, int & next_interval );
	// 打断，受攻击时调用
	bool Interrupt( SKILL::Data & skilldata, object_interface player );
	// 玩家主动取消正在执行的技能
	bool Cancel( SKILL::Data & skilldata, object_interface player );
	// 提前结束当前状态，例如蓄力
	int Continue( SKILL::Data& skilldata, object_interface player, const XID* target,int size, int& next_interval,int elapse_time);

	// 使用瞬发技能, 0 成功，-1 失败
	int InstantSkill( SKILL::Data & skilldata, object_interface player, const XID * target, int size);

	// 使用物品附加技能, 0 成功, -1 失败
	int CastRune(SKILL::Data & skilldata, object_interface player, int level);

	//使用物品附加技能
	int StartRuneSkill(SKILL::Data& skilldata, object_interface player, const XID* target, int size, int& next_interval, int level);
	int RunRuneSkill( SKILL::Data & skilldata, object_interface player, const XID * target, int size , int& next_interval, int level);
	int ContinueRuneSkill( SKILL::Data& skilldata, object_interface player, const XID* target, int size, int& next_interval, int elapse_time, int level);
	bool InterruptRuneSkill( SKILL::Data & skilldata, object_interface player, int level);
	bool CancelRuneSkill( SKILL::Data & skilldata, object_interface player, int level);
	int RuneInstantSkill(SKILL::Data& skilldata, object_interface player, const XID * target, int size, int level);

	// 读取和保存PersistentData
	void LoadDatabase( archive & ar );
	void StoreDatabase( archive & ar );
	size_t StoreDatabaseSize();

	void Load( archive & ar );
	void Store( archive & ar );

	void StorePartial( archive & ar );

	int ActivateSkill(object_interface player, ID id, int level);
	int DeactivateSkill(object_interface player, ID id, int level);
	int ActivateReboundSkill(object_interface player, ID id, int level, int trigger_prob);
	int DeactivateReboundSkill(object_interface player, ID id, int level, int trigger_prob);

	int GetSkillLevel(ID id);

	// 主动攻击效果计算
	bool Attack(object_interface target, const XID&, const A3DVECTOR&,const attack_msg& msg, bool invader);
	bool Attack(object_interface target, const XID&, const A3DVECTOR&,const enchant_msg& msg, bool invader );
	// 被动附加的状态包
	bool Infect(object_interface target, const XID&, const A3DVECTOR&,const attack_msg& msg, bool invader);
	bool Infect(object_interface target, const XID&, const A3DVECTOR&,const enchant_msg& msg, bool invader );

	// 被动技能，人物初始化
	bool EventReset(object_interface player);
	// 被动技能，人物初始化
	bool EventUnreset(object_interface player);
	// 被动技能，装备武器
	bool EventWield(object_interface player, int weapon_class );
	// 被动技能，卸载武器
	bool EventUnwield(object_interface player, int weapon_class );
	// 被动技能，变身
	bool EventChange(object_interface player, int from, int to);
	// 被动技能，场景进入
	bool EventEnter(object_interface player,int worldtag );
	bool EventLeave(object_interface player,int worldtag );

	// 返回值为下次调用时间间隔(毫秒),-1表示结束
	int NpcStart(ID id, object_interface npc, int level, const XID * target, int size, int& next_interval);
	void NpcEnd(ID id, object_interface npc, int level, const XID * target, int size );
	// 打断，受攻击时调用
	bool NpcInterrupt(ID id, object_interface npc, int level);
	float NpcCastRange(ID id, int level);

	// 修改吟唱间隔
	int IncPrayTime(int inc);
	int DecPrayTime(int dec);
	// 修改冷却时间修正值
	void IncCDAdjust(int v) { cd_adjust += v; }
	void DecCDAdjust(int v) { cd_adjust -= v; }
	int  GetCDAdjust() { if(cd_adjust_count) { --cd_adjust_count; return cd_adjust;} else return 0; }
	int  GetCDAdjustCount() const { return cd_adjust_count; }
	void SetCDAdjustCount(int count) { cd_adjust_count = count; }
	// 修改吟唱距离附加值
	float IncPrayDisAdjust(float pd) { pray_distance_adjust += pd; return pray_distance_adjust; }
	float DecPrayDisAdjust(float pd) { pray_distance_adjust -= pd; return pray_distance_adjust; }
	float GetPrayDisAdjust() const { return pray_distance_adjust; }

	int IncAbility(object_interface player, ID id, int inc);
	int GetAbility(ID id);
	float GetAbilityRatio(ID id);	//-1 出错 0-1 当前熟练度比例
	int IncAbilityRatio(object_interface player, ID id, float ratio);
	int SetSealed(object_interface player, int second);
	int SetFlager(object_interface player, float hurt_ratio, float hurt_ratio2/*近战普攻*/, float speed_ratio, float max_hp_ratio);//添加国战旗手filter
	int UnSetFlager(object_interface player);//删除国战旗手filter
	int OnCountryBattleRevive(object_interface player, int time, int ap, float physic_ratio, float magic_ratio, int time2, float incresist, float incdefense, float inchp);
	void CountryBattleWeakProtect(object_interface player, int time, int inc_atk_degree, int inc_def_degree);
	void SetChariotFilter(object_interface player, int shape, int inc_hp, int inc_defence, int inc_magic_defence[5], int inc_damage, int inc_magic_damage, float inc_speed, float inc_hp_ratio, int dyn_skill[4]);
	static int GetCooldownID(ID id);
	static int GetMpCost(ID id, object_interface npc, int level);
	static int PetLearn(ID id,int petlevel,object_interface owner,unsigned int *skills,int size); // 返回新的级别，错误返回-1
	static int ElfLearnSkill(int skill_id, int skill_level, struct SKILL::elf_requirement& elf, object_interface player); 
	static bool IsElfSkillActive(int skill_id, int skill_level, struct SKILL::elf_requirement& elf, object_interface player); 
	int RunElfSkill( SKILL::Data & skilldata, int skill_level, object_interface player, const XID *target, int size);
public:
	void SetSkillTalent(Skill* skill, const int* list);

	void IncImmuneBuffDebuff(){	immune_buff_debuff ++;}	//lgc
	void DecImmuneBuffDebuff(){	immune_buff_debuff --;}

	void IncInterruptProb(int prob) { interrupt_prob += prob; }
	void DecInterruptProb(int prob) { interrupt_prob -= prob; }

	void IncIgnoreInterrupt(int ignore) { ignore_interrupt += ignore; }
	void DecIgnoreInterrupt(int ignore) { ignore_interrupt -= ignore; }

	int GetPraySpeed(){ return prayspeed; }
	
	// Following interfaces are intended for internal use
	int GetAsid() { return asid; }
	int GetAslevel() { return aslevel; }

	void SetLevel(ID id, int level);
	void OverrideSkill(const std::vector<std::pair<ID,int> > & pre_skills);

	bool SetSkillInc(int magic, float inc);
	float GetSkillInc(int magic);
	void Swap(SkillWrapper&);
	int  GetProduceSkill();

	int  GetLevel(ID id, int cls ,bool use=false);
	bool TestCommonCoolDown(int cd_mask, object_interface player);

	int  GetProduceSkillLevel(ID id);

	void ModifyDynamicPray(ID id, float ratio, int times);
	int  GetDynamicPrayTimes(ID id); 

	void OnSkillPerform(ID id, ID perid, object_interface player);
	void OnComboPreSkillEnd(Skill* skill, object_interface player);
	bool SetComboSkillArg(int index, int arg) { return combo_state.SetArg(index,arg);} 
	int  GetComboSkillArg(int index) { return combo_state.GetArg(index);}
	void SetComboState(ID id, int stime,int type,int ex) { combo_state.SetState(id,stime,type,ex); }
	void SetComboState(ID id) { combo_state.skillid = id; }
	ID   GetComboState() const { return combo_state.skillid; }
	void SyncComboState(object_interface player);
	void ClearComboState() { combo_state.Clear();}
	bool CheckComboBreak(int now, bool sflag) { return combo_state.Break(now,sflag);}
	bool CheckComboState(ID id, int interval, int now) { return combo_state.Condition(id,interval,now);}

	int  GetBlackWhiteBalls() { return black_white_ball.GetIndex(); }
	bool AddBlackWhiteBalls(int ball, int& new_vstate, int& old_vstate, int& hstate); 
	bool FlipBlackWhiteBalls(int& new_vstate, int& old_vstate, int& hstate);
	void SoloChallengeAddFilter(object_interface player, int filter_id, float *param);
	void MnFactionAddFilter(object_interface player, float ratio);
    void ResurrectByCashAddFilter(object_interface player, int buff_period, const float* buff_ratio, int buff_size);

	virtual ~SkillWrapper(){}
};

};

#endif

