#ifndef __ONLINEGAME_GS_PLAYER_MNFACTION_H__
#define __ONLINEGAME_GS_PLAYER_MNFACTION_H__

#define DEBUFF_ENHANCE_INTERVAL 60
#define SYNC_POS_INTERVAL       5

class gplayer_mnfaction : public gplayer_imp
{
private:
	int _attack_faction; 	//攻击附加的faction
	int _defense_faction;	//受到攻击时自己附加的faction
	int _mnfaction_domain_id; //跨服帮战战场id
	int _expire_time;
	int _debuff_appear_time;
	float _debuff_init_ratio; 
	float _debuff_enhance_ratio_per_minute;
	int _debuff_have_appear_time;//单位为分钟,记录DEBUFF已经出现的时间
	int _debuff_tick;
	int _sync_pos_tick;
	int _delay_start_timestamp;
public:
	DECLARE_SUBSTANCE(gplayer_mnfaction);
	gplayer_mnfaction():_attack_faction(0),_defense_faction(0),_mnfaction_domain_id(0),_expire_time(0),_debuff_appear_time(0),_debuff_init_ratio(0),_debuff_enhance_ratio_per_minute(0),_debuff_have_appear_time(0),_debuff_tick(0),_sync_pos_tick(0),_delay_start_timestamp(0)
	{
		_attacker_resource_point = -1;
		_defender_resource_point = -1;
		_attend_attacker_player_count = -1;
		_attend_defender_player_count = -1;
		_attacker_killed_player_count = -1;
		_defender_killed_player_count = -1;
		_is_small_boss_appear         = false;
		_cur_degree.clear();
		_attacker_resouse_tower_state.clear();
		_defender_resouse_tower_state.clear();
		_switch_tower_state.clear();
		_transmit_pos_state.clear();
	}
	virtual void PlayerEnterWorld();  
	virtual void PlayerEnterServer(int source_tag); 
	virtual void PlayerLeaveServer(); 
	virtual void PlayerLeaveWorld();
	void SetBattleFaction();
	
	void SetMnfactionDomainID(int mnfaction_domain_id)
	{
		_mnfaction_domain_id = mnfaction_domain_id;
	}
	
	int GetMnfactionDomainID()
	{
		return _mnfaction_domain_id;
	}

	void SetExpireTime(int expire_time)
	{
		_expire_time = expire_time;
	}

	void SetDebuffInfo(int debuff_appear_time, float debuff_init_ratio, float debuff_enhance_ratio_per_minute);
	virtual int GetFaction();
	virtual int GetEnemyFaction();
	virtual void OnDeath(const XID & lastattack,bool is_pariah, char attacker_mode, int taskdead);
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual void FillAttackMsg(const XID & target,attack_msg & attack,int dec_arrow);
	virtual void FillEnchantMsg(const XID & target,enchant_msg &enchant);
	virtual attack_judge GetPetAttackHook();
	virtual enchant_judge GetPetEnchantHook();
	virtual attack_fill GetPetAttackFill();
	virtual enchant_fill GetPetEnchantFill();
	virtual void OnHeartbeat(size_t tick);

private:
	static bool __GetPetAttackHook(gactive_imp * __this, const MSG & msg, attack_msg & amsg);
	static bool __GetPetEnchantHook(gactive_imp * __this, const MSG & msg,enchant_msg & emsg);
	static void __GetPetAttackFill(gactive_imp * __this, attack_msg & amsg);
	static void __GetPetEnchantFill(gactive_imp * __this, enchant_msg & emsg);

public:
	int _attacker_resource_point;
	int _defender_resource_point;
	int _attend_attacker_player_count;
	int _attend_defender_player_count;
	int _attacker_killed_player_count;
	int _defender_killed_player_count;
	bool _is_small_boss_appear;
	abase::vector<int> _cur_degree;
	
	abase::vector<MNFactionStateInfo> _attacker_resouse_tower_state;
	abase::vector<MNFactionStateInfo> _defender_resouse_tower_state;
	abase::vector<MNFactionStateInfo> _switch_tower_state;
	abase::vector<MNFactionStateInfo> _transmit_pos_state;

	enum
	{
		SMALL_BOSS_APPEAR = 0, //小BOSS出现 喊话
		DEBUFF_APPEAR,         //DEBUFF出现 喊话
		BATTLE_GROUND_FROM_START_TIME,//距战场开始还有多长时间(分钟)
	};
};

//用于aei_filter里得到各资源状态
struct MNFactionStateInfo
{
	int _index;
	int _state;
	int _own_faction;
	int _time_out;
	MNFactionStateInfo():_index(0),_state(0),_own_faction(0),_time_out(-1){}
	MNFactionStateInfo(int index, int state, int own_faction, int time_out):_index(index),_state(state),_own_faction(own_faction),_time_out(time_out)
	{}
	MNFactionStateInfo &operator = (const MNFactionStateInfo &rhs)
	{
		_index       = rhs._index;
		_state       = rhs._state;
		_own_faction = rhs._own_faction;
		_time_out    = rhs._time_out;
		return *this;
	}
};

#endif
