#ifndef __ONLINEGAME_GS_MERIDIANMANAGER_H__
#define __ONLINEGAME_GS_MERIDIANMANAGER_H__

#include <common/base_wrapper.h>
#include <db_if.h>

class gplayer_imp;
class meridian_manager
{
public:
	enum
	{
		MERIDIAN_FATAL_ERR,
		MERIDIAN_LIFE_NOTREFINE,
		MERIDIAN_LIFE_REFINE,
		MERIDIAN_DEATH_NOFAIL,
		MERIDIAN_DEATH_FAIL,
	};
private:
	
	static int MERIDIAN_COST_LIST[80][3];	
	
	int _meridian_level;
	int _lifegate_times;
	int _deathgate_times;
	int	_free_refine_times;  
	int _paid_refine_times;
	int _player_login_time; //�����0��
	int _continu_login_days; //������������
	int _trigrams_map[3];

public:
	meridian_manager():_meridian_level(0),_lifegate_times(0),_deathgate_times(0),_free_refine_times(0),_paid_refine_times(0),_player_login_time(0),_continu_login_days(0)
	{
		memset(_trigrams_map,0,sizeof(int)*3);
	}

	~meridian_manager(){}
	
	//���Գ�������ӿ�
	int TryRefineMeridian(gplayer_imp *pImp,int index);

	void Save(archive & ar);
	void Load(archive & ar);
	void Swap(meridian_manager & rhs);
	//���������	
	void Activate(gplayer_imp *pImp);
	//ȥ����������
	void Deactivate(gplayer_imp *pImp);
	
	void Heartbeat(gplayer_imp *pImp,int cur_time);
	
	void MakeDBData(GDB::meridian_data &meridian);
	void InitFromDBData(const GDB::meridian_data &meridian);
	
	//֪ͨ�ͻ��˾�������
	void NotifyMeridianData(gplayer_imp *pImp);
	//����Ƿ���ϳ��������Ҫ��
	bool CheckMeridianCondition(int plevel);
	//����Ƿ�����Ѵ���
	bool CheckMeridianFreeRefineTimes();
	//������Ѵ�����������
	void AddFreeRefineTime(int num);
	//��Ч�������ڵ�½ʱ����
	void InitEnhance(gplayer_imp *pImp);	
private:
	//���ݵ�ǰʱ��ˢ����������ʱ�������
	void RefreshRefineTimes(gplayer_imp *pImp,int cur_time);
};

#endif
