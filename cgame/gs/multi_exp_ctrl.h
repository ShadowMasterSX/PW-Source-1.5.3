#ifndef __ONLINEGAME_GS_MULTI_EXP_CTRL_H__
#define __ONLINEGAME_GS_MULTI_EXP_CTRL_H__

#define MAX_DEACTIVATE_TIMES_PER_DAY 10

class gactive_imp;
class multi_exp_ctrl
{
	char state;
	int timeout;

	int last_timestamp;	
	float enhance_factor;
	int enhance_time;
	int buffer_time;
	int impair_time;
	int activate_times;	//当日已开启双倍次数
	int refresh_timestamp;
public:
	enum{
		STATE_NORMAL = 0,
		STATE_ENHANCE,
		STATE_BUFFER,
		STATE_IMPAIR,
	};

	multi_exp_ctrl():state(STATE_NORMAL),timeout(0),last_timestamp(0),enhance_factor(0.f),enhance_time(0),buffer_time(0),impair_time(0),activate_times(0),refresh_timestamp(0){}

	inline float GetExpFactor()
	{
		float f[4] = {0.f, enhance_factor-1.f, 0.f, -0.9f};
		return f[(int)state];
	}

	void Update(gactive_imp* imp, int cur_time);
	bool ActivateMultiExp(gactive_imp* imp);
	bool DeactivateMultiExp(gactive_imp* imp);
	bool IncMultiExpTime(gactive_imp* imp, float e_factor, int e_time, int b_time, int i_time);
	//数据库读写
	bool DBLoadData(archive& ar);
	bool DBSaveData(archive& ar);
	//切换服务器
	bool Save(archive& ar);
	bool Load(archive& ar);

	void NotifyClientInfo(gactive_imp* imp);
	void NotifyClientState(gactive_imp* imp);
private:
	void StateInit(int cur_time);
	void StateReset(int cur_time);
};

#endif
