#ifndef __ONLINEGAME_GS_OBJ_PROPERTY_H__
#define __ONLINEGAME_GS_OBJ_PROPERTY_H__ 
#include "config.h"

//对象的基本属性(很少受外界影响的属性）
struct basic_prop
{
	short level;		//级别
	short sec_level;	//修真级别
	int exp;		//经验值
	int skill_point;	//剩余的技能点
	int hp;			//当前hp
	int mp;			//当前mp
	int status_point;	//剩余的属性点
	int ap;			//当前怒气值
};


template <typename WRAPPER>
WRAPPER & operator >>(WRAPPER & wrapper, basic_prop &rhs)
{
	wrapper >> rhs.level >> rhs.exp >> rhs.skill_point;
	return wrapper;
}


template <typename WRAPPER>
WRAPPER & operator <<(WRAPPER & wrapper, basic_prop &rhs)
{
	wrapper << rhs.level << rhs.exp << rhs.skill_point;
	return wrapper;
}

//扩展属性，随时会收到外界影响的属性
struct extend_prop
{
	/* 基础属性 */
	int vitality;		//命
	int energy;		//神
	int strength;		//力
	int agility;		//敏
	int max_hp;		//最大hp
	int max_mp;		//最大mp
	int hp_gen;		//hp恢复速度
	int mp_gen;		//mp恢复速度 

	/* 运动速度*/
	float walk_speed;	//行走速度 单位  m/s
	float run_speed;	//奔跑速度 单位  m/s
	float swim_speed;	//游泳速度 单位  m/s
	float flight_speed;	//飞行速度 单位  m/s

	/* 攻击属性*/
	int attack;		//攻击率 attack rate
	int damage_low;		//最低damage
	int damage_high;	//最大物理damage
	int attack_speed;	//攻击时间间隔 以tick为单位
	float attack_range;	//攻击范围
	struct 			//物理攻击附加魔法伤害
	{
		int damage_low;
		int damage_high;
	} addon_damage[MAGIC_CLASS];		
	int damage_magic_low;		//魔法最低攻击力
	int damage_magic_high;		//魔法最高攻击力

	/* 防御属性 */	
	int resistance[MAGIC_CLASS];	//魔法抗性
	int defense;		//防御力
	int armor;		//闪躲率（装甲等级）
	int max_ap;		//最大怒气值
};

	template <typename WRAPPER>
WRAPPER & operator <<(WRAPPER & wrapper, const extend_prop &rhs)
{
	wrapper.push_back(&rhs,sizeof(rhs));
	return wrapper;
}

	template <typename WRAPPER>
WRAPPER & operator >>(WRAPPER & wrapper, extend_prop &rhs)
{
	wrapper.pop_back(&rhs,sizeof(rhs));
	return wrapper;
}

//装备产生的本体增强
struct item_prop
{
	short weapon_type;		//武器攻击类别（melee,range,missile）
	short weapon_delay;		//武器攻击点的延迟时间 单位就是一个tick
	int attack_delay;		//计算后的延迟时间，这个数据不是直接从武器上来的
	int weapon_class;		//武器的攻击子类
	int weapon_level;		//武器的级别
	int damage_low;			//武器最低攻击力
	int damage_high;		//武器最高攻击力
	int damage_magic_low;		//魔法武器最低攻击力
	int damage_magic_high;		//魔法武器最高攻击力
	int attack_speed;		//武器攻击速度修正
	float attack_range;		//武器攻击范围
	float short_range;		//武器的最小攻击距离（低于这个距离将受到攻击惩罚）
	/*
	   int attack;			//武器产生的攻击率修正
	 */
	//	int defense;			//装备产生的防御
	//	int armor;			//装备产生闪躲
};

//各种优化符所使用的数据
struct property_rune
{
	int rune_type;				// 0 是物理攻击 1 是法术攻击
	short rune_level_min;
	short rune_level_max;
	int rune_enhance;
};

//各种操作产生的非本体增强，可以为负值
struct enhanced_param
{
	/*基础属性*/
	int max_mp;			//最大mana加值
	int max_hp;			//最大生命加值
	int hp_gen;
	int mp_gen;
	int vit;
	int eng;
	int agi;
	int str;

	/*攻击属性*/
	//int weapon_damage;		//武器攻击加值	参与比例加成
	int damage_low;
	int damage_high;		//物理攻击加值
	int attack;			//攻击率
	int attack_speed;
	float attack_range;
	float walk_speed;
	float run_speed;
	float flight_speed;		//飞行
	
	int addon_damage[MAGIC_CLASS];	//物理攻击附加魔法伤害
	/*魔法属性*/
	int resistance[MAGIC_CLASS];	//魔法抗性
	int magic_dmg_low;			//魔法攻击加值
	int magic_dmg_high;			//魔法攻击加值
	//	int magic_weapon_dmg;		//魔法武器攻击加值 参与比例加成

	/*防御属性*/	
	int defense;			//防御加值(装备)
	int armor;			//闪躲加值(装备)

	float override_speed;		//覆盖速度，只对陆地有效
};

//各种操作产生的非本体增强(都是百分比)，可以为负值
struct scale_enhanced_param
{
	int max_mp;			//最大mana加值
	int max_hp;			//最大生命加值

	int hp_gen;			//hp恢复速度增加比率
	int mp_gen;			//mp恢复速度增加比率

	int walk_speed;			//走路
	int run_speed;			//跑步
	int swim_speed;			//游泳	
	int flight_speed;		//飞行
	int mount_speed;		//骑乘速度

	int attack_speed;		//攻击速度

	int damage;			//物理攻击加值
	int attack;			//攻击率

	int base_damage;		//由本体数据产生的百分比加成
	int base_magic;			//由本体数据产生的魔法攻击百分比加成

	int magic_dmg;			//魔法攻击加值
	int resistance[MAGIC_CLASS];	//魔法抗性

	int defense;			//防御加值(装备)
	int armor;			//闪躲加值(装备)
};

class world_manager;
struct team_mutable_prop
{
	short level;
	char  combat_state;
	unsigned char sec_level;
	int hp;
	int mp;
	int max_hp;
	int max_mp;
	int world_tag;
	int plane_index;
	int wallow_level;
	int profit_level;
	int force_id;
	int reincarnation_times;
	team_mutable_prop(){}
	template <typename ACTIVE_IMP>
		explicit team_mutable_prop(ACTIVE_IMP * pImp)
		{
			Init(pImp);
		}

/*	team_mutable_prop(const basic_prop & bp, const extend_prop &ep,char cs, int tag)
		:level(bp.level),combat_state(cs),sec_level(bp.sec_level),hp(bp.hp),mp(bp.mp),max_hp(ep.max_hp),max_mp(ep.max_mp)
		{}*/

	int GetWorldTag();	//{ world_manager::GetWorldTag(); }

	template <typename ACTIVE_IMP>
		void Init(ACTIVE_IMP * pImp)
		{
			const basic_prop & bp = pImp->_basic;
			const extend_prop &ep = pImp->_cur_prop;
			level = bp.level;
			combat_state = pImp->IsCombatState()?1:0;
			sec_level = bp.sec_level;
			hp = bp.hp;
			mp = bp.mp;
			max_hp = ep.max_hp;
			max_mp = ep.max_mp;
			world_tag = 1;
			plane_index = pImp->_plane->w_plane_index;
			wallow_level = pImp->_wallow_level;
			profit_level = pImp->_profit_level;
			force_id = pImp->_player_force.GetForce();
			reincarnation_times = pImp->GetReincarnationTimes(); 
		}

	bool operator == (const team_mutable_prop & rhs)
	{
		return memcmp(this,&rhs,sizeof(rhs)) == 0;
	}

	bool operator != (const team_mutable_prop & rhs)
	{
		return memcmp(this,&rhs,sizeof(rhs));
	}
};

template <typename WRAPPER>
WRAPPER & operator <<(WRAPPER & wrapper, const team_mutable_prop & rhs)
{
	wrapper.push_back(&rhs,sizeof(rhs));
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator >>(WRAPPER & wrapper, team_mutable_prop &rhs)
{
	wrapper.pop_back(&rhs,sizeof(rhs));
	return wrapper;
}

/*
struct  team_member_data
{
	int id;
	team_mutable_prop data;
	team_member_data()
	{}

	team_member_data(int member,const team_mutable_prop &prop):id(member),data(prop)
	{}
};
*/

struct plus_enhanced_param
{
	int damage;
	int magic_dmg;
	int defence;
	int resistance[MAGIC_CLASS];
	int max_hp;
};

template <typename WRAPPER>
WRAPPER & operator >>(WRAPPER & wrapper, plus_enhanced_param &rhs)
{
	wrapper.pop_back(&rhs,sizeof(rhs));
	return wrapper;
}


template <typename WRAPPER>
WRAPPER & operator <<(WRAPPER & wrapper, plus_enhanced_param &rhs)
{
	wrapper.push_back(&rhs,sizeof(rhs));
	return wrapper;
}
#endif
