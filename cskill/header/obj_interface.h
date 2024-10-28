#ifndef __ONLINEGAME_GS_OBJECT_INTERFACE_H__
#define __ONLINEGAME_GS_OBJECT_INTERFACE_H__

#include "property.h"
#include "attack.h"

class gactive_imp;
class filter;
class world;
struct pet_data;
namespace GNET { class SkillWrapper; }
namespace GDB { struct itemdata; struct shoplog; struct pet; struct petcorral; }
class object_interface
{
	gactive_imp * _imp;
public: 
	object_interface():_imp(0)
	{}

	object_interface(gactive_imp * imp):_imp(imp)
	{}

	void Attach(gactive_imp * imp)
	{
		_imp = imp;
	}
	gactive_imp * GetImpl() { return _imp;}
public:
	void BeHurt(const XID & who,const attacker_info_t & info, int damage,char attacker_mode = 0);
	void BeHurt(const XID & who,const attacker_info_t & info, int damage,bool invader, char attacker_mode = 0);
	void SendHurtMsgToSelf(const XID & attacker, int damage, bool invader, char attack_mode);
	void Heal(const XID & healer,size_t life);
	void Heal(size_t life);
	void HealBySkill(const XID & healer,size_t life);
	void HealBySkill(size_t life);
	void HealByPotion(size_t life);
	void InjectMana(int mana);
	bool DrainMana(int mana);
	void DecHP(int hp);
	void DecSkillPoint(int sp);
	bool SessionOnAttacked(int session_id);
	bool ActionOnAttacked(int action_id);

	void SetATDefenseState(char state);

	//飞行相关
	int SpendFlyTime(int tick);
	int GetFlyTime();	//取得可以使用的剩余飞行时间
	void TakeOff();	//起飞
	void Landing(); //降落
	bool IsPlayerClass();

	//玩家所在位置
	bool IsOnGround();
	bool IsInAir();
	bool IsInWater();
	bool IsMountMode();
	bool IsInSanctuary(const A3DVECTOR & pos);

	void Say(const char * msg);

	//玩家限制
	void SetNoFly(bool b);
	void SetNoChangeSelect(bool b);
	void SetNoMount(bool b);
	void SetNoBind(bool b);
	void SetNoAmulet(bool b);
	void SetNoLongJump(bool b);
	void SetNoSpeedUp(bool b);
	void SetNoInvisible(bool b);

    bool GetNoInvisible();

	void DenyAttackCmd();
	void AllowAttackCmd();
	void DenyElfSkillCmd();
	void AllowElfSkillCmd();
	void DenyUseItemCmd();
	void AllowUseItemCmd();
	void DenyNormalAttackCmd();
	void AllowNormalAttackCmd();
	void DenyPetCmd();
	void AllowPetCmd();
public:
	//filter 相关
	void AddFilter(filter*);
	void RemoveFilter(int filter_id);
	void ClearSpecFilter(int mask);
	bool IsFilterExist(int id);
	int  QueryFilter(int filterid,int index);
public:
	class MsgAdjust
	{
	public:
		virtual void AdjustAttack(attack_msg & attack) {}
	};

	int GenerateDamage();
	int GenerateMagicDamage();

	int GenerateDamage(int scale_enhance, int point_enhance);
	int GenerateMagicDamage(int scale_enhance, int point_enhance);


	int GenerateDamageWithoutRune();
	int GenerateMagicDamageWithoutRune();

	int GenerateDamageWithoutRune(int scale_enhance, int point_enhance);
	int GenerateMagicDamageWithoutRune(int scale_enhance, int point_enhance);

	int GenerateEquipDamage();
	int GenerateEquipMagicDamage();
	//攻击
	bool UseProjectile(size_t count);
	bool CanAttack(const XID & target);	//是否可以进行物理攻击
	void SetRetortState();			//设置下次攻击为反震攻击，一次有效
	bool GetRetortState();                  //测试下次攻击是否为反震，以避免错误的TranslateSendAttack
	void SetAuraAttackState();		//设置下次攻击为光环攻击，一次有效
	bool GetAuraAttackState();		//测试下次攻击为反震，以避免错误的TranslateSendAttack
	void SetReboundState();			//设置下次攻击为反弹攻击，一次有效
	void SetBeatBackState();		//设置下次攻击为反击攻击，一次有效
	void Attack(const XID & target, attack_msg & attack,int use_projectile);
	void RegionAttack1(const A3DVECTOR& pos, float radius,attack_msg & attack,int use_projectile,const XID& target);		//球
	void RegionAttack2(const A3DVECTOR& pos, float radius,attack_msg & attack,int use_projectile);		//柱
	void RegionAttack3(const A3DVECTOR& pos, float cos_half_angle,attack_msg & attack,int use_projectile);	//椎
	void MultiAttack(const XID * target,size_t size, attack_msg & attack,int use_projectile);
	void MultiAttack(const XID * target,size_t size, attack_msg & attack,MsgAdjust & adj,int use_projectile);
	void DelayAttack(const XID & target, attack_msg & attack,int use_projectile, size_t delay_tick);//仅在击中效果中使用了
	
	//技能
	void Enchant(const XID & target, enchant_msg & msg);
	void EnchantZombie(const XID & target, enchant_msg & msg);

	void TransferPetEgg(const XID & target, int pet_egg_id);

	void RegionEnchant1(const A3DVECTOR& pos, float radius,enchant_msg & msg, const XID& target);		//球
	void RegionEnchant2(const A3DVECTOR& pos, float radius,enchant_msg & msg );		//柱
	void RegionEnchant3(const A3DVECTOR& pos, float cos_half_angle,enchant_msg & msg );	//椎
	void MultiEnchant(const XID * target,size_t size, enchant_msg & msg);
	void TeamEnchant(enchant_msg & msg,bool exclude_self);
	void DelayEnchant(const XID & target, enchant_msg & msg, size_t delay_tick);//仅在击中效果中使用了

	void SetAttackMonster();
	void EnterCombatState();
	bool IsCombatState();

	void KnockBack(const XID & attacker, const A3DVECTOR &source,float distance,int time=0/*毫秒*/,int stun_time=0/*秒*/);
	void PullOver(const XID & attacker, const A3DVECTOR &source,float distance, int time);
	void KnockUp(float distance,int time=0/*毫秒*/);

	//复活
	bool Resurrect(float exp_reduce, float hp_factor, float mp_factor); //0.0 ~ 1.0
	void SendHealMsg(const XID & target, int hp);		//给某个对象加血
	void Reclaim();					//死亡后消失

	//查询 
	bool IsPet();
	bool IsDead();
	bool IsMember(const XID & member);
	bool IsInTeam();
	bool IsTeamLeader();
	bool IsPalsyImmune();
	int  GetClass();
	int  GetFaction();
	int  GetEnemyFaction();
	bool IsMafiaMember();
	int  GetMafiaID();
	char  GetMafiaRank();
	int GetSpouseID();
	int GetWorldTag();
	const A3DVECTOR & GetPos();
	const XID & GetSelfID();
	const XID & GetCurTargetID();
	const XID & GetLeaderID();
	float GetBodySize();
	int QueryObject(const XID & who, A3DVECTOR & pos, float & body_size); 	//0 不存在 1: 正常 2: 死亡
	int QueryObject(const XID & who, A3DVECTOR & pos, int & hp, int & mp); 	//0 不存在 1: 正常 2: 死亡
	int QueryObject(const XID & who, int & invisible_degree, int & anti_invisible_degree); 	//0 不存在 1: 正常 2: 死亡
	bool CheckGMPrivilege();
	bool IsPVPEnable();	//是否开启了PK开关
	char GetForceAttack();
	int GetInvaderState();	//取得红粉白名的状态
	int GetPetEggID();
	XID GetPetID();
	int GetSpherePlayerListSize(const A3DVECTOR& pos,float radius);
	float GetHpScale();
	int  GetTargetClass(const XID & target);
	int  GetTargetHp(const XID & target);
	int  GetTargetMp(const XID & target);
	int  GetTargetMaxhp(const XID & target);
	int  GetTargetLevel(const XID & target);

	//给客户端操作
	void SendClientMsgSkillCasting(const XID &target, int skill, unsigned short time,unsigned char level);
	void SendClientMsgRuneSkillCasting(const XID &target, int skill, unsigned short time,unsigned char level);
	void SendClientMsgSkillInterrupted(char reason);
	void SendClientMsgSkillPerform();
	void SendClientNotifyRoot(unsigned char type);	//广播行为
	void SendClientDispelRoot(unsigned char type);	//只发送给个人
	void SendClientCurSpeed();		//发送速度给客户端
	void SendClientEnchantResult(const XID & caster, int skill, char level, bool invader,int at_state,unsigned char section);
	void SendClientInstantSkill(const XID & target, int skill, unsigned char level);
	void SendClientRuneInstantSkill(const XID & target, int skill, unsigned char level);
	void SendClientElfSkill(const XID & target, int skill, unsigned char level);	//lgc
	void SendClientSkillAbility(int id, int ability);
	void SendClientCastPosSkill(const A3DVECTOR & pos, const XID &target, int skill,unsigned short time, unsigned char level);
	void SendClientRushMode(unsigned char is_active);
	void SendClientAttackData();
	void SendClientDefenseData();
	void SendClientMsgFlySwordTime(int time);
	void SendClientDuelStart(const XID & target);
	void SendClientDuelStop(const XID & target);
	void SendClientScreenEffect(int effectid, int state);
	void SendClientGfxEffect(int effectid, int state);
	void SendClientComboSkillPerpare(int skillid,int timestamp,int arg1, int arg2, int arg3);
	void SendClientPrayDistanceChange(float pd);
public:
	//表面状态操作
	void IncVisibleState(unsigned short state);
	void DecVisibleState(unsigned short state);
	void ClearVisibleState(unsigned short state);

	void InsertTeamVisibleState(unsigned short state);
	void RemoveTeamVisibleState(unsigned short state);
	void InsertTeamVisibleState(unsigned short state, int param);
	void InsertTeamVisibleState(unsigned short state, int param, int param2);
	void InsertTeamVisibleState(unsigned short state, int param, int param2, int param3);

	void ModifyTeamVisibleState(unsigned short state, int param);
	void ModifyTeamVisibleState(unsigned short state, int param, int param2);
	void ModifyTeamVisibleState(unsigned short state, int param, int param2, int param3);


	//设置变身标志	
	void ChangeShape(int shape);
	int GetForm();

	void LockEquipment(bool is_lock);
	void BindToGound(bool is_bind);

	//策略状态操作
	//void SetIdleMode(bool sleep,bool stun);	//让对象处于震晕或者睡眠状态，不用了
	//void SetSealMode(bool silent,bool root);//让对象处于定身或者无法攻击状态 

	//void GetIdleMode(bool & sleep, bool & stun);
	//void GetSealMode(bool & silent, bool & root);

	void IncIdleSealMode(unsigned char mode);//lgc
	void DecIdleSealMode(unsigned char mode);

	bool IsAggressive();
	void SetAggressive(bool isActive = true);

	void DuelStart(const XID & target);
	void DuelStop();
	
	//在自己的仇恨列表中添加一定仇恨
	void AddAggro(const XID & attacker , int rage);
	void AddAggroToEnemy(const XID & helper, int rage);

    void RemoveAggro(const XID& src, const XID& dest, float ratio);

	void BeTaunted(const XID & who,int aggro);

	void AddPlayerEffect(short effect);
	void RemovePlayerEffect(short effect);

	void AddMultiObjEffect(const XID& target, char type);
	void RemoveMultiObjEffect(const XID& target, char type);

	//取得免疫哪部分内容
	int GetImmuneMask();
	//void SetImmuneMask(int mask);		//为1的位是要设置免疫的属性,不用了
	//void ClearImmuneMask(int mask);		//为1的位是要清除的免疫属性
	void IncImmuneMask(int mask);
	void DecImmuneMask(int mask);

	//怒气相关
	void ModifyAP(int ap);

    void ModifyScaleHP(int hp);

	void ActiveMountState(int mount_id, unsigned short mount_color);
	void DeactiveMountState();

	void ResurrectPet();

	void Disappear();
	void Die();
public:
	//增强属性
	void EnhanceHPGen(int hpgen);		//无需调用更新函数
	void ImpairHPGen(int hpgen);		//无需调用更新函数

	void EnhanceScaleHPGen(int hpgen);
	void ImpairScaleHPGen(int hpgen);

	void EnhanceScaleMPGen(int mpgen);
	void ImpairScaleMPGen(int mpgen);

	void EnhanceMPGen(int mpgen);		//无需调用更新函数
	void ImpairMPGen(int mpgen);		//无需调用更新函数

	void EnhanceMaxHP(int hp);		//无需调用更新函数
	void ImpairMaxHP(int hp);		//无需调用更新函数

	void EnhanceMaxMP(int mp);		//无需调用更新函数
	void ImpairMaxMP(int mp);		//无需调用更新函数

	void EnhanceScaleMaxHP(int hp,bool update=true);		//无需调用更新函数
	void ImpairScaleMaxHP(int hp,bool update=true);		//无需调用更新函数

	void EnhanceScaleMaxMP(int mp);		//无需调用更新函数
	void ImpairScaleMaxMP(int mp);		//无需调用更新函数

    void EnhanceScaleExp(float exp_sp_factor, float realm_exp_factor);
    void ImpairScaleExp(float exp_sp_factor, float realm_exp_factor);

	void EnhanceDefense(int def);
	void ImpairDefense(int def);
	void EnhanceScaleDefense(int def);
	void ImpairScaleDefense(int def);

	void EnhanceArmor(int ac);
	void ImpairArmor(int ac);
	void EnhanceScaleArmor(int ac);
	void ImpairScaleArmor(int ac);

	void EnhanceResistance(size_t cls, int res);	 //cls = [0,4]
	void ImpairResistance(size_t cls, int res);

	void EnhanceScaleResistance(size_t cls, int res);	 //cls = [0,4]
	void ImpairScaleResistance(size_t cls, int res);

    void IncAntiDefenseDegree(int val);
    void DecAntiDefenseDegree(int val);

    void IncAntiResistanceDegree(int val);
    void DecAntiResistanceDegree(int val);

	void EnhanceDamage(int dmg);
	void ImpairDamage(int dmg);

	void EnhanceMagicDamage(int dmg);
	void ImpairMagicDamage(int dmg);

	void EnhanceScaleDamage(int dmg);
	void ImpairScaleDamage(int dmg);

	void EnhanceScaleMagicDamage(int dmg);
	void ImpairScaleMagicDamage(int dmg);

	void EnhanceSpeed0(float speedup);
	void ImpairSpeed0(float speedup);

	void EnhanceOverrideSpeed(float speedup);
	void ImpairOverrideSpeed(float speedup);

	void EnhanceSpeed(int speedup);
	void ImpairSpeed(int speedup);

	void EnhanceSwimSpeed(int speedup);
	void ImpairSwimSpeed(int speedup);

	void EnhanceFlySpeed(float speedup);
	void ImpairFlySpeed(float speedup);
	
	void EnhanceScaleFlySpeed(int speedup);
	void ImpairScaleFlySpeed(int speedup);

	void EnhanceMountSpeed(int speedup);
	void ImpairMountSpeed(int speedup);

	void EnhanceAttackSpeed(int speedup);
	void ImpairAttackSpeed(int speedup);
	
	void EnhanceScaleAttackSpeed(int speedup);
	void ImpairScaleAttackSpeed(int speedup);

	void EnhanceScaleAttack(int attack);
	void ImpairScaleAttack(int attack);

	void EnhanceBreathCapacity(int value);
	void ImpairBreathCapacity(int value);

	void EnhanceAttackRange(float range);
	void ImpairAttackRange(float range);

	void EnhanceAttackDegree(int value);
	void ImpairAttackDegree(int value);

	void EnhanceDefendDegree(int value);
	void ImpairDefendDegree(int value);

	void EnableEndlessBreath(bool bVal);
	void InjectBreath(int value);
	void AdjustBreathDecPoint(int offset);

	void EnhanceCrit(int val);
	void ImpairCrit(int val);
	//增加减少暴击伤害百分比
	void EnhanceCritDamage(int percent);
	void ImpairCritDamage(int percent);

	void EnhanceCritDamageReduce(int percent);
	void ImpairCritDamageReduce(int percent);

	void EnhanceCritResistance(int val);
	void ImpairCritResistance(int val);
	
	//用于被动技能提升反隐身级别，刺客专用
	void IncAntiInvisiblePassive(int val);
	void DecAntiInvisiblePassive(int val);
	//用于技能物品提升反隐身级别
	void IncAntiInvisibleActive(int val);
	void DecAntiInvisibleActive(int val);
	//用于被动技能提升隐身级别，刺客专用
	void IncInvisiblePassive(int val);
	void DecInvisiblePassive(int val);
	//增加减少伤害闪避概率
	void EnhanceDamageDodge(int val);
	void ImpairDamageDodge(int val);
	//增加减少状态闪避概率
	void EnhanceDebuffDodge(int val);
	void ImpairDebuffDodge(int val);
	//增加减少吸血百分比
	void EnhanceHpSteal(int val);
	void ImpairHpSteal(int val);
	//增加减少红符冷却时间
	void IncHealCoolTime(int ms);
	void DecHealCoolTime(int ms);
	//增加减少受伤害时产生的仇恨
	void IncAggroOnDamage(const XID & attacker, int val);
	void DecAggroOnDamage(const XID & attacker, int val);
	
	void EnhanceSkillDamage(int value);
	void ImpairSkillDamage(int value);
	void EnhanceSkillDamage2(int value);
	void ImpairSkillDamage2(int value);

	void IncApPerHit(int value);
	void DecApPerHit(int value);

	void ReduceResurrectExpLost(int value);
	void IncreaseResurrectExpLost(int value);

	void IncPenetration(int val);
	void DecPenetration(int val);
	void IncResilience(int val);
	void DecResilience(int val);

	void IncVigour(int val);
	void DecVigour(int val);

	void EnhanceNearNormalDmgReduce(float scale);
	void ImpairNearNormalDmgReduce(float scale);
	void EnhanceNearSkillDmgReduce(float scale);
	void ImpairNearSkillDmgReduce(float scale);
	void EnhanceFarNormalDmgReduce(float scale);
	void ImpairFarNormalDmgReduce(float scale);
	void EnhanceFarSkillDmgReduce(float scale);
	void ImpairFarSkillDmgReduce(float scale);

	//单人副本状态包属性操作
	void ImpairPlusDamage(int dmg);
	void EnhancePlusDamage(int dmg);
	void ImpairPlusMagicDamage(int dmg);
	void EnhancePlusMagicDamage(int dmg);
	void ImpairPlusDefense(int defence);
	void EnhancePlusDefense(int defence);
	void ImpairPlusResistance(size_t cls, int res);
	void EnhancePlusResistance(size_t cls, int res);
	void ImpairPlusMaxHP(int hp, bool update = true);
	void EnhancePlusMaxHP(int hp);


	//属性的重新计算
	void UpdateDefenseData();
	void UpdateAttackData();
	void UpdateMagicData();	//包含魔法防御
	void UpdateSpeedData();
	void UpdateHPMPGen();

	void SetInvincibleFilter(bool is_vin, int timeout,bool immune);
	void SetInvincibleFilter2(bool is_vin, int timeout,bool immune);

//冷却
	bool TestCoolDown(unsigned short id);
	void SetCoolDown(unsigned short id, int ms);
	void ResetCoolDown(unsigned short id, int ms = 1);
	void SendReduceCDMsg(const XID& target,int id, int ms);

	bool TestCommonCoolDown(unsigned short i);		//i:0-4 技能冷却0-4, 5-9物品冷却0-4
	void SetCommonCoolDown(unsigned short i, int ms);
	
	void ReturnToTown();
	bool CanReturnToTown();

	void ReturnWaypoint(int point);
	bool CheckWaypoint(int point_index, int & point_domain);

	bool SkillMove(const A3DVECTOR & pos, bool notarget);
	void BreakCurAction();

	int CalcPhysicDamage(int raw_damage, int attacker_level);
	int CalcMagicDamage(int dmg_class, int raw_damage, int attacker_level);

	float CalcLevelDamagePunish(int atk_level , int def_level);
	int CalcPenetrationEnhanceDamage(int penetration, int raw_damage);
	int CalcVigourEnhanceDamage(int atk_vigour,int def_vigour,bool pvp, int raw_damage);

	bool IsInvisible();
	void SetInvisible(int invisible_degree);	//只在filter_invisible中被调用
	void ClearInvisible();						//只在filter_invisible中被调用
	void SetInvisibleFilter(bool is_invisible, int time, int mana_per_sec,int invisible_degree,int speeddown);	//切换隐身/非隐身状态,player使用
	bool IsGMInvisible();
	void SetGMInvisible();		//只在gm_invisible_filter中被调用
	void ClearGMInvisible();	//只在gm_invisible_filter中被调用
	void SetGMInvisibleFilter(bool is_invisible, int time, int mask);	//切换gm隐身/非隐身状态,player使用

	bool ActivateSharpener(int id, int equip_index);
	void TransferSpecFilters(int filter_mask, const XID & target, int count);	
	void AbsorbSpecFilters(int filter_mask, const XID & target, int count);	
	bool SummonPet2(int pet_egg_id, int skill_level, int life_time);
	bool PetSacrifice();
	bool SummonPlantPet(int pet_egg_id, int skill_level, int life_time, const XID & target, char force_attack);
	bool PlantSuicide(float distance, const XID & target, char force_attack);
	void InjectPetHPMP(int hp, int mp);
	void DrainPetHPMP(const XID & pet_id, int hp, int mp);
	void DrainLeaderHPMP(const XID & attacker, int hp, int mp);
	void LongJump(A3DVECTOR & pos, int tag);
	void LongJumpToSpouse();
	void ShortJump(float distance,bool is_circle);
	void WeaponDisabled(bool disable);
	void DetectInvisible(float range);
	void ForceSelectTarget(const XID & target);
	void ExchangePosition(const XID & target);
	void SetSkillAttackTransmit(const XID & target);
	void ForbidBeSelected(bool b);
	void CallUpTeamMember(const XID& member);
	void QueryOtherInventory(const XID& target); 
	void TurretOutOfControl();
	void EnterNonpenaltyPVPState(bool b);
	void SetNonpenaltyPVPFilter(bool b, int time);
	void GiveMafiaPvPAward(const XID & target, int type); 
	void RequestPunish(const XID & target, int skillid, int skilllevel);
	int	 ChangeVisibleTypeId(int tid);
	bool ModifyFilter(int filterid, int ctrlname, void * ctrlval, size_t ctrllen);
	void SetInfectSkill(int skill,int level);
	void ClrInfectSkill(int skill);

	void SetSoloChallengeFilterData(int filter_id, int num);

public:
	//lgc 小精灵相关
	bool GetElfProp(short& level, short& str, short& agi, short& vit, short& eng);
	bool EnhanceElfProp(short str, short agi, short vit, short eng);
	bool ImpairElfProp(short str, short agi, short vit, short eng);
	int GetElfVigor();
	int GetElfStamina();
	bool DrainElfVigor(int dec);
	bool DrainElfStamina(int dec);

public:
	//增强召唤物
	void IncPetHp(int inc);
	void IncPetMp(int inc);
	void IncPetDamage(int inc);
	void IncPetMagicDamage(int inc);
	void IncPetDefense(int inc);
	void IncPetMagicDefense(int inc);
	void IncPetAttackDegree(int inc);
	void IncPetDefendDegree(int inc);

public:
	//物品
	int TakeOutItem(int item_id);			//去掉一个物品
	bool CheckItem(int item_id,size_t count);	//检查是否存在某种物品
	bool CheckItem(int inv_index, int item_id, size_t count); //检查制定位置是否存在某种物品
	void DropSpecItem(bool isProtected, const XID & owner);
	bool IsInventoryFull();
	size_t GetMoney();
	void DecMoney(size_t money);
	void DropMoney(size_t money, bool isProtected);
	size_t GetInventorySize();
	size_t GetEmptySlotSize();
	size_t QueryItemPrice(int inv_index,int item_id);
	size_t QueryItemPrice(int item_id);
	size_t GetMallOrdersCount();
	int GetMallOrders(GDB::shoplog * list, size_t size);

	int GetSystime();
	int GetLinkIndex();
	int GetLinkSID();
	int GetInventoryDetail(GDB::itemdata * list, size_t size);
	int GetTrashBoxDetail(GDB::itemdata * list, size_t size);
	int GetTrashBox2Detail(GDB::itemdata * list, size_t size);
	int GetTrashBox3Detail(GDB::itemdata * list, size_t size);
	int GetTrashBox4Detail(GDB::itemdata * list, size_t size);
	int GetEquipmentDetail(GDB::itemdata * list, size_t size);
	bool GetMallInfo(int & cash, int & cash_used, int &cash_delta,  int &order_id);
	bool IsCashModified();
	size_t GetEquipmentSize();
	size_t GetTrashBoxCapacity();
	size_t GetTrashBox2Capacity();
	size_t GetTrashBox3Capacity();
	size_t GetTrashBox4Capacity();
	size_t GetTrashBoxMoney();
	bool IsTrashBoxModified();
	bool IsEquipmentModified();
	int TradeLockPlayer(int get_mask,int put_mask);
	int TradeUnLockPlayer();
	int GetDBTimeStamp();
	int InceaseDBTimeStamp();
	size_t GetSkillDataSize();					//获取技能数据占用的字节数
	bool GetSkillData(void * buf, size_t size);	//将技能数据拷贝到buf所指向的内存
	size_t GetPetsCount();					//获取宠物数量
	bool GetPetsData(GDB::petcorral pets);	//将宠物数据拷贝到pets.list指向的内存

	int GetCityOwner(int city_id);
	bool TestSafeLock();


public:
	//造一个小弟
	struct minor_param
	{
		int mob_id;		//模板ID是多少
		int vis_id;		//可见id，如果为0此值无效
		int remain_time;	//0 表示永久 否则表示存留的秒数
		int policy_classid;	//不能随意填写， 很重要默认填写0
		int policy_aggro;	//不能随意填写， 很重要默认填写0
		float exp_factor;	//经验值因子
		float sp_factor;	//sp 因子
		float drop_rate;	//掉落率修正
		float money_scale;	//掉落金钱修正
		XID spec_leader_id;	//指定的leader是谁 
		bool parent_is_leader;	//调用者就是leader 此时 spec_leader_id 无效
		bool use_parent_faction;//使用调用者的阵营信息，否则使用默认数据
		//bool die_with_leader;	//leader 死亡或消失则自己也消失
		char die_with_who;//mask, 0x01是随leader死亡，0x02是随target死亡
		XID	owner_id;	//所有者，不为0的话只有该所有者能对怪物进行攻击,设置的时候优先于召唤者
		XID target_id;
		int path_id;
		unsigned char mob_name_size;	//非0则用名称
		char mob_name[18];
	};

	struct mine_param
	{
		int mine_id;		//模板ID是多少
		int remain_time;	//0 表示永久 否则表示存留的秒数
		bool bind_target;
	};

	struct npc_param
	{
		int npc_id;			
		int remain_time;
		int path_id;
	};

	//以下三个函数返回npc的id(注意是负值)，如失败返回-1
	int CreateMinors(const A3DVECTOR & pos ,const minor_param & p);//在指定位置创建小弟
	int CreateMinors(const minor_param & p,float radius = 6.0f);	//在附近随机的位置创建小弟
	int CreateMinors(const minor_param & p, const XID & target, float radius = 6.0f);	//在目标附近随机的位置创建小弟
	//以下函数成功返回0，失败返回-1
	int CreateMine(const A3DVECTOR & pos , const mine_param & p, const XID & target = XID(-1,-1));	//在制定位置创建矿物
	int CreateMine(const mine_param & p, const XID & target, float radius = 6.0f);	//在目标附近随机位置创建矿物
	int CreateMine(const A3DVECTOR & pos , const mine_param & p, const int dir, const XID & target = XID(-1,-1));	//在制定位置创建矿物
	int CreateNPC(const A3DVECTOR & pos , const npc_param & p);	//在制定位置召唤npc
	int CreateNPC(const npc_param & p, const XID & target, float radius = 6.0f);	//在目标附近随机位置召唤npc
	
	struct  pet_param
	{
		int mob_id;		//从哪一种模板里创建出来
		int vis_id;		//可见id
		float body_size;	//体型
		int exp;
		int sp;
		char use_pet_name;
		unsigned char pet_name_size;
		char pet_name[18];
	};
	
//	bool CreatePet(const A3DVECTOR & pos, const pet_param & p, XID & id);
	bool CreatePet(const A3DVECTOR & pos, char inhabit_mode, const pet_data * pData , unsigned char pet_stamp, int honor_level, XID & id, char aggro_state, char stay_state);
	bool CreatePet2(const A3DVECTOR & pos, char inhabit_mode, const pet_data * pData , unsigned char pet_stamp, int honor_level, XID & id, char aggro_state, char stay_state, int skill_level, extend_prop * pProp=NULL);
	bool CreatePet3(const A3DVECTOR & pos, char inhabit_mode, const pet_data * pData , unsigned char pet_stamp, int honor_level, XID & id, char aggro_state, char stay_state, extend_prop * pProp = NULL);

	static void CreateMob(world * pPlane,const A3DVECTOR & pos ,const minor_param & p);//创建一个怪物 非小弟
	bool IsEquipWing();

public:
	//取得基本参数
	const basic_prop & 		GetBasicProp();
	const extend_prop & 		GetExtendProp();
	const enhanced_param & 		GetEnhancedParam();
	const scale_enhanced_param & 	GetScaleEnhanecdParam();
	const item_prop &		GetCurWeapon();
	GNET::SkillWrapper &		GetSkillWrapper();
	int GetDefendDegree();
	int GetAttackDegree();
	int GetCrit();
	int GetCritDamage();
	int GetSoulPower();
	int GetInvisibleDegree();
	int GetAntiInvisibleDegree();
	int GetReputation();
	int GetSkillEnhance();
	int GetSkillEnhance2();
	int GetHistoricalMaxLevel();
	int GetVigour();
	int GetRealm();
	int GetLocalVal(int index);
};
#endif

