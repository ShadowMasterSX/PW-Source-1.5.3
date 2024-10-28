#ifndef __ONLINEGAME_GS_PLAYER_COUNTRYBATTLE_H__
#define __ONLINEGAME_GS_PLAYER_COUNTRYBATTLE_H__

//国战战场玩家实现,攻击判定与gplayer_battleground基本一致

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

	int attack_faction; 	//攻击附加的faction
	int defense_faction;	//受到攻击时自己附加的faction
	int sync_counter;		//同步计数
	int combat_time;		//战斗时间
	int attend_time;		//参与时间
	int damage_output;		//对玩家输出伤害
	int damage_outout_weighted;	//对玩家输出伤害加权值,权重系数为目标魂力*0.0001
	int damage_endure;		//承受的伤害
	int damage_output_npc;	//对npc输出伤害
	int resurrect_rest_times;	//复活剩余次数
	bool flag_carrier;		//抗旗手
	int flag_timeout;		//扛旗超时
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

