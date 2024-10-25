#ifndef __ONLINEGAME_GS_HISTORY_MANAGER_H__
#define __ONLINEGAME_GS_HISTORY_MANAGER_H__

#include <vector.h>

class itemdataman;
// 由uniquedataclient进行操作序列化所以本地不加锁
class history_manager
{
public:
	history_manager() : _initialized(false), _stagelimit(0), _stagevalue(0), _stageversion(-1) {}
	~history_manager() 	{}
public:
	bool Initialize(itemdataman & dataman);
	bool OnSetVersion(int version);									// 版本加载
	bool OnSetValue(int value, int stageid);						// 历史变量加载
	bool OnAdvance(int version, bool localflag, int retcode); 		// 版本推进
	bool OnStep(int value,int stageid,bool advance,int retcode ,bool localflag); 	// 历史变量变更

	int  GetLocalStageVal()  { return _stagevalue; }
	int  GetStageVersion() { return _stageversion +1; }
	int  GetStageLimit() { return _stagelimit; }	
	
private:
	bool _initialized;
	int  _stagelimit;
	int  _stagevalue;
	int  _stageversion;
};

#endif

