#ifndef __ONLINEGAME_GS_PLAYER_COUNTRYBATTLE_H__
#define __ONLINEGAME_GS_PLAYER_COUNTRYBATTLE_H__

//��սս�����ʵ��,�����ж���gplayer_battleground����һ��

class gplayer_countrybattle: public gplayer_imp
{
	enum
	{
		INIT_RESURRECT_TIMES = 5,
		FLAG_TIMEOUT = 480,
	};
	
	struct damage_endure_bucket
	{
		int output_weighted_ceil;
		int endure_ceil;
		int output_weighted;
		int endure;
	};

	int attack_faction; 	//�������ӵ�faction
	int defense_faction;	//�ܵ�����ʱ�Լ����ӵ�faction
	int sync_counter;		//ͬ������
	int combat_time;		//ս��ʱ��
	int attend_time;		//����ʱ��
	int damage_output;		//���������˺�
	int damage_outout_weighted;	//���������˺���Ȩֵ,Ȩ��ϵ��ΪĿ�����*0.0001
	int damage_endure;		//���ܵ��˺�
	int damage_output_npc;	//��npc����˺�
	int resurrect_rest_times;	//����ʣ�����
	bool flag_carrier;		//������
	int flag_timeout;		//���쳬ʱ
	damage_endure_bucket bucket;
		
public:
	gplayer_countrybattle()
	{
		attack_faction = 0;
		defense_faction = 0;
		sync_counter = 0;
		combat_time = 0;
		attend_time = 0;
		damage_output = 0;
		damage_outout_weighted = 0;
		damage_endure = 0;
		damage_output_npc = 0;
		resurrect_rest_times = INIT_RESURRECT_TIMES;
		flag_carrier = false;
		flag_timeout = 0;	
		memset(&bucket, 0, sizeof(bucket));
	}
private:
	static bool __GetPetAttackHook(gactive_imp * __this, const MSG & msg, attack_msg & amsg);
	static bool __GetPetEnchantHook(gactive_imp * __this, const MSG & msg,enchant_msg & emsg);
	static void __GetPetAttackFill(gactive_imp * __this, attack_msg & amsg);
	static void __GetPetEnchantFill(gactive_imp * __this, enchant_msg & emsg);
	
	void SetBattleFaction();
public:
	DECLARE_SUBSTANCE(gplayer_countrybattle);
	virtual	void OnHeartbeat(size_t tick);
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual int ZombieMessageHandler(world * pPlane ,const MSG & msg);
	virtual void FillAttackMsg(const XID & target,attack_msg & attack,int dec_arrow);
	virtual void FillEnchantMsg(const XID & target,enchant_msg & enchant);
	virtual void PlayerEnterWorld();  
	virtual void PlayerEnterServer(int source_tag); 
	virtual void PlayerLeaveServer(); 
	virtual void PlayerLeaveWorld();
	virtual int GetFaction();
	virtual int GetEnemyFaction();
	virtual attack_judge GetPetAttackHook();
	virtual enchant_judge GetPetEnchantHook();
	virtual attack_fill GetPetAttackFill();
	virtual enchant_fill GetPetEnchantFill();
	virtual void OnDamage(const XID & attacker,int skill_id,const attacker_info_t&info,int damage,int at_state,char speed,bool orange,unsigned char section);
	virtual void OnHurt(const XID & attacker,const attacker_info_t&info,int damage,bool invader);
	virtual void OnDeath(const XID & lastattack,bool is_pariah, char attacker_mode, int taskdead);
	virtual bool CanResurrect(int param);
	virtual int  Resurrect(const A3DVECTOR & pos,bool nomove,float exp_reduce,int target_tag,float hp_factor, float mp_factor, int param, float ap_factor, int extra_invincible_time);
	virtual void SyncScoreToPlane();
	virtual void SetFlagCarrier(bool b);

public:
};


#endif

