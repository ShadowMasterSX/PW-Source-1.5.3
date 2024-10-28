
#ifndef __ONLINEGAME_GS_ONLINE_AWARD_H__
#define __ONLINEGAME_GS_ONLINE_AWARD_H__

#include <vector>

class gplayer_imp;
class online_award
{
public:
	enum
	{ 
		CUR_VERSION = 1, 
		MIN_LEVEL_REQUIRED = 31,
		MAX_AWARD_TIME = 7 * 86400,
	};
	enum
	{
		AWARD_TYPE_EXP1 = 0,
		AWARD_TYPE_EXP2,
		AWARD_TYPE_EXP3,
		AWARD_TYPE_EXP4,
		MAX_AWARD_TYPE,
	};
	struct award_data
	{
		int type;
		int time;
		int reserved;
		award_data() : type(-1), time(0), reserved(0){}
	};

public:
	online_award() : _version(CUR_VERSION), _refresh_timestamp(0), _total_award_time(0), _award_list(MAX_AWARD_TYPE,award_data()){}

	void Update(gplayer_imp* imp, int cur_time);
	bool ActivateAward(gplayer_imp* imp, int type);
	bool DeactivateAward(gplayer_imp* imp, int type);
	bool IncAwardTime(gplayer_imp* imp, int type, int time); 
	int  SpendAwardTime(gplayer_imp* imp, int type, int time);
	void NotifyClientAllData(gplayer_imp* imp);
	void NotifyClientData(gplayer_imp* imp, int type);

	bool DBLoadData(archive& ar);
	bool DBSaveData(archive& ar);
	bool Save(archive& ar);
	bool Load(archive& ar);
	void Swap(online_award & rhs);

private:
	int _version;
	int _refresh_timestamp;
	int _total_award_time;
	abase::vector<award_data> _award_list;

};



#endif
