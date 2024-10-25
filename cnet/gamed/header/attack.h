#ifndef __ONLINEGAME_GS_ATTACK_H__
#define __ONLINEGAME_GS_ATTACK_H__


#include "config.h"

struct attacker_info_t
{
	int level;			//攻击者级别
	int team_id;			//攻击者组id
	int team_seq;			//攻击者组的序号
	int sid;			//如果是player，表示了player的cs_index 对应的sid cs_index 是 msg.param
};

struct attack_msg
{
	attacker_info_t ainfo;		//攻击者的信息
//	int level;			//攻击者级别
//	int team_id;			//攻击者组id
//	int team_seq;			//攻击者组的序号
//	int sid;			//如果是player，表示了player的cs_index 对应的sid cs_index 是 msg.param

	float attack_range;		//此次攻击的范围（攻击点在消息里面）
	float short_range;		//此次攻击的最小范围  超出这个攻击范围则攻击力减半 魔法攻击不算
	int physic_damage;		//物理攻击的伤害力
	int attack_rate;		//物理攻击的命中
	int magic_damage[MAGIC_CLASS];	//魔法伤害力
	int attacker_faction;		//攻击者阵营
	int target_faction;		//攻击者的敌人阵营(自己的阵营只有符合这个阵营才能被非强制攻击伤害)
	char physic_attack;		//是否物理攻击
	char force_attack;		//是否强制攻击
	char attacker_layer;		//攻击者处于什么位置 0 地上 1 天上 2 水上 
	char attack_state;		//0x01 重击  0x02 攻击优化符
	int speed;
	struct
	{
		int skill;
		int level;
	} attached_skill;
};

struct enchant_msg
{
	attacker_info_t ainfo;		//攻击者的信息
/*
	int level;			//攻击者级别
	int team_id;			//攻击者组id
	int team_seq;			//攻击者组的序号
	int sid;			//如果是player，表示了player的cs_index 对应的sid cs_index 是 msg.param
	*/

	int attacker_faction;		//攻击者阵营
	int target_faction;		//攻击者的敌人阵营(自己的阵营只有符合这个阵营才能被非强制攻击伤害)
	float attack_range;
	int skill;
	int skill_reserved1;		//技能内部使用
	int invader_time;		//增加粉名时间表
	char force_attack;		//是否强制攻击
	char skill_level;
	char attacker_layer;
	char helpful;			//是否善意法术
	//char attack_state;
};

struct damage_entry
{
	float physic_damage;
	float magic_damage[MAGIC_CLASS];
};

#endif

