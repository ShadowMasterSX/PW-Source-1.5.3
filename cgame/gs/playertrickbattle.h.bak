#ifndef __ONLINEGAME_GS_PLAYER_TRICKBATTLE_H__
#define __ONLINEGAME_GS_PLAYER_TRICKBATTLE_H__

class gplayer_trickbattle : public gplayer_battle_base
{
	enum{ CHARIOT_ENERGY_MAX = 100, };
	struct SCORE
	{
		int kill;
		int death;
		int score;
	};
	
	int sync_counter;			//向world_ctrl同步积分计数
	int multi_kill;				//连杀
	bool changed;				//积分是否改变
	bool notify_client;			//是否需要通知客户端当前分数
	SCORE score_total;			//从进入战场开始的积分累计量
	SCORE score_delta;			//从上一次同步积分后的积分变化量
	int chariot_id;				//当前战车
	int chariot_lv;				//当前战车的等级
	int chariot_energy;			//当前战车能量
	int last_primary_chariot;	//上一个初级战车
	int death_couter;			//死亡状态计数

public:
	gplayer_trickbattle():sync_counter(0),multi_kill(0),changed(false),notify_client(false),chariot_id(0),chariot_lv(0),chariot_energy(0),last_primary_chariot(0),death_couter(0)
	{
		memset(&score_total, 0, sizeof(SCORE));
		memset(&score_delta, 0, sizeof(SCORE));
	}
	
public:
	DECLARE_SUBSTANCE(gplayer_trickbattle);
	virtual	void OnHeartbeat(size_t tick);
	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual void PlayerEnterServer(); 
	virtual void PlayerLeaveServer(); 
	virtual void PlayerLeaveWorld();
	virtual void OnDeath(const XID & lastattack,bool is_pariah, char attacker_mode, int taskdead);
	virtual bool CanResurrect(int param);
	virtual int  Resurrect(const A3DVECTOR & pos,bool nomove,float exp_reduce,int target_tag,float hp_factor, float mp_factor, int param);

	virtual void SyncScoreToPlane();
	virtual bool TrickBattleTransformChariot(int chariot);
	virtual bool TrickBattleUpgradeChariot(int chariot);
	virtual void TrickBattleIncChariotEnergy(int energy);

	virtual void QueryTrickBattleChariots();

};

#endif
