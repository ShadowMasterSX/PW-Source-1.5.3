#ifndef __NETGAME_GS_PET_DATA_MAN_H__
#define __NETGAME_GS_PET_DATA_MAN_H__

#include <hashtab.h>
#include <timer.h>
#include <threadpool.h>
#include <arandomgen.h>
#include <common/types.h>
#include <glog.h>
#include "petman.h"
#include "playertemplate.h"
#include "property.h"

class itemdataman;

//宠物数据模板
struct pet_data_temp
{
	int tid;
	int pet_class; //宠物种类 在pet_data中定义
	//其他参数

	inline int CalcHP(int level) const
	{
		return (int)(hp_a * (level - hp_b*level_require + hp_c));
	}

	inline int CalcHPGEN(int level) const
	{
		return (int)(hp_gen_a *(level - hp_gen_b*level_require + hp_gen_c));
	}

	inline int CalcDamage(int level) const
	{
		return (int)(damage_a * (damage_b * level*level + damage_c * level + damage_d));
	}

	inline int CalcDefense(int level) const
	{
		return (int)(defense_a * (defense_b*(level - defense_c*level_require) + defense_d));

	}

	inline int CalcAttack(int level) const
	{
		return (int)(attack_a * (level - attack_b*level_require + attack_c));
	}

	inline int CalcArmor(int level) const
	{
		return (int)(armor_a * (level - armor_b*level_require + armor_c));
	}

	inline int CalcResistance(int level) const
	{
		return (int)(resistance_a * (resistance_b*(level - resistance_c*level_require) + resistance_d));
	}
	
	inline int CalcMP(int level) const
	{
		return (int)(mp_a * (level + 5));
	}

	inline int CalcMPGEN(int level) const
	{
		return (int)(mp_gen_a *(level + 5));
	}
	
	inline int CalcAttackDegree(int level) const
	{
		return (int)(attack_degree_a * (level + 5));
	}
	inline int CalcDefendDegree(int level) const
	{
		return (int)(defend_degree_a * (level + 5));
	}

	float hp_a;
	float hp_b;
	float hp_c;			//hp系数  hp = hp_a * (level - hp_b*level_require + hp_c);
	
	float hp_gen_a;
	float hp_gen_b;
	float hp_gen_c;			//hpgen系数 hp_gen = hp_gen_a *(level - hp_gen_b*level_require + hp_gen_c);

	float damage_a;
	float damage_b;
	float damage_c;			
	float damage_d;			//攻击力系数 damage = damage_a * (damage_b * level*level + damage_c * level + damage_d);
	float speed_a;
	float speed_b;			//速度函数 speed = speed_a + speed_b*(level - 1);

	float attack_a;
	float attack_b;
	float attack_c;			//命中系数  attack = attack_a * (level - attack_b*level_require + attack_c);
	
	float armor_a;
	float armor_b;
	float armor_c;			//闪躲系数  armor = armor_a * (level - armor_b*level_require + armor_c);

	float defense_a;
	float defense_b;
	float defense_c;
	float defense_d;			//防御系数  defense = defense_a * (defense_b*(level - defense_c*level_require) + defense_d); 

	float resistance_a;
	float resistance_b;
	float resistance_c;
	float resistance_d;		//魔防系数  resistance = resistance_a * (resistance_b*(level - resistance_c*level_require) + resistance_d); 

	float mp_a;				//mp系数
	
	float mp_gen_a;			//mp回复系数
	
	float attack_degree_a;	//攻击等级系数

	float defend_degree_a;	//防御等级系数

	float 	body_size;		//宠物大小
	float 	attack_range;		//宠物攻击距离
	int	damage_delay;		//攻击延迟
	int	attack_speed;		//攻击速度
	float	sight_range;		//视野范围 

	unsigned int food_mask;		//食物种类
	unsigned int inhabit_type;	//栖息地
	unsigned int inhabit_mode;	//栖息地 给内部使用

	int immune_type; 
	int sacrifice_skill;	//主人牺牲宠物后，主人可获得此物品技能的效果。

	int	max_level;		//最大宠物级别
	int 	level_require;

	int plant_group;		//植物所属组
	int group_limit; 		//当前组内最多允许召唤数量
	
	int evolution_id; 		//进化后的宠物蛋模板id，为0则不可以进化
	int max_r_attack;
	int max_r_defense;
	int max_r_hp;
	int max_r_atk_lvl;
	int max_r_def_lvl;
	
	int specific_skill_id;
};

class pet_dataman
{
	typedef abase::hashtab<pet_data_temp , int ,abase::_hash_function> MAP;
	MAP _pt_map;
	
	bool __InsertTemplate(const pet_data_temp & pt)
	{       
		return _pt_map.put(pt.tid, pt);
	}       

	const pet_data_temp * __GetTemplate(int tid)
	{       
		return _pt_map.nGet(tid);
	}

	static pet_dataman & __GetInstance()
	{
		static pet_dataman __Singleton;
		return __Singleton; 
	}
	pet_dataman():_pt_map(1024) {}
	size_t __GetLvlupExp(size_t cur_lvl) const;
public:
	
	static bool Insert(const pet_data_temp & pt)
	{
		bool rst = __GetInstance().__InsertTemplate(pt);
		ASSERT(rst);
		return rst;
	}

	static const pet_data_temp * Get(int tid)
	{
		return __GetInstance().__GetTemplate(tid);
	}

	static bool LoadTemplate(itemdataman & dataman);

	static bool CalcMountParam(int tid, int lvl , float & speedup)
	{
		const pet_data_temp * pTmp = Get(tid);
		if(!pTmp) return false;
		speedup = pTmp->speed_a + pTmp->speed_b * (lvl - 1);
		//speedup = pTmp->speed_b + pTmp->speed_a * lvl;

		return true;
	}
	static bool GenerateBaseProp(int tid, int level, extend_prop & prop);
	static bool GenerateBaseProp2(int tid, int level, int skill_level, extend_prop & prop, int& attack_degree, int& defend_degree);

	static size_t GetLvlupExp(size_t cur_lvl)
	{
		return player_template::GetPetLvlupExp(cur_lvl);
	}

	static int GetMaxLevel(int tid)
	{
		const pet_data_temp * pTmp = Get(tid);
		if(!pTmp) return 0;
		return pTmp->max_level;
	}
};

#endif

