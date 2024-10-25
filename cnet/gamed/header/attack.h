#ifndef __ONLINEGAME_GS_ATTACK_H__
#define __ONLINEGAME_GS_ATTACK_H__


#include "config.h"

struct attacker_info_t
{
	int level;			//�����߼���
	int team_id;			//��������id
	int team_seq;			//������������
	int sid;			//�����player����ʾ��player��cs_index ��Ӧ��sid cs_index �� msg.param
};

struct attack_msg
{
	attacker_info_t ainfo;		//�����ߵ���Ϣ
//	int level;			//�����߼���
//	int team_id;			//��������id
//	int team_seq;			//������������
//	int sid;			//�����player����ʾ��player��cs_index ��Ӧ��sid cs_index �� msg.param

	float attack_range;		//�˴ι����ķ�Χ������������Ϣ���棩
	float short_range;		//�˴ι�������С��Χ  �������������Χ�򹥻������� ħ����������
	int physic_damage;		//���������˺���
	int attack_rate;		//������������
	int magic_damage[MAGIC_CLASS];	//ħ���˺���
	int attacker_faction;		//��������Ӫ
	int target_faction;		//�����ߵĵ�����Ӫ(�Լ�����Ӫֻ�з��������Ӫ���ܱ���ǿ�ƹ����˺�)
	char physic_attack;		//�Ƿ�������
	char force_attack;		//�Ƿ�ǿ�ƹ���
	char attacker_layer;		//�����ߴ���ʲôλ�� 0 ���� 1 ���� 2 ˮ�� 
	char attack_state;		//0x01 �ػ�  0x02 �����Ż���
	int speed;
	struct
	{
		int skill;
		int level;
	} attached_skill;
};

struct enchant_msg
{
	attacker_info_t ainfo;		//�����ߵ���Ϣ
/*
	int level;			//�����߼���
	int team_id;			//��������id
	int team_seq;			//������������
	int sid;			//�����player����ʾ��player��cs_index ��Ӧ��sid cs_index �� msg.param
	*/

	int attacker_faction;		//��������Ӫ
	int target_faction;		//�����ߵĵ�����Ӫ(�Լ�����Ӫֻ�з��������Ӫ���ܱ���ǿ�ƹ����˺�)
	float attack_range;
	int skill;
	int skill_reserved1;		//�����ڲ�ʹ��
	int invader_time;		//���ӷ���ʱ���
	char force_attack;		//�Ƿ�ǿ�ƹ���
	char skill_level;
	char attacker_layer;
	char helpful;			//�Ƿ����ⷨ��
	//char attack_state;
};

struct damage_entry
{
	float physic_damage;
	float magic_damage[MAGIC_CLASS];
};

#endif

