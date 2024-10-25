#ifndef __NETGAME_GS_PLAYER_TEMPLATE_H__
#define __NETGAME_GS_PLAYER_TEMPLATE_H__

#include "property.h"
#include <common/types.h>
#include <vector.h>
#include <hashtab.h>
#include "template/city_region.h"

enum
{	
	USER_CLASS_SWORDSMAN,			//人类拿菜刀的
	USER_CLASS_MAGE,			//人类拄拐棍的
	USER_CLASS_NEC,				//影族巫师
	USER_CLASS_HAG,				//妖精
	USER_CLASS_ORGE,			//妖兽
	USER_CLASS_ASN,				//影族刺客
	USER_CLASS_ARCHER,			//羽芒
	USER_CLASS_ANGEL,			//羽灵
	USER_CLASS_BLADE,			//剑灵
	USER_CLASS_GENIE,			//魅灵
	USER_CLASS_SHADOW,			//夜影
	USER_CLASS_FAIRY,			//月仙
	USER_CLASS_COUNT
};


class itemdataman;
class player_template
{
	enum { MAX_LEVEL_DIFF = 200 ,
		BASE_LEVEL_DIFF = -100,
		MAX_REALM_LEVEL = 100,
	};
	struct class_data 
	{
		int lvl_hp;
		int lvl_mp;
		int vit_hp;
		int eng_mp;
		int agi_attack;
		int agi_armor;
		float lvl_magic;
		float lvl_dmg;
		float lvl_defense;
		float lvl_resistance;
		int crit_rate;
		int faction;
		int enemy_faction;
		int ap_per_hit;		//每次攻击产生的怒气值
		float fatering_adjust[PLAYER_FATE_RING_TOTAL];	//每个类型命轮的修正系数
	};
	struct level_adjust
	{
		float exp_adjust;
		float sp_adjust;
		float money_adjust;	//amount adjust
		float item_adjust;	//prob adjust
		float attack_adjust;	//攻击时的等级惩罚
	};
	struct team_adjust
	{	
		float exp_adjust;
		float sp_adjust;
	};
	struct team_race_adjust
	{
		float exp_adjust;
		float sp_adjust;
	};
	float _use_soul_exp_adjust[PLAYER_FATE_RING_MAX_LEVEL];
	struct realm_node
	{
		realm_node(int e,int v) : exp_goal(e),vigour_base(v) {}
		int	exp_goal;
		int vigour_base;
	};
	extend_prop _template_list[USER_CLASS_COUNT];
	class_data  _class_list[USER_CLASS_COUNT];
	level_adjust _level_adjust_table[MAX_LEVEL_DIFF+1];
	team_adjust _team_adjust[TEAM_MEMBER_CAPACITY+1];
	team_race_adjust _team_race_adjust[USER_CLASS_COUNT+1];
	float       dmg_adj_to_spec_atk_speed;		//攻击速度等于或超过4.0次/秒后攻击调整系数
	float       atk_rate_adj_to_spec_atk_speed;	//攻击速度等于或超过4.0次/秒后命中调整系数
	abase::vector<abase::pair<A3DVECTOR,rect> > _region_list;
	abase::vector<int> _exp_list;
	abase::vector<int> _pet_exp_list;
	abase::vector<float> _death_exp_punish;
	abase::vector<realm_node > _realm_list;
	abase::vector<abase::vector<float> > _generalcard_cls_adjust_list;
	abase::vector<int> _generalcard_exp_list;
	abase::vector<float> _generalcard_exp_rank_adjust_list;
	bool _debug_mode;
	int  _max_player_level;
	float _exp_bonus;
	float _sp_bonus;
	float _money_bonus;
	
	player_template();
	
	static player_template _instance;

	bool __LoadDataFromDataMan(itemdataman & dataman);
	bool __Load(const char * filename,itemdataman * pDataMan);

	bool __CopyTemplate(int cls, extend_prop & data) const;
	bool __LevelUp(int cls, int oldlvl,extend_prop & data) const;
	bool __LevelRollback(int cls, int oldlvl,extend_prop & data) const;
	bool __UpdateBasic(int cls, extend_prop & data, int vit,int eng,int str,int agi) const;
	int  __GetBasicAttackRate(int cls,int agility);
	int  __GetBasicArmor(int cls,int agility);
	int  __Rollback(int cls, extend_prop & data) const;
	int  __Rollback(int cls, extend_prop & data,int str, int agi ,int vit, int eng) const;
	int  __GetLvlupExp(int cls, int cur_lvl) const;
	int  __GetPetLvlupExp(int cur_lvl) const;
	void __InitPlayerData(int cls,gactive_imp * pImp);
	bool __GetTownPosition(const A3DVECTOR & source, A3DVECTOR & target);
	int  __GetVitHP(int cls, int vit);
	int  __GetEngMP(int cls, int eng);
	float __GetResurrectExpReduce(size_t sec_level);
	bool __CheckData(int cls, int level, const extend_prop &data);
	float __GetFateRingAdjust(int cls, int index);
	float __GetUseSoulExpAdjust(int fatering_level, int soul_level);

	int  __GetRealmLvlupExp(int realmlvl) const;
	int  __GetRealmVigour(int realmlvl) const;
	float __GetGeneralCardClsAdjust(int cls, int card_type);
	int __GetGeneralCardLvlupExp(int rank, int lvl);
		
public:
	static player_template & GetInstance()
	{
		return _instance;
	}

	static inline bool Load(const char * filename,itemdataman * pDataMan)
	{
		return GetInstance().__Load(filename,pDataMan);
	}
	static inline bool LoadRegionData(const char * filename, const char * region_file)
	{
		return city_region::InitRegionData(filename,region_file);
	}
	
	static inline void GetRegionTime(int &rtime, int &ptime)
	{
		return city_region::GetRegionTime(rtime, ptime);
	}
	

/*	static inline bool CopyTemplate(int cls, extend_prop & data)
	{
		return GetInstance().__CopyTemplate(cls, data);

	}
	*/

	static inline bool GetDebugMode()
	{
		return GetInstance()._debug_mode;
	}

	static inline void InitPlayerData(int cls,gactive_imp * pImp)	//初始化对象的一些基本数据
	{
		return GetInstance().__InitPlayerData(cls,pImp);
	}

	static inline bool LevelUp(int cls, int oldlvl,extend_prop & data)
	{
		return GetInstance().__LevelUp(cls,oldlvl,data);

	}

	static inline bool LevelRollback(int cls, int oldlvl,extend_prop & data)
	{
		return GetInstance().__LevelRollback(cls,oldlvl,data);
	}

	static inline bool CheckData(int cls, int level, const extend_prop &data)
	{
		return GetInstance().__CheckData(cls,level,data);
	}

	static inline bool UpdateBasic(int cls, extend_prop & data, int vit,int eng,int str,int agi)
	{
		return GetInstance().__UpdateBasic(cls,data,vit,eng,str,agi);
	}

	static inline int GetVitHP(int cls, int vit)
	{
		return GetInstance().__GetVitHP(cls,vit);
	}

	static inline int GetEngMP(int cls, int eng)
	{
		return GetInstance().__GetEngMP(cls,eng);
	}


	static int GetBasicAttackRate(int cls,int agility)
	{
		return GetInstance().__GetBasicAttackRate(cls,agility);
	}
	
	static int  GetBasicArmor(int cls,int agility)
	{
		return GetInstance().__GetBasicArmor(cls,agility);
	}
	
	static inline int  Rollback(int cls, extend_prop & data)
	{
		return GetInstance().__Rollback(cls,data);
	}

	static inline int  Rollback(int cls, extend_prop & data,int str, int agi ,int vit, int eng)
	{
		return GetInstance().__Rollback(cls,data,str,agi,vit,eng);
	}

	static inline int  GetLvlupExp(int cls, int cur_lvl)
	{
		return GetInstance().__GetLvlupExp(cls,cur_lvl);
	}

	static inline int  GetPetLvlupExp(int cur_lvl)
	{
		return GetInstance().__GetPetLvlupExp(cur_lvl);
	}

	static inline int GetMaxLevelLimit()
	{
		return MAX_PLAYER_LEVEL;
	}

	static inline int GetMaxRealmLevelLimit()
	{
		return MAX_REALM_LEVEL;
	}

	static inline int GetMaxLevel()
	{
		return GetInstance()._max_player_level;
	}

	static inline int GetRealmLvlupExp(int realmlvl)
	{
		return GetInstance().__GetRealmLvlupExp(realmlvl);
	}
	
	static inline int GetRealmVigour(int realmlvl)
	{
		return GetInstance().__GetRealmVigour(realmlvl);
	}

	static inline float GetGeneralCardClsAdjust(int cls, int card_type)
	{
		return GetInstance().__GetGeneralCardClsAdjust(cls, card_type);
	}

	static inline int GetGeneralCardLvlupExp(int rank, int lvl)
	{
		return  GetInstance().__GetGeneralCardLvlupExp(rank, lvl);
	}

	static inline int GetStatusPointPerLevel()
	{
		return 5;
	}

	static inline float GetFateRingAdjust(int cls, int index)
	{
		return GetInstance().__GetFateRingAdjust(cls, index);
	}

	static inline float GetUseSoulExpAdjust(int fatering_level, int soul_level)
	{
		return GetInstance().__GetUseSoulExpAdjust(fatering_level, soul_level);
	}
/*
	static inline int GetHPGenFactor(int level)
	{
		int rst;
		rst = level / 30 + 1;
		return rst;
	}

	static inline int GetMPGenFactor(int level)
	{
		int rst;
		rst = level / 50 + 1;
		return rst;
	}
*/
	static inline bool GetTownPosition(const A3DVECTOR & source, A3DVECTOR & target,int & world_tag)
	{
		//return GetInstance().__GetTownPosition(source,target);
		return city_region::GetCityPos(source.x,source.z,target,world_tag);
	}

	static inline bool IsInSanctuary(const A3DVECTOR & source)
	{
		//return GetInstance().__GetTownPosition(source,target);
		return city_region::IsInSanctuary(source.x,source.z);
	}

	static inline bool IsInPKProtectDomain(const A3DVECTOR & source)
	{
		return city_region::IsInPKProtectDomain(source.x,source.z);
	}

	static inline void SetTeamBonus(size_t team_count, size_t race_count,float * exp, float * sp)
	{
		//这个操作会在比例的基础值上进行调整 
		ASSERT(team_count < TEAM_MEMBER_CAPACITY + 1);
		ASSERT(race_count < USER_CLASS_COUNT + 1);
		const player_template & pt = GetInstance();
		*exp *= pt._team_adjust[team_count].exp_adjust + pt._team_race_adjust[race_count].exp_adjust;
		*sp *= pt._team_adjust[team_count].sp_adjust + pt._team_race_adjust[race_count].sp_adjust;
	}

	static inline void  AdjustGlobalExpSp(int &exp,int & sp)
	{
		/*
		const player_template & pt = GetInstance();
		exp = (int)(exp * (1 + pt._exp_bonus));
		sp = (int)(sp * (1 + pt._sp_bonus));
		*/
	}

	static inline void GetGlobalExpBonus(float * exp_bonus, float * sp_bonus)
	{
	/*
		const player_template & pt = GetInstance();
		*exp_bonus = pt._exp_bonus;
		*sp_bonus = pt._sp_bonus;
		*/

		*exp_bonus = 0;
		*sp_bonus = 0;
	}

	static inline void SetGlobalExpBonus(float exp_bonus, float sp_bonus)
	{
		if(exp_bonus <-1.0f || exp_bonus > 100.f) return;
		if(sp_bonus <-1.0f || sp_bonus > 100.f) return;

		player_template & pt = GetInstance();
		pt._exp_bonus = exp_bonus;
		pt._sp_bonus = sp_bonus;
	}

	static inline  void AdjustGlobalMoney(int & money)
	{
		const player_template & pt = GetInstance();
		money = (int)(money * (1 + pt._money_bonus));
	}

	static inline void SetGlobalMoneyBonus(float money_bonus)
	{
		if(money_bonus <-1.0f || money_bonus > 100.f) return;

		player_template & pt = GetInstance();
		pt._money_bonus = money_bonus;
	}
	
	static inline void GetGlobalMoneyBonus(float * money_bonus)
	{
		const player_template & pt = GetInstance();
		*money_bonus = pt._money_bonus; 
	}

	static inline float GetDamageReduce(int defense, int attacker_level)
	{
		float def = defense;
		def = def / (def + 40.f * attacker_level - 25.f);
		if(def > 0.95f) def = 0.95f;
		return def;
	}

	static inline float GetFarDamageReduceFactor(float dist,float ratio)
	{
		if(dist <= 8.f ) return 0;
		if(dist >= 40.f) dist = 40.f;
		ratio = (dist-8.f)/(dist+20.f) * ratio;
		if(ratio >= 1.f) ratio = 1.f;
		return ratio;
	}

	static inline float GetNearDamageReduceFactor(float dist,float ratio)
	{
		if(dist >= 8.f ) return 0;
		if(ratio >= 1.f) ratio = 1.f;
		return ratio;
	}

	static inline void GetAttackLevelPunishment(int plevel_minus_mlevel, float & adjust)
	{
		const player_template & pt = GetInstance();
		if(plevel_minus_mlevel < BASE_LEVEL_DIFF)
		{
			adjust = pt._level_adjust_table[0].attack_adjust;
			return;
		}
		plevel_minus_mlevel = plevel_minus_mlevel - BASE_LEVEL_DIFF;
		if(plevel_minus_mlevel >= MAX_LEVEL_DIFF)
		{
			adjust = pt._level_adjust_table[MAX_LEVEL_DIFF-1].attack_adjust;
			return;
		}
		adjust = pt._level_adjust_table[plevel_minus_mlevel].attack_adjust;
	}

	static inline float GetPenetrationEnhance(int penetration)
	{
		return 3.0f * penetration / (penetration + 300);
	}
	
	static inline float GetResilienceImpair(int resilience, int attacker_level)
	{
		return 1.f * resilience / (resilience + attacker_level);
	}
	
	static inline void GetExpPunishment(int plevel_minus_mlevel, float * exp, float * sp)
	{
		const player_template & pt = GetInstance();
		if(plevel_minus_mlevel < BASE_LEVEL_DIFF)
		{
			*exp = pt._level_adjust_table[0].exp_adjust;
			*sp = pt._level_adjust_table[0].sp_adjust;
			return;
		}
		plevel_minus_mlevel = plevel_minus_mlevel - BASE_LEVEL_DIFF;
		if(plevel_minus_mlevel >= MAX_LEVEL_DIFF)
		{
			*exp = pt._level_adjust_table[MAX_LEVEL_DIFF-1].exp_adjust;
			*sp = pt._level_adjust_table[MAX_LEVEL_DIFF-1].sp_adjust;
			return;
		}
		*exp = pt._level_adjust_table[plevel_minus_mlevel].exp_adjust;
		*sp = pt._level_adjust_table[plevel_minus_mlevel].sp_adjust;
	}

	static inline void GetDropPunishment(int plevel_minus_mlevel,float * money ,float * drop)
	{
		const player_template & pt = GetInstance();
		if(plevel_minus_mlevel < BASE_LEVEL_DIFF)
		{
			*money = pt._level_adjust_table[0].money_adjust;
			*drop = pt._level_adjust_table[0].item_adjust;
			return;
		}
		plevel_minus_mlevel = plevel_minus_mlevel - BASE_LEVEL_DIFF;
		if(plevel_minus_mlevel >= MAX_LEVEL_DIFF)
		{
			*money = pt._level_adjust_table[MAX_LEVEL_DIFF-1].money_adjust;
			*drop = pt._level_adjust_table[MAX_LEVEL_DIFF-1].item_adjust;
			return;
		}
		*money = pt._level_adjust_table[plevel_minus_mlevel].money_adjust;
		*drop = pt._level_adjust_table[plevel_minus_mlevel].item_adjust;
	}

	static  inline float GetRepairCost(int offset, int max_durability,size_t base_price)
	{
		if(max_durability > 0 && offset > 0) 
		{
			float factor = offset / (float)max_durability;
			return base_price * factor;
		}
		else
		{
			return 0;
		}
	}
	/*
	static inline float GetNormalDropMoneyRate()
	{
		return 0.0f;
	}
	
	static inline float GetInvaderDropMoneyRate()
	{
		return 0.15f;
	}

	static inline float GetPariahDropMoneyRate()
	{
		return 0.15f;
	}
*/
	static inline float * GetNormalInventoryDropRate()
	{
		//static float prop[10] = {0.99f,0.011f,0.00,0.0f,0.f,0.f,0.f};
		static float prop[10] = {1.1f,0.0f,0.00,0.0f,0.f,0.f,0.f};	//20130107修改为不再掉落
		return prop;
	}

	static inline float * GetNormalEquipmentDropRate()
	{
		static float prop[10] = {1.1,0.0f,0.0f,0.0f,0.f,0.f,0.f};
		return prop;
	}

	static inline float * GetInvaderInventoryDropRate()
	{
		static float prop[10] = {0.80f,0.21f,0.0f,0.0f,0.f,0.f,0.f};
		return prop;
	}

	static inline float * GetInvaderEquipmentDropRate()
	{
		static float prop[10] = {1.1f,0.f,0.0f,0.0f,0.f,0.f,0.f};
		return prop;
	}
	
	static inline float * GetPariahInventoryDropRate(int state)
	{
		static float prop[][10] = {{0.40,0.0,0.3,0.20,0.11,0,0},{0.20,0.0,0.40,0,0.30,0.0,0.11},{0,0,0,0.6f,0,0,0.3,0,0,0.11f}};
		return prop[state]; 
	}

	static inline float * GetPariahEquipmentDropRate(int state)
	{
		static float prop[][10] = {{0.50,0.51f,0,0,0,0,0},{0.30,0.50f,0.21f,0,0,0},{0.0,0.60,0.30,0.11,0,0,0}};
		return prop[state]; 
	}

	//被怪物杀死后的惩罚 决不会掉落已经装备的物品
	static inline float * GetMobNormalInventoryDropRate()
	{
		static float prop[10] = {1.1f,0.0f,0.0f,0.0f,0.f,0.f,0.f};
		return prop;
	}

	static inline float * GetMobInvaderInventoryDropRate()
	{
		static float prop[10] = {0.99f,0.011f,0.0f,0.0f,0.f,0.f,0.f};
		return prop;
	}

	static inline float * GetMobPariahInventoryDropRate(int state)
	{
		static float prop[][10] = {{0.99f,0.01f,0,0,0,0,0},{0.99f,0.01f,0,0,0,0,0},{0.99f,0.01f,0,0,0,0,0}};

		return prop[state]; //这个每个状态掉落的都一样
	}

	static inline int UpdatePariahState(int pariah_time)
	{
		return IncPariahState(0,pariah_time);
	}

	static inline int IncPariahState(int cur_state, int pariah_time)
	{	
		static int t[] = {PARIAH_TIME_PER_KILL*2, PARIAH_TIME_PER_KILL*10, PARIAH_TIME_PER_KILL *100};
		for(int i = cur_state ; i < (int)(sizeof(t) / sizeof(int)); i ++)
		{
			if(pariah_time < t[i]) return i;
		}
		return 0;
	}

	static inline int DecPariahState(int cur_state, int pariah_time)
	{
		return IncPariahState(0,pariah_time);
	}

	static inline float GetResurrectExpReduce(size_t sec_level)
	{
		return GetInstance().__GetResurrectExpReduce(sec_level);
	}

	static inline float GetAttackPunishment(int attacker_lvl,int defender_lvl)
	{
		int delta = defender_lvl - attacker_lvl;
		if(delta <=5) return 1.0f;
		return 1.5f/(delta - 4);
	}

	static inline float GetDmgAdjToSpecAtkSpeed()
	{
		return GetInstance().dmg_adj_to_spec_atk_speed;	
	}
	static inline float GetAtkRateAdjToSpecAtkSpeed()
	{
		return GetInstance().atk_rate_adj_to_spec_atk_speed;
	}
	/*根据收益时间计算收益级别*/
	static inline int GetProfitLevel(int time)
	{
		static int table[] = {0, 3600, 0x7fffffff};
		for(size_t i=0; i<sizeof(table)/sizeof(int); ++i)
		{
			if(time <= table[i]) return i;
		}

		return PROFIT_LEVEL_NORMAL;
	}
	/*根据收益级别计算收益比例*/
	static inline float GetProfitRate(int level)
	{
		static const float table[] = {0.f, 1.0f, 1.0f};
		ASSERT(level >= 0 && level <(int)(sizeof(table)/sizeof(float)));
		return table[level];
	}
	/*根据气魄计算伤害增强*/
	static inline float GetVigourEnhance(int atk_vigour,int def_vigour,int k)
	{
		return (1000.0f+(float)atk_vigour/k) / (1000.0f+(float)def_vigour/k);
	}
};

class property_policy
{
	static inline int Result(int base, int base2, int percent_enhance)
	{
		int rst = (int)((base + base2)*0.01f*((float)( 100 + percent_enhance)) + 0.5f);
		if(rst < 0) rst = 0;
		return rst;
	}

	static inline int Result2(int base, int percent_enhance, int offset)
	{
		int rst = (int)(base * 0.01f * (percent_enhance + 100) + 0.5f) + offset;
		if(rst < 0) rst = 0;
		return rst;
	}
public:
	static inline void UpdatePlayer(int cls,gactive_imp * pImp)
	{
		UpdatePlayerBaseProp(cls,pImp);
		UpdateSpeed(pImp);
		UpdateAttack(cls,pImp);
		UpdateMagic(pImp);
		UpdateDefense(cls,pImp);
		UpdatePlayerInvisible(pImp);
	}

	
	static inline void UpdatePlayerLimit(gactive_imp * pImp)
	{
		extend_prop & base = pImp->_base_prop;
		extend_prop & dest = pImp->_cur_prop;
		enhanced_param & en_point = pImp->_en_point;
		dest.vitality = base.vitality + en_point.vit;
		dest.strength = base.strength + en_point.str;
		dest.energy = base.energy + en_point.eng;
		dest.agility = base.agility + en_point.agi;
	}

	static inline void UpdateNPC(gactive_imp *pImp)
	{
		UpdateNPCBaseProp(pImp);
		UpdateSpeed(pImp);
		UpdateAttack(0,pImp);
		UpdateMagic(pImp);
		UpdateDefense(0,pImp);
	}

	static inline void UpdatePlayerBaseProp(int cls,gactive_imp * pImp)
	{
		extend_prop & base = pImp->_base_prop;
		extend_prop & dest = pImp->_cur_prop;
		dest = base;
		enhanced_param & en_point = pImp->_en_point;
		scale_enhanced_param & en_percent = pImp->_en_percent;
		player_template::UpdateBasic(cls,dest,en_point.vit,en_point.eng,en_point.str,en_point.agi);

		
		UpdatePlayerMPHPGen(pImp);

		dest.max_hp = Result(dest.max_hp,en_point.max_hp,en_percent.max_hp);
		dest.max_mp = Result(dest.max_mp,en_point.max_mp,en_percent.max_mp);
		pImp->SetRefreshState();
	}

	static inline void UpdatePlayerMPHPGen(gactive_imp * pImp)
	{
		extend_prop & dest = pImp->_cur_prop;
		enhanced_param & en_point = pImp->_en_point;
		scale_enhanced_param & en_percent = pImp->_en_percent;

		//设置回血
		int tmp = pImp->_base_prop.hp_gen;
		int pt_enh = (dest.vitality / PLAYER_HP_GEN_FACTOR);
		int enh = en_percent.hp_gen;
		tmp = Result2(tmp + pt_enh + en_point.hp_gen,enh,0);
		if(tmp < 0) tmp = 0;
		dest.hp_gen = tmp;

		//设置回魔
		tmp = pImp->_base_prop.mp_gen;
		enh = en_percent.mp_gen;
		pt_enh = (dest.energy / PLAYER_MP_GEN_FACTOR);
		tmp = Result2(tmp + pt_enh + en_point.mp_gen,enh,0);
		if(tmp < 0) tmp = 0;
		dest.mp_gen = tmp;
	}

	static inline void UpdateNPCBaseProp(gactive_imp * pImp)
	{
		extend_prop & base = pImp->_base_prop;
		extend_prop & dest = pImp->_cur_prop;
		/* 当前数据首先是从基础属性中来 */
		memcpy(&dest,&base,sizeof(extend_prop));
		enhanced_param & en_point = pImp->_en_point;
		scale_enhanced_param & en_percent = pImp->_en_percent;
		dest.vitality += en_point.vit;
		dest.strength += en_point.str;
		dest.energy += en_point.eng;
		dest.agility += en_point.agi;
		dest.hp_gen += en_point.hp_gen;
		dest.mp_gen += en_point.hp_gen;
		dest.max_hp = Result(dest.max_hp,en_point.max_hp,en_percent.max_hp);
		dest.max_mp = Result(dest.max_mp,en_point.max_mp,en_percent.max_mp);
		pImp->SetRefreshState();
	}

	static inline void UpdateAttack(int cls,gactive_imp * pImp)
	{
		int enh = 0;
		item_prop &cur_item  = pImp->_cur_item;
		extend_prop & base = pImp->_base_prop;
		extend_prop & dest = pImp->_cur_prop;
		enhanced_param & en_point = pImp->_en_point;
		scale_enhanced_param & en_percent = pImp->_en_percent;
		switch (cur_item.weapon_type)
		{
			case 0:	//melee
			enh = (int)(dest.strength * (100.f/150.f) + 0.5f);
			break;
			case 1:	//range
			case 2:	//melee_asn
			enh = (int)(dest.agility * (100.f/150.f) + 0.5f);
			break;
			default: 
			ASSERT(false);
		}
		en_percent.base_damage = enh;

		//首先处理物理攻击
		enh += en_percent.damage;
		dest.damage_low  = Result(cur_item.damage_low + en_point.damage_low,base.damage_low ,enh);
		dest.damage_high = Result(cur_item.damage_high + en_point.damage_high,base.damage_high,enh); 
		int base_attack = player_template::GetBasicAttackRate(cls,dest.agility);
		dest.attack = (int)((base.attack + base_attack + en_point.attack) *0.01f*(100 + en_percent.attack) +0.5f);
		if(dest.attack < 0) dest.attack = 0;
		//自身的本体攻击范围在这里也要累计，原因是npc的射程应该在这里制定
		if(cur_item.attack_range > 0.1f)
		{
			dest.attack_range = cur_item.attack_range + en_point.attack_range;
		}
		else
		{
			dest.attack_range = base.attack_range + en_point.attack_range;	
		}
		dest.attack_range += pImp->_parent->body_size;	//在自身的攻击范围内加入自身的大小

		//计算攻击速度
		int attack_speed;
		if(cur_item.weapon_class)
		{
			attack_speed = cur_item.attack_speed;
			if(attack_speed  < 4) attack_speed = 50;
			//存在武器
			attack_speed = Result(attack_speed, en_point.attack_speed,en_percent.attack_speed);
		}
		else
		{
			//空手
			attack_speed = Result(base.attack_speed, en_point.attack_speed,en_percent.attack_speed);
		}

		if(attack_speed < 4) 
		{
			attack_speed = 4;
		}
		else if (attack_speed > 300) 
		{
			attack_speed = 300;
		}
		dest.attack_speed = attack_speed;
		cur_item.attack_delay = (int)(attack_speed * 0.8f) - 1;
		
		//计算攻击暴击率
		UpdateCrit(pImp);

		for(size_t i = 0; i< MAGIC_CLASS; i ++)
		{
			dest.addon_damage[i].damage_low = base.addon_damage[i].damage_low + en_point.addon_damage[i];
			dest.addon_damage[i].damage_high = base.addon_damage[i].damage_high + en_point.addon_damage[i];
		}
	}

	static inline void UpdateCrit(gactive_imp * pImp)
	{
		const extend_prop & dest = pImp->_cur_prop;
		int base_crit_rate = (int)(dest.agility *0.05f + 0.001f);
		if(base_crit_rate + pImp->_crit_rate > 100) base_crit_rate = 100 - pImp->_crit_rate;
		pImp->_base_crit_rate = base_crit_rate;
	}

	static inline void UpdateMagic(gactive_imp * pImp)
	{
		extend_prop & base = pImp->_base_prop;
		extend_prop & dest = pImp->_cur_prop;
		item_prop & it = pImp->_cur_item;
		int enh = dest.energy;
		pImp->_en_percent.base_magic = enh;
		enh += pImp->_en_percent.magic_dmg;
		dest.damage_magic_low = Result(base.damage_magic_low + pImp->_en_point.magic_dmg_low, it.damage_magic_low, enh);
		dest.damage_magic_high = Result(base.damage_magic_high + pImp->_en_point.magic_dmg_high, it.damage_magic_high, enh);

		int res_enh = (int)((dest.vitality*2 + dest.energy*3) *(100.f/2500.f) + 0.5f);
		int res_point_enh = (dest.vitality + dest.energy) >> 2;
		for(size_t i = 0; i < MAGIC_CLASS; i ++)
		{
			int res = Result(base.resistance[i], pImp->_en_point.resistance[i], pImp->_en_percent.resistance[i] + res_enh) + res_point_enh;
			if(res <0) res = 0;
			dest.resistance[i] = res;
		}
	}

	static inline void UpdateSpeed(gactive_imp *pImp)
	{
		extend_prop & dest = pImp->_cur_prop;
		extend_prop & src = pImp->_base_prop;
		if(pImp->_en_point.override_speed > 1e-3)
		{
			//覆盖选项存在，使用之
			float sp = pImp->_en_point.override_speed;
			dest.run_speed = sp;
			dest.walk_speed = 2.5f;		//美工的动作都按照1.5的速度制作
			int enh = pImp->_en_percent.mount_speed;
			if(pImp->_en_percent.walk_speed < 0)
			{
				dest.walk_speed *= 0.01f*(pImp->_en_percent.walk_speed + enh+ 100);
			}
			else
			{
				dest.walk_speed *= 0.01f*(enh + 100);
			}

			if(pImp->_en_percent.run_speed < 0)
			{
				dest.run_speed *= 0.01f*(pImp->_en_percent.run_speed + enh + 100);
			}
			else
			{
				dest.run_speed *= 0.01f*(enh + 100);
			}
		}
		else
		{
			float en_speed1 = pImp->_en_point.walk_speed;
			float en_speed2 = pImp->_en_point.run_speed;
			//这里限制一下最大速度固定增益
			if(en_speed1 > 3.0f) 
			{
				if(en_speed1 > 5.0f) en_speed1 = 0.f; else en_speed1 = 3.f;
			}
			if(en_speed2 > 3.0f) 
			{
				if(en_speed2 > 5.0f) en_speed2 = 0.f; else en_speed2 = 3.f;
			}
			dest.walk_speed = src.walk_speed * 0.01f*(pImp->_en_percent.walk_speed + 100) + en_speed1;
			dest.run_speed = src.run_speed * 0.01f*(pImp->_en_percent.run_speed + 100) + en_speed2;
		}
		dest.swim_speed = src.swim_speed* 0.01f*(pImp->_en_percent.swim_speed + 100);
		dest.flight_speed = src.flight_speed + pImp->_en_point.flight_speed; 
		dest.flight_speed *= 0.01f*(pImp->_en_percent.flight_speed + 100);
		if(dest.flight_speed > MAX_FLIGHT_SPEED) dest.flight_speed = MAX_FLIGHT_SPEED;
		if(dest.run_speed > MAX_RUN_SPEED) dest.run_speed = MAX_RUN_SPEED;
		if(dest.walk_speed > MAX_WALK_SPEED) dest.walk_speed = MAX_WALK_SPEED;
		if(dest.run_speed <= 1e-3 ) dest.run_speed = MIN_RUN_SPEED;
		if(dest.walk_speed <= 1e-3 ) dest.walk_speed = MIN_WALK_SPEED;
	}

	static inline void UpdateDefense(int cls,gactive_imp *pImp)
	{
		extend_prop & base = pImp->_base_prop;
		extend_prop & dest = pImp->_cur_prop;
		int enh = (int)((dest.vitality*2 + dest.strength*3) *(100.f/2500.f) + 0.5f);
		int point_enh = (dest.vitality + dest.strength) >> 2;
		dest.defense = Result2(base.defense + pImp->_en_point.defense,pImp->_en_percent.defense + enh, point_enh);
		if(dest.defense < 0) dest.defense = 0;
		int base_armor = player_template::GetBasicArmor(cls,dest.agility);
		dest.armor = Result(base_armor,pImp->_en_point.armor,pImp->_en_percent.armor);
		if(dest.armor < 0) dest.armor = 0;
	}

	static inline void UpdatePlayerInvisible(gactive_imp *pImp)
	{
		gactive_object* player = (gactive_object *)pImp->_parent;	
		player->anti_invisible_degree = ANTI_INVISIBLE_CONSTANT + pImp->GetHistoricalMaxLevel();
		if(pImp->_anti_invisible_passive > pImp->_anti_invisible_active)
			player->anti_invisible_degree += pImp->_anti_invisible_passive;
		else
			player->anti_invisible_degree += pImp->_anti_invisible_active;
	
		if(pImp->_invisible_active > 0)
			player->invisible_degree = pImp->GetHistoricalMaxLevel() + pImp->_invisible_active + pImp->_invisible_passive;
		else
			player->invisible_degree = 0;
	}

	/*
	*	部分update
	*/
	static inline void UpdateLife(gactive_imp * pImp)
	{
		extend_prop & dest = pImp->_cur_prop;
		int xp = pImp->_en_point.vit;
		if(xp)
		{
			xp = player_template::GetVitHP(pImp->GetObjectClass(),xp);
		}
		
		dest.max_hp = Result(pImp->_base_prop.max_hp + xp,pImp->_en_point.max_hp,pImp->_en_percent.max_hp);
		if(dest.max_hp < 1) dest.max_hp = 1;
		if(pImp->_basic.hp > dest.max_hp)
		{
			pImp->_basic.hp = dest.max_hp;
		}
	}

	static inline void UpdateMana(gactive_imp * pImp)
	{
		int xp = pImp->_en_point.eng;
		if(xp)
		{
			xp = player_template::GetEngMP(pImp->GetObjectClass(),xp);
		}
		extend_prop & dest = pImp->_cur_prop;
		dest.max_mp = Result(pImp->_base_prop.max_mp + xp,pImp->_en_point.max_mp,pImp->_en_percent.max_mp);
		if(pImp->_basic.mp > dest.max_mp)
		{
			pImp->_basic.mp = dest.max_mp;
		}
	}

};

#endif

