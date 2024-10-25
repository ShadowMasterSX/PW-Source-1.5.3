#ifndef __ONLINEGAME_GS_OBJ_PROPERTY_H__
#define __ONLINEGAME_GS_OBJ_PROPERTY_H__ 
#include "config.h"
#include <string.h>

//����Ļ�������(���������Ӱ������ԣ�
struct basic_prop
{
	short level;		//����
	short sec_level;	//���漶��
	int exp;		//����ֵ
	int skill_point;	//ʣ��ļ��ܵ�
	int hp;			//��ǰhp
	int mp;			//��ǰmp
	int status_point;	//ʣ������Ե�
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

//��չ���ԣ���ʱ���յ����Ӱ�������
struct extend_prop
{
	/* �������� */
	int vitality;		//��
	int energy;		//��
	int strength;		//��
	int agility;		//��
	int max_hp;		//���hp
	int max_mp;		//���mp
	int hp_gen;		//hp�ָ��ٶ�
	int mp_gen;		//mp�ָ��ٶ� 

	/* �˶��ٶ�*/
	float walk_speed;	//�����ٶ� ��λ  m/s
	float run_speed;	//�����ٶ� ��λ  m/s
	float swim_speed;	//��Ӿ�ٶ� ��λ  m/s
	float flight_speed;	//�����ٶ� ��λ  m/s

	/* ��������*/
	int attack;		//������ attack rate
	int damage_low;		//���damage
	int damage_high;	//�������damage
	int attack_speed;	//����ʱ���� ��tickΪ��λ
	float attack_range;	//������Χ
	struct 			//����������ħ���˺�
	{
		int damage_low;
		int damage_high;
	} addon_damage[MAGIC_CLASS];		
	int damage_magic_low;		//ħ����͹�����
	int damage_magic_high;		//ħ����߹�����

	/* �������� */	
	int resistance[MAGIC_CLASS];	//ħ������
	int defense;		//������
	int armor;		//�����ʣ�װ�׵ȼ���
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

//װ�������ı�����ǿ
struct item_prop
{
	short weapon_type;		//�����������melee,range,missile��
	short weapon_delay;		//������������ӳ�ʱ�� ��λ����һ��tick
	int weapon_class;		//�����Ĺ�������
	int weapon_level;		//�����ļ���
	int damage_low;			//������͹�����
	int damage_high;		//������߹�����
	int damage_magic_low;		//ħ��������͹�����
	int damage_magic_high;		//ħ��������߹�����
	int attack_speed;		//���������ٶ�����
	float attack_range;		//����������Χ
	float short_range;		//��������С�������루����������뽫�ܵ������ͷ���
	/*
	   int attack;			//���������Ĺ���������
	 */
	//	int defense;			//װ�������ķ���
	//	int armor;			//װ����������
};

//�����Ż�����ʹ�õ�����
struct property_rune
{
	int physic_rune_level;
	float physic_rune_enhance;
	
	int magic_rune_level;
	float magic_rune_enhance;
};

//���ֲ��������ķǱ�����ǿ������Ϊ��ֵ
struct enhanced_param
{
	/*��������*/
	int max_mp;			//���mana��ֵ
	int max_hp;			//���������ֵ
	int hp_gen;
	int mp_gen;
	int vit;
	int eng;
	int agi;
	int str;

	/*��������*/
	//int weapon_damage;		//����������ֵ	��������ӳ�
	int damage_low;
	int damage_high;		//��������ֵ
	int attack;			//������
	int attack_speed;
	float attack_range;
	float flight_speed;		//����
	struct 
	{
		int damage_low;
		int damage_high;
	} addon_damage[MAGIC_CLASS];	//����������ħ���˺�
	/*ħ������*/
	int resistance[MAGIC_CLASS];	//ħ������
	int magic_dmg_low;			//ħ��������ֵ
	int magic_dmg_high;			//ħ��������ֵ
	//	int magic_weapon_dmg;		//ħ������������ֵ ��������ӳ�

	/*��������*/	
	int defense;			//������ֵ(װ��)
	int armor;			//�����ֵ(װ��)
};

//���ֲ��������ķǱ�����ǿ(���ǰٷֱ�)������Ϊ��ֵ
struct scale_enhanced_param
{
	int max_mp;			//���mana��ֵ
	int max_hp;			//���������ֵ

	int hp_gen;			//hp�ָ��ٶ����ӱ���
	int mp_gen;			//mp�ָ��ٶ����ӱ���

	int walk_speed;			//��·
	int run_speed;			//�ܲ�
	int swim_speed;			//��Ӿ	
	int flight_speed;		//����

	int damage;			//��������ֵ
	int attack;			//������

	int magic_dmg;			//ħ��������ֵ

	int defense;			//������ֵ(װ��)
	int armor;			//�����ֵ(װ��)
};

struct team_mutable_prop
{
	short level;
	short sec_level;
	int hp;
	int mp;
	int max_hp;
	int max_mp;
	team_mutable_prop(){}
	template <typename ACTIVE_IMP>
		explicit team_mutable_prop(ACTIVE_IMP * pImp)
		{
			const basic_prop & bp = pImp->_basic;
			const extend_prop &ep = pImp->_cur_prop;
			level = bp.level;
			sec_level = bp.sec_level;
			hp = bp.hp;
			mp = bp.mp;
			max_hp = ep.max_hp;
			max_mp = ep.max_mp;
		}

	team_mutable_prop(const basic_prop & bp, const extend_prop &ep)
		:level(bp.level),hp(bp.hp),mp(bp.mp),max_hp(ep.max_hp),max_mp(ep.max_mp)
		{}

	template <typename ACTIVE_IMP>
		void Init(ACTIVE_IMP * pImp)
		{
			const basic_prop & bp = pImp->_basic;
			const extend_prop &ep = pImp->_cur_prop;
			level = bp.level;
			sec_level = bp.sec_level;
			hp = bp.hp;
			mp = bp.mp;
			max_hp = ep.max_hp;
			max_mp = ep.max_mp;
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
#endif
