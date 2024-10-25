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
	int _player_login_time; //当天的0点
	int _continu_login_days; //连续上线天数
	int _trigrams_map[3];

public:
	meridian_manager():_meridian_level(0),_lifegate_times(0),_deathgate_times(0),_free_refine_times(0),_paid_refine_times(0),_player_login_time(0),_continu_login_days(0)
	{
		memset(_trigrams_map,0,sizeof(int)*3);
	}

	~meridian_manager(){}
	
	//尝试冲击经脉接口
	int TryRefineMeridian(gplayer_imp *pImp,int index);

	void Save(archive & ar);
	void Load(archive & ar);
	void Swap(meridian_manager & rhs);
	//激活经脉属性	
	void Activate(gplayer_imp *pImp);
	//去除经脉属性
	void Deactivate(gplayer_imp *pImp);
	
	void Heartbeat(gplayer_imp *pImp,int cur_time);
	
	void MakeDBData(GDB::meridian_data &meridian);
	void InitFromDBData(const GDB::meridian_data &meridian);
	
	//通知客户端经脉数据
	void NotifyMeridianData(gplayer_imp *pImp);
	//检查是否符合冲击经脉的要求
	bool CheckMeridianCondition(int plevel);
	//检查是否还有免费次数
	bool CheckMeridianFreeRefineTimes();
	//增加免费次数，调试用
	void AddFreeRefineTime(int num);
	//生效函数，在登陆时调用
	void InitEnhance(gplayer_imp *pImp);	
private:
	//根据当前时间刷新连续在线时间等数据
	void RefreshRefineTimes(gplayer_imp *pImp,int cur_time);
};

#endif
