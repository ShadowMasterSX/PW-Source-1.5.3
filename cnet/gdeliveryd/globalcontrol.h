#ifndef __GNET_GLOBALCONTROL_H__
#define __GNET_GLOBALCONTROL_H__

#include "gglobalcontroldata"
#include <sstream>

namespace GNET
{

class GlobalControl
{
	enum
	{
		FORBID_CTRL = 1,
		FORBID_ITEM = 2,
		FORBID_SVR = 3,
		FORBID_TASK = 4,
		FORBID_SKILL = 5,
		TRIGGER_CTRL = 6,
        FORBID_SHOPITEM = 7,
        FORBID_RECIPE = 8,
	};
	bool _initialized;
	GGlobalControlData _data;

	GlobalControl():_initialized(false){}
public:
	static GlobalControl * GetInstance(){ static GlobalControl instance; return &instance; }
	
	void OnDBConnect(Protocol::Manager * manager, int sid);
	void OnDBLoad(const GGlobalControlData& data);
	void OnGSConnect(Protocol::Manager * manager, int sid);

	int SetCashMoneyExchgRate(int rate);
	int SetCashMoneyExchgOpen(char open);
	int SetForbidList(const Octets cmd_list);
	int GetForbidList(Octets &forbid_cmd);
private:
	void SyncAllToGS(int sid = -1);
	void SyncCashMoneyExchgToGS(int sid = -1);
	void SyncForbidListToGS(int sid = -1);
	void SyncTriggerListToGS(int sid = -1);
	void SyncToDB();
	void MakeCMDList(std::vector<int> &forbid_list,std::vector<int> &cmd_list);
	void ForbidList2Stream(std::vector<int> &forbid_list,std::ostringstream &cmd_os,int type);
	bool IsCMERateValid(int rate){ return rate >= 500000 && rate <= 3000000; }
	void RemoveConflictCMD();
};

}

#endif
