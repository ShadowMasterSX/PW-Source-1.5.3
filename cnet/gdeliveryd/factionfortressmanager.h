#ifndef __GNET_FACTIONFORTRESSMANAGER_H__
#define __GNET_FACTIONFORTRESSMANAGER_H__

#include "gfactionfortressdetail"
#include "gfactionfortressbriefinfo"
#include "gfactionfortressbattleinfo"
#include "factionext"
#define FACTION_FORTRESS_UPDATE_INTERVAL	1000000	//1秒update一次
#define FACTION_FORTRESS_CHECKSUM_ONUPDATE	10		//每次update检查10个
#define FACTION_FORTRESS_PAGE_SIZE			16

namespace GNET
{

void FactionFortressDetailToBrief(const GFactionFortressDetail & detail, GFactionFortressBriefInfo & brief);
	
class FactionFortressObj
{
public:
	FactionFortressObj():open(false),change_counter(0),syncdb(false),needsyncgs(false){}
	FactionFortressObj(const GFactionFortressDetail & _detail):open(false),change_counter(0),syncdb(false),needsyncgs(false),detail(_detail){}
	~FactionFortressObj(){}
	
	GFactionFortressDetail & GetDetail(){ return detail; }
	bool IsActive(){ return detail.info2.health > 0; }
	void SetOpen(bool b){ open = b; }

	/*gs数据以及健康度采用延迟写盘方式：
	1 收到操作请求后修改数据，IncChangeFlag
	2 update()中判断是否NeedSyncDB，如是则SyncDB 开始写盘
	3 写盘结束OnDBSync(counter)*/
	void IncChangeFlag()
	{ 
		if(++change_counter == 0) 
			change_counter = 1; 
	}
	bool NeedSyncDB(){ return change_counter != 0 && !syncdb; }
	void SyncDB();
	void OnDBSync(size_t save_counter);
	/*宣战相关采用即时写盘方式:
	1 收到操作请求后判断InSync，如否则开始写盘SetSync(true)
	2 写盘结束OnDBSync(0)*/
	bool InSyncDB(){ return change_counter != 0 || syncdb; }
	void SetSyncDB(bool b){ syncdb = b; }

	void SetNeedSyncGS(bool b){ needsyncgs = b && open; }
	bool NeedSyncGS(){ return needsyncgs; }
	void SyncGS(int server_id);
private:
	bool open;		//基地的开启状态是gs通知的，可能因为网络原因导致不准确
	size_t change_counter;
	bool syncdb;
	bool needsyncgs;
	GFactionFortressDetail detail;
};

class CreateFactionFortress;
class FactionFortressChallenge;
class PlayerInfo;
class GMailSyncData;
class FactionFortressMan : public IntervalTimer::Observer
{
public:
	enum
	{
		ST_INIT,		//正在初始化
		ST_OPEN,		//已开放
		ST_CHALLENGE,	//宣战阶段
		ST_BATTLE_WAIT,	//等待战斗阶段
		ST_BATTLE,		//战斗阶段
	};	
		
	typedef std::map<int/*factionid*/,FactionFortressObj> FortressMap;

	struct BATTLE_PERIOD
	{
		int challenge_start_time;
		int challenge_end_time;
		int battle_start_time;
		int battle_end_time;
		BATTLE_PERIOD(int cst,int cet,int bst,int bet):challenge_start_time(cst),challenge_end_time(cet),battle_start_time(bst),battle_end_time(bet){}
	};
	typedef std::vector<BATTLE_PERIOD> BattlePeriodList;
	
public:		
	FactionFortressMan():status(ST_INIT),lock("FactionFortressMan::lock"),health_update_time(0),update_cursor(0),server_id(0),world_tag(0),t_base(0){}
	~FactionFortressMan(){}
	static FactionFortressMan & GetInstance(){ static FactionFortressMan instance; return instance; }
	bool Initialize();
	void OnDBConnect(Protocol::Manager * manager, int sid);
	void OnDBLoad(const std::vector<GFactionFortressDetail>& list, bool finish);
	bool Update();

private:
	void FinalInit();
	bool IsOpen(){ return status != ST_INIT; }
	void UpdateTime(int cur_t);
	void UpdateStatus(int cur_t);
	void GetNextBattleTime(int cur_t, int& start, int& end);
	void ChallengeStart();
	void ChallengeEnd();
	void BattleStart();
	void BattleEnd();
	void BattleClear(const char * msg);
	
public:
	void RegisterServer(int server_id, int world_tag);
	int GameGetFortress(int factionid, GFactionFortressDetail & detail);
	int GamePutFortress(int factionid, const GFactionFortressInfo & info);
	void GameNotifyFortressState(int factionid, int state);
	void SyncFactionServer();
		
public:
	int TryCreateFortress(const CreateFactionFortress & proto, const GFactionFortressInfo & info, const PlayerInfo & ui, const GMailSyncData & sync);
	bool OnDBCreateFortress(const GFactionFortressDetail & detail);
	bool OnDBPutFortress(int factionid, size_t save_counter);
	bool OnDBDelFortress(int factionid);
	void OnDBSyncFailed(int factionid);
	bool CheckEnterFortress(int factionid, int dst_factionid, int & dst_world_tag);
	void GetFortressList(unsigned int & begin, int & status, std::vector<GFactionFortressBriefInfo> & list);
	int TryChallenge(const FactionFortressChallenge & proto, const PlayerInfo & ui, const GMailSyncData & sync);
	bool OnDBChallenge(int factionid, int target_faction); 
	void OnDelFaction(int factionid);
	void GetBattleList(int& status, std::vector<GFactionFortressBattleInfo>& list);
	bool GetFortress(int factionid, GFactionFortressBriefInfo & brief);
	bool GetFactionExt(int factionid, FactionExt & fe);	
	bool DebugDecHealthUpdateTime();
	void DebugAdjustBattlePeriod(bool fastmode);
private:
	int				status;
	Thread::Mutex 	lock;
	FortressMap		fortress_map;

	int health_update_time;
	int update_cursor;

	int	server_id;
	int	world_tag;	
	
	int 			t_base;		//每周周起始时间，周日00:00:00
	BattlePeriodList bp_list;
};

}
#endif
