#ifndef __SKILL_SKILL_H
#define __SKILL_SKILL_H

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <wchar.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include "/usr/include/assert.h"
#include "range.h"
#include "common/types.h"
#include "attack.h"
#include "playerwrapper.h"
#include "targetwrapper.h"

#ifdef INTEPRETED_EXPR 
#include "marshal.h"
#endif

#pragma pack(1)
namespace SKILL
{
	struct Data;
}
struct enchant_msg;
struct attack_msg;
struct XID;

namespace GNET
{
class  PlayerWrapper;
class  TargetWrapper;
class Skill;

enum { 
	TYPE_ATTACK    = 1,  // 主动攻击
	TYPE_BLESS     = 2,  // 主动祝福
	TYPE_CURSE     = 3,  // 主动诅咒
	TYPE_RUNE      = 4,  // 物品技能
	TYPE_PASSIVE   = 5,  // 被动
	TYPE_ENABLED   = 6,  // 武器附加
	TYPE_LIVE      = 7,  // 生活
	TYPE_JUMP      = 8,  // 瞬移
	TYPE_PRODUCE   = 9,  // 生产(互斥)
	TYPE_BLESSPET  = 10, // 宠物祝福
	TYPE_NEUTRALBLESS = 11, //中性祝福
	TYPE_NEUTRALBLESS2 = 12, //中性祝福2
};

enum // 同gs/playertemplate.h
{   
	USER_CLASS_SWORDSMAN,       //武侠
	USER_CLASS_MAGE,            //法师
	USER_CLASS_NEC,             //巫师
	USER_CLASS_HAG,             //妖精
	USER_CLASS_ORGE,            //妖兽
	USER_CLASS_ASN,             //刺客
	USER_CLASS_ARCHER,          //羽芒
	USER_CLASS_ANGEL,           //羽灵
	USER_CLASS_BLADE,           //剑灵
	USER_CLASS_GENIE,           //魅灵
	USER_CLASS_SHADOW,          //夜影
	USER_CLASS_FAIRY,           //月仙
	USER_CLASS_COUNT
};

enum EVENT{
	EVENT_RESET  = 0x01,
	EVENT_WIELD  = 0x02,
	EVENT_CHANGE = 0x04,
	EVENT_ENTER  = 0x08,
};

enum FORM
{
	FROM_MASK_HIGH 	= 0xC0,
	FORM_NORMAL		= 0,	//正常形态
	FORM_CLASS		= 1,	//职业变身
	FORM_BEASTIE	= 2,	//小动物
};

/* 状态攻击类存根，保存一些静态数据 */
#ifdef INTEPRETED_EXPR 
class StateAttackStub : public Marshal
#else
class StateAttackStub
#endif
{
public:
	typedef unsigned int ID;

	ID id;
	std::string	name;
	std::string	icon;
	std::string	effect;

public:
	StateAttackStub(ID i,std::string n,std::string ic,std::string e)
		: id(i), name(n), icon(ic), effect(e)
	{ if(!GetStub(id)) GetMap().insert(std::make_pair(id, this)); }
	virtual ~StateAttackStub()	{ }

	typedef std::map<ID, const StateAttackStub*> Map;
	static Map& GetMap() { static Map map; return map; }
	static const StateAttackStub *GetStub(ID i)
	{
		Map::const_iterator it = GetMap().find(i);
		return it == GetMap().end() ? NULL : (*it).second;
	}

#ifdef INTEPRETED_EXPR 
	virtual OctetsStream& marshal(OctetsStream &os)	const
	{ return os << id << name << icon << effect; }
	virtual const OctetsStream& unmarshal(const OctetsStream &os)
	{ return os >> id >> name >> icon >> effect; }
#endif
};

/* 技能类存根，保存一些静态数据 */
#ifdef INTEPRETED_EXPR 
class SkillStub : public Marshal
{
public:
	class State : public Marshal
#else
class SkillStub 
{
public:
	class State
#endif
	{
	public:
		int				stateid;
		std::string		name;
		State() { }
		virtual ~State() { }
#ifdef INTEPRETED_EXPR 
		std::string		time_exp;
		std::string		quit_exp;
		std::string		loop_exp;
		std::string		bypass_exp;
		std::string		calculate_exp;
		std::string		interrupt_exp;
		std::string		cancel_exp;
		std::string		skip_exp;
	public:
		State(int id, std::string n,std::string te, std::string ee, std::string le, std::string be,
				std::string ce, std::string ie, std::string cae, std::string ske )
				: stateid(id), name(n), time_exp(te), quit_exp(ee), loop_exp(le), bypass_exp(be),
				calculate_exp(ce), interrupt_exp(ie), cancel_exp(cae), skip_exp(ske)
		{ }
		State(const State& rhs) : stateid(rhs.stateid), name(rhs.name), time_exp(rhs.time_exp),
				quit_exp(rhs.quit_exp), loop_exp(rhs.loop_exp), bypass_exp(rhs.bypass_exp),
				calculate_exp(rhs.calculate_exp), interrupt_exp(rhs.interrupt_exp), cancel_exp(rhs.cancel_exp),
				skip_exp(rhs.skip_exp)
		{ }
		State& operator = (const State& rhs)
		{
			stateid = rhs.stateid;
			name = rhs.name;
			time_exp = rhs.time_exp;
			quit_exp = rhs.quit_exp;
			loop_exp = rhs.loop_exp;
			bypass_exp = rhs.bypass_exp;
			calculate_exp = rhs.calculate_exp;
			interrupt_exp = rhs.interrupt_exp;
			cancel_exp = rhs.cancel_exp;
			skip_exp = rhs.skip_exp;
			return *this;
		}

		virtual OctetsStream& marshal(OctetsStream &os)	const
		{
			os << stateid << name << time_exp << quit_exp << loop_exp << bypass_exp;
			os << calculate_exp << interrupt_exp << cancel_exp << skip_exp;
			return os;
		}
		virtual const OctetsStream& unmarshal(const OctetsStream &os)
		{
			os >> stateid >> name >> time_exp >> quit_exp >> loop_exp >> bypass_exp;
			os >> calculate_exp >> interrupt_exp >> cancel_exp >> skip_exp;
			return os;
		}

		virtual int  GetTime(Skill * skill) const;
		virtual bool Quit(Skill * skill) const;
		virtual bool Loop(Skill * skill) const;
		virtual bool Bypass(Skill * skill) const;
		virtual void Calculate(Skill * skill) const;
		virtual bool Interrupt(Skill * skill) const;
		virtual bool Cancel(Skill * skill) const;
		virtual bool Skip(Skill * skill) const;
#else
	public:
		State(int id, std::string n) : stateid(id), name(n) { }
		State(const State& rhs) : stateid(rhs.stateid), name(rhs.name) { }
		State& operator = (const State& rhs)
		{
			stateid = rhs.stateid;
			name = rhs.name;
			return *this;
		}
		virtual int  GetTime(Skill * skill) const = 0;
		virtual bool Quit(Skill * skill) const = 0;
		virtual bool Loop(Skill * skill) const = 0;
		virtual bool Bypass(Skill * skill) const = 0;
		virtual void Calculate(Skill * skill) const = 0;
		virtual bool Interrupt(Skill * skill) const = 0;
		virtual bool Cancel(Skill * skill) const = 0;
		virtual bool Skip(Skill * skill) const = 0;
#endif
	};

	typedef unsigned int ID;

	enum { MIN_LEVEL = 1, MAX_LEVEL = 10 };

public:
	// base info
	ID	id;		// 唯一数字标识
	int	cls;		// 职业
	std::basic_string<wchar_t>	name;	// 技能名字
	std::string		nativename;	// 技能名
	std::string		icon;		// 技能图标
	int	max_level;	// 技能最大级别
	char	type;				
	char	attr;		// 主技能属性, 1物理、2金、3木、4水、5火、6土 六选一 默认（物理）
	int	rank;		// 修真级别
	int	eventflag;	// 被动技能注册事件
	int	npcdelay;	// NPC响应延时
	int	showorder;	// 显示顺序
	int	apgain;		// 怒气值增加
	int	apcost;		// 怒气值消耗
	int	arrowcost;	// 箭支消耗

	// learn condition
	std::vector<std::pair<ID,int> > pre_skills;
	bool	is_senior;	// 覆盖前提技能
	bool	is_inherent;//天生技能，不用学就能用
	bool	is_movingcast;// 移动释放

	// execute condition
	bool	allow_land;	// 陆地有效
	bool	allow_air;	// 空中有效
	bool	allow_water;	// 水中有效
	bool	allow_ride;	// 骑乘有效
	bool 	notuse_in_combat;	//战斗状态不可用
	char 	restrict_corpse;// 目标类型限制
	bool	auto_attack;	// 自动攻击
	char	time_type;	// 瞬发技能
	char	allow_forms;	// 允许的变身状态
	char	long_range;	// 远程攻击
	char	posdouble;	// 空地选择
	int     clslimit;       // 小精灵技能职业限制
	std::vector<int>	restrict_weapons;	// 武器限制

	// effect
	std::string	effect;

	// range
	Range	range;

	// state
	typedef std::vector<State*>	StateVector;
	StateVector	statestub;

	bool	doenchant;		// 是否有对目标的状态攻击
	bool	dobless;		// 是否有对自己的祝福
	
	//公共冷却
	int commoncooldown;		//公共冷却mask bit0-4 技能冷却0-4 bit5-9 物品冷却0-4
	int commoncooldowntime;	//公共冷却时间 毫秒

	//使用时消耗物品
	int itemcost;
	//连续技
	int combosk_preskill;
	int combosk_interval;
	int combosk_nobreak;
	
protected:
	SkillStub(ID i) : id(i), is_senior(0),is_inherent(false),is_movingcast(false),notuse_in_combat(false),posdouble(0),clslimit(0),itemcost(0),combosk_preskill(0),combosk_interval(0),combosk_nobreak(0)
	{ 
		if(!GetStub(id)) 
		{
			GetMap().insert(std::make_pair(id, this));
		}
	}
	virtual ~SkillStub()	{ Clear(); }
	virtual void Clear();

	typedef std::map<ID, const SkillStub*> Map;
	static Map& GetMap() { static Map map; return map; }

	typedef std::map<ID, int> ComboMap; 
	static ComboMap& GetComboPreskill() { static ComboMap cmap; return cmap; }
	
	typedef std::set<ID> Set;
	static Set& GetClsInherent(int cls) { static Set clsinhmap[USER_CLASS_COUNT+1]; return clsinhmap[cls < USER_CLASS_COUNT ? cls : USER_CLASS_COUNT ];}
	
	bool LearnCondition(Skill * skill ) const;

public:
	static const SkillStub *GetStub(ID i)
	{
		Map::const_iterator it = GetMap().find(i);
		return it == GetMap().end() ? NULL : (*it).second;
	}

	static bool IsClsInherent(ID i,int cls)
	{
		return GetClsInherent(cls).find(i) != GetClsInherent(cls).end();
	}

	static bool IsComboPreskill(ID i)
	{
		return GetComboPreskill().find(i) != GetComboPreskill().end();
	}

	static int GetComboExpire(ID i)
	{
		ComboMap::iterator iter = GetComboPreskill().find(i);
		return iter != GetComboPreskill().end() ? iter->second : 0;
	}

	static void Initialize()
	{
		Map::const_iterator iter = GetMap().begin();
		Map::const_iterator iend = GetMap().end();

		for(;iter != iend; ++iter)
		{
			const SkillStub* sk = iter->second;
			if(sk->is_inherent)	GetClsInherent(sk->cls).insert(sk->id); 
			if( sk->combosk_preskill &&	GetComboExpire(sk->combosk_preskill) < sk->combosk_interval)
				GetComboPreskill()[sk->combosk_preskill] = sk->combosk_interval;
		}
	}

	size_t GetStateSize( ) const { return statestub.size(); }
	const State * GetState( int index ) const { 
		assert((unsigned int)index<statestub.size());
		return statestub[index]; 
	}

	ID GetId() const { return id; }
	int GetCls() const { return cls; }
	const wchar_t * GetName() const { return name.c_str(); }
	const std::string & GetIcon() const { return icon; }
	int GetMaxLevel() const { return max_level; }
	char GetType() const { return type; }
	char GetAttr() const { return attr; }
	bool IsAttack() const { return  TYPE_ATTACK == type; }
	bool IsBless() const { return TYPE_BLESS == type; }
	bool IsCurse() const { return TYPE_CURSE == type; }
	bool IsRune() const { return TYPE_RUNE == type; }
	bool IsPassive() const { return TYPE_PASSIVE == type; }
	bool IsEnabled() const { return TYPE_ENABLED == type; }
	bool IsInstant() const { return time_type==1; }
	bool GetAllowLand() const { return allow_land; }
	bool GetAllowAir() const { return allow_air; }
	bool GetAllowWater() const { return allow_water; }
	bool GetAllowRide() const { return allow_ride; }
	bool GetNotuseInCombat() const { return notuse_in_combat; }
	char GetRestrictCorpse() const { return restrict_corpse; }
	bool DoEnchant() const { return doenchant; }
	bool DoBless() const { return dobless; }
	int GetEventFlag() const { return eventflag; }
	const Range & GetRange() const { return range; }
	int GetItemCost(){ return itemcost; }
	bool IsComboSkill() const { return combosk_preskill > 0; }
	int  GetComboPreSkill() const { return combosk_preskill; }
	int  GetComboPreInterval() const { return (int)(0.001*combosk_interval); }
	int  GetComboBreakType() const { return combosk_nobreak;}
	bool IsMovingSkill() const { return is_movingcast; }

	bool Learn(Skill * skill) const;
	int Condition(Skill *skill) const;
	int ElfCondition(Skill *skill) const;

#ifdef INTEPRETED_EXPR 
	static void Store( std::string filename );
	static bool Load( std::string filename );
	virtual int GetExecutetime(Skill *skill) const;
	virtual int GetCoolingtime(Skill *skill) const;
	virtual int GetEnmity(Skill* skill) const;
	virtual int GetRequiredSp(Skill* skill) const;
	virtual int GetRequiredLevel(Skill* skill) const;
	virtual int GetRequiredItem(Skill* skill) const;
	virtual int GetRequiredMoney(Skill* skill) const;
	virtual int GetMaxAbility(Skill* skill) const;
	virtual int GetRequiredRealmLevel(Skill* skill) const;
	virtual bool StateAttack(Skill * skill) const;
	virtual bool TakeEffect(Skill *skill) const;
	virtual float GetRadius(Skill *skill) const;
	virtual float GetAttackdistance(Skill *skill) const;
	virtual float GetAngle(Skill *skill) const;
	virtual float GetPraydistance(Skill *skill) const;
	virtual float GetEffectdistance(Skill *skill) const;
	virtual int GetShoworder() const { return showorder; }
	virtual int GetAttackspeed(Skill* skill) const { return 1; }
	virtual float GetMpcost(Skill* skill) const { return 0; }
	virtual bool CheckHpCondition(int hp, int max_hp) const { return true; }
	virtual bool CheckComboSkExtraCondition(Skill* skill) const { return true; }
	virtual void ComboSkEndAction(Skill* skill) const {}
#else
	virtual int GetExecutetime(Skill *skill) const = 0;
	virtual int GetCoolingtime(Skill *skill) const = 0;
	virtual int GetEnmity(Skill* skill) const = 0;
	virtual int GetRequiredSp(Skill* skill) const { return 0; }
	virtual int GetRequiredLevel(Skill* skill) const { return 0; }
	virtual int GetRequiredItem(Skill* skill) const { return 0; }
	virtual int GetRequiredMoney(Skill* skill) const { return 0; }
	virtual int GetMaxAbility(Skill* skill) const { return 0; }
	virtual int GetRequiredRealmLevel(Skill* skill) const { return 0; }
	virtual bool StateAttack(Skill * skill) const { return 0; }
	virtual bool BlessMe(Skill * skill) const { return 0; }
	virtual float GetRadius(Skill *skill) const = 0;
	virtual float GetAttackdistance(Skill *skill) const = 0;
	virtual float GetAngle(Skill *skill) const = 0;
	virtual float GetPraydistance(Skill *skill) const = 0;
	virtual float GetEffectdistance(Skill *skill) const = 0;
	virtual float GetTalent0 (PlayerWrapper * player) const { return 0; }
	virtual float GetTalent1 (PlayerWrapper * player) const { return 0; }
	virtual float GetTalent2 (PlayerWrapper * player) const { return 0; }

	virtual int GetShoworder() const { return showorder; }
	virtual int GetAttackspeed(Skill* skill) const { return 1; }
	virtual float GetMpcost(Skill* skill) const { return 0; }
	virtual bool TakeEffect(Skill *skill) const { return true; }
	virtual float GetHitrate(Skill *skill) const { return 1.0; }
	virtual bool CheckHpCondition(int hp, int max_hp) const { return true; }
	virtual bool CheckComboSkExtraCondition(Skill* skill) const { return true; }
	virtual void ComboSkEndAction(Skill* skill) const {}
#endif
};

class Skill
{
public:
	typedef unsigned int ID;

protected:
	ID					id;
	int                 level;

	const SkillStub     * stub;
	PlayerWrapper       * player;
	TargetWrapper       * target;
	SKILL::Data  * pdata;
	const enchant_msg   * enchantmsg;  
	const attack_msg    * attackmsg;  

	XID                 performerid;
	int                 performerlevel;
	A3DVECTOR           performerpos;

	float	ratio; 
	float	plus;          
	int	damage;
	int	magic;
	int	degree;
	int	magicdamage[5];	// 0金 1木 2水 3火 4土
	int	t0;
	int	t1;
	int	t2;

	typedef std::map<ID, const Skill*> Map;
	static Map& GetMap() { static Map map; return map; }

	friend class SkillStub;
public:
	Skill(ID i=0) : id(i), level(0), stub(SkillStub::GetStub(i)), player(NULL), pdata(NULL), enchantmsg(NULL), attackmsg(NULL), performerid(0,0), 
			ratio(0), plus(0), damage(0), t0(0), t1(0), t2(0)
	{	
		if(!GetStub(id)) 
			GetMap().insert(std::make_pair(id, this));	
		memset(magicdamage,0,sizeof(magicdamage));
		magic = 0;
		degree = 0;
	}

	Skill(const Skill &r) : id(r.id), level(r.level), stub(r.stub), player(r.player),
				pdata(r.pdata), enchantmsg(r.enchantmsg), attackmsg(r.attackmsg), 
				performerid(r.performerid),ratio(r.ratio), plus(r.plus), damage(r.damage)
	{ 
		memcpy(magicdamage,r.magicdamage,sizeof(magicdamage));
		magic = r.magic;
		degree = r.degree;
	}

	virtual ~Skill()		{ Clear(); }
	virtual void Clear()	{ }

	static	ID NextSkill(ID start)
	{
		static Map::iterator it = GetMap().begin();
		if(start==0)
			it = GetMap().begin();
		else 
			it++;
		if(it==GetMap().end())
			return 0;
		return (*it).second->GetId();
	}

public:
	static const Skill *GetStub(ID i)
	{
		Map::const_iterator it = GetMap().find(i);
		return it == GetMap().end() ? NULL : (*it).second;
	}

	virtual Skill * Clone() const { return new Skill(*this); }

	static Skill * Create(ID i)
	{
		const Skill * sk = GetStub(i);
		return sk?sk->Clone():NULL;
	}

	void SetId(ID i) { id = i; }
	ID GetId() const { return id; }
	const std::vector<std::pair<ID,int> > & GetJunior() const { return stub->pre_skills; }

	void SetLevel(int l) {  level = l;}
	int GetLevel() const { return level; }

	void Destroy() { delete this; }

	void SetMessage(const enchant_msg* msg) 
	{ 
		performerlevel = msg->ainfo.level;
		magic = msg->skill_reserved1;
		enchantmsg = msg; 
	}
	void SetMessage(const attack_msg* msg) 
	{ 
		damage = msg->physic_damage;
		memcpy( magicdamage, msg->magic_damage, sizeof(magicdamage));
		performerlevel = msg->ainfo.level;
		attackmsg = msg; 
	}

	const SkillStub * GetStub() const { return stub; }

	size_t GetStateSize( ) const { return stub->GetStateSize(); }
	bool IsStateEnd() { return GetNextindex() == -1;  }

	void SetStateindex(int s);
	int GetStateindex();

	void SetNextindex(int s);
	int GetNextindex();

	void SetData(SKILL::Data* data) { pdata = data; }
	const SKILL::Data& GetData() { return *pdata; }

	bool Learn()              { return stub->Learn(this); }
	bool StateAttack()        { return stub->StateAttack(this); }
	bool BlessMe()            { return stub->BlessMe(this); }
	bool IsInstant()          { return stub->time_type==1; }
	bool IsDurative()         { return stub->time_type==2; }
	bool IsWarmup()           { return stub->time_type==3; }
	bool IsRune()             { return stub->type==TYPE_RUNE; }
	bool IsProduceSkill()     { return stub->type==TYPE_PRODUCE; }
	int Condition()           { return stub->Condition(this); }
	int ElfCondition()        { return stub->ElfCondition(this); }
	int GetExecutetime()      { return stub->GetExecutetime(this); }
	int GetCoolingtime()      { return (int)(0.001*stub->GetCoolingtime(this)); }
	int GetEnmity()           { return stub->GetEnmity(this); }
	int GetRequiredSp()       { return stub->GetRequiredSp(this); }
	int GetRequiredLevel()    { return stub->GetRequiredLevel(this); }
	int GetRequiredItem()     { return stub->GetRequiredItem(this); }
	int GetRequiredMoney()    { return stub->GetRequiredMoney(this); }
	int GetMaxability()       { return stub->GetMaxAbility(this); }
	int GetRequiredRealmLevel(){ return stub->GetRequiredRealmLevel(this); }
	int GetAttackspeed()      { return stub->GetAttackspeed(this); }
	int GetMaxLevel()         { return stub->max_level; }
	int GetAbility()          { return 0; }
	int GetApcost()           { return stub->apcost; }
	int GetApgain()           { return stub->apgain; }
	float GetRadius()         { return stub->GetRadius(this); }
	float GetAttackdistance() { return stub->GetAttackdistance(this); }
	float GetAngle()          { return stub->GetAngle(this); }
	float GetPraydistance()   { return stub->GetPraydistance(this); }
	float GetEffectdistance() { return stub->GetEffectdistance(this); }
	float GetHitrate()        { return stub->GetHitrate(this); }
	bool  DoBless() const     { return stub->dobless; }
	bool  DoEnchant() const   { return stub->doenchant; }
	bool  IsSenior() const    { return stub->is_senior; }
	char  GetRestrictCorpse() const { return stub->restrict_corpse; }
	bool  IsLongRange() const { return stub->long_range==1; }
	int   RangeAdjust() const { return stub->long_range; }
	int   PositionAdjust() const { return stub->posdouble; }
	const Range & GetRange()  { return stub->GetRange(); }
	bool  IsCastSelf() const { return stub->range.type==5; }
	bool  IsElfSkill() const { return stub->cls==258; }
	bool CanAttack(bool usearrow=true);
	char GetType() const { return stub->type; }
	int   GetCommonCoolDown() { return stub->commoncooldown; }
	int   GetCommonCoolDownTime() { return stub->commoncooldowntime; }
	int GetItemCost(){ return stub->itemcost; }
	bool IsComboSkill() { return stub->IsComboSkill(); }
	int  GetComboPreSkill() { return stub->GetComboPreSkill(); }
	int  GetComboPreInterval() { return stub->GetComboPreInterval(); }
	int  GetComboBreakType() { return stub->GetComboBreakType(); }
	int  GetComboExpire() { return SkillStub::GetComboExpire(id); }
	bool IsMovingSkill() { return stub->IsMovingSkill(); }
public:
	void SetPlayer(PlayerWrapper * p) { player = p; }
	void SetVictim(PlayerWrapper * p) { player = p; }
	void SetTarget(TargetWrapper * t) { target = t; }
	PlayerWrapper * GetPlayer() const { return player; }
	PlayerWrapper * GetVictim() const { return player; }
	TargetWrapper * GetTarget() const { return target; }

	void SetPerformerid(const XID& p) { performerid = p; }
	const XID& GetPerformerid() const { return performerid; }
	void SetPerformerpos(const A3DVECTOR& p) { performerpos = p; }
	const A3DVECTOR& GetPerformerpos() const { return performerpos; }
	char GetForceattack();

public:

	void SetPlus(float p)  { plus = p; }
	void SetRatio(float r) { ratio = r*100; }

	int GetPlus()        { return (int)plus; }
	int GetRatio()       { return (int)ratio; }

	int GetAttack();    
	int GetMagicattack();

	void SetDamage(int attack)      { damage = attack; }
	void SetGolddamage(int attack);
	void SetWooddamage(int attack);
	void SetWaterdamage(int attack);
	void SetFiredamage(int attack);
	void SetEarthdamage(int attack);
	void SetMagicDamage(int attack) { magic = attack; }
	void SetDegree(int d) { degree = d; }

	int GetDamage()      const { return damage; }
	int GetGolddamage()  const { return magicdamage[0]; }
	int GetWooddamage()  const { return magicdamage[1]; }
	int GetWaterdamage() const { return magicdamage[2]; }
	int GetFiredamage()  const { return magicdamage[3]; }
	int GetEarthdamage() const { return magicdamage[4]; }
	int GetMagicdamage() const { return magic; }
	int GetDegree() const { return degree; }

	const int* GetFivedamage() const{ return magicdamage; }

	const attacker_info_t& GetPerformerinfo() const 
	{ 
		if(attackmsg)
			return attackmsg->ainfo;
		assert(enchantmsg);
		return enchantmsg->ainfo;
	}
	char GetAttackerMode() const
	{
		if(attackmsg)
			return attackmsg->attacker_mode;
		if(enchantmsg)
			return enchantmsg->attacker_mode;
		return 0;
	}
	int GetAttackerDegree() const
	{
		if(attackmsg)
			return attackmsg->attack_degree;
		if(enchantmsg)
			return enchantmsg->attack_degree;
		return 0;
	}
	int GetAttackState() const
	{
		if(attackmsg)
			return attackmsg->_attack_state;
		if(enchantmsg)
			return enchantmsg->_attack_state;
		return 0;
	}
	int GetPenetration() const
	{
		if(attackmsg)
			return attackmsg->penetration;
		if(enchantmsg)
			return enchantmsg->penetration;
		return 0;
	}
	int GetVigour() const
	{
		if(attackmsg)
			return attackmsg->vigour;
		if(enchantmsg)
			return enchantmsg->vigour;
		return 0;
	}

	void SetPlayerlevel(int n ) { }
	int  GetPlayerlevel() const { return performerlevel; }

	void SetCharging(int w);
	unsigned int GetCharging();

	void SetSection(int s);
	unsigned int GetSection();
	
	void SetLvalue(int v);
	int  GetLvalue();

	void SetRand(int) { }
	int  GetRand() { return rand()%100; }
	int GetT0() { return t0; }
	int GetT1() { return t1; }
	int GetT2() { return t2; }
	void  SetTalent(int index, int value)
	{
		switch(index)
		{
			case 0:
				t0 = value;
				break;
			case 1:
				t1 = value;
				break;
			case 2:
				t2 = value;
				break;
		}
	}
	void PrepareTalent(PlayerWrapper* player);

public:
	int FirstRun( int & next_interval, int prayspeed );

	int Run( int & next_interval );

	int InstantRun()
	{
		const SkillStub::State * state = stub->GetState(0);
		if(state)
			Run(state);
		return 1;
	}

	bool Interrupt()
	{
		if(InvalidState())
			return false;
		const SkillStub::State * state = stub->GetState(GetStateindex());
		return state->Interrupt(this);
	}

	bool Cancel()
	{
		if(InvalidState())
			return false;
		const SkillStub::State * state = stub->GetState(GetStateindex());
		return state->Cancel(this);
	}

	void NpcFirstRun()
	{
		const SkillStub::State * state = stub->GetState(0);
		Run( state );
	}

	void NpcRun()
	{
		const SkillStub::State * state = stub->GetState(1);
		Run( state );
	}

	bool TakeEffect(PlayerWrapper* player, int arg);
	bool UndoEffect(PlayerWrapper* player, int arg);

	int GetEventFlag()
	{
		return GetStub()->GetEventFlag();
	}

	void ComboSkEndAction()
	{
		stub->ComboSkEndAction(this);
	}
protected:
	bool Run(const SkillStub::State *state)
	{
		state->Calculate( this );
		return true;
	}

	bool InvalidState();
	int NextState( int index );

};
typedef Skill* SkillPTR;
class SkillKeeper
{
	Skill* pointer;
	SkillKeeper()
	{
		pointer = NULL;
	}
public:
	SkillKeeper(Skill* p) : pointer(p)
	{
	}
	~SkillKeeper()
	{
		if(pointer)
			pointer->Destroy();
	}
	Skill* operator -> ()
	{
		return pointer;
	}
	operator SkillPTR ()
	{
		return pointer;
	}
	operator bool ()
	{
		return pointer;
	}
	SkillKeeper& operator = (Skill* p)
	{
		if(pointer)
			pointer->Destroy();
		pointer = p;
		return *this;
	}
};

};

#pragma pack()

#endif

