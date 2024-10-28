#ifndef __ONLINEGAME_GS_PLAYER_IMP_BATTLE_H__
#define __ONLINEGAME_GS_PLAYER_IMP_BATTLE_H__

#include "player_imp.h"

class gplayer_battleground: public gplayer_imp
{
	int turret_counter;
	XID turret_id;
	int attack_faction; 	//攻击附加的faction
	int defense_faction;	//受到攻击时自己附加的faction
public:
	gplayer_battleground()
	{
		attack_faction = 0;
		defense_faction = 0;
		turret_counter = 0;
		turret_id = XID(-1,-1);
	}
private:
	static bool __GetPetAttackHook(gactive_imp * __this, const MSG & msg, attack_msg & amsg);
	static bool __GetPetEnchantHook(gactive_imp * __this, const MSG & msg,enchant_msg & emsg);
	static void __GetPetAttackFill(gactive_imp * __this, attack_msg & amsg);
	static void __GetPetEnchantFill(gactive_imp * __this, enchant_msg & emsg);
	
	void SetBattleFaction();
public:
	DECLARE_SUBSTANCE(gplayer_battleground);
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
	virtual int  Resurrect(const A3DVECTOR & pos,bool nomove,float exp_reduce,int target_tag,float hp_factor, float mp_factor, int param, float ap_factor, int extra_invincible_time);
	virtual int CheckUseTurretScroll();
	virtual void TurretOutOfControl();
	virtual attack_judge GetPetAttackHook();
	virtual enchant_judge GetPetEnchantHook();
	virtual attack_fill GetPetAttackFill();
	virtual enchant_fill GetPetEnchantFill();
};


#endif

