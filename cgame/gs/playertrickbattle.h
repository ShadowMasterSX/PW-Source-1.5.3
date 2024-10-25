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
	
	int sync_counter;			//��world_ctrlͬ�����ּ���
	int multi_kill;				//��ɱ
	bool changed;				//�����Ƿ�ı�
	bool notify_client;			//�Ƿ���Ҫ֪ͨ�ͻ��˵�ǰ����
	SCORE score_total;			//�ӽ���ս����ʼ�Ļ����ۼ���
	SCORE score_delta;			//����һ��ͬ�����ֺ�Ļ��ֱ仯��
	int chariot_id;				//��ǰս��
	int chariot_lv;				//��ǰս���ĵȼ�
	int chariot_energy;			//��ǰս������
	int last_primary_chariot;	//��һ������ս��
	int death_couter;			//����״̬����

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
