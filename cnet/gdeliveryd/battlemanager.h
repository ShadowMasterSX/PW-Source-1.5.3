#ifndef __GNET_BATTLEMANAGER_H
#define __GNET_BATTLEMANAGER_H

#include <vector>
#include <map>

#include "thread.h"
#include "gterritorydetail"
#include "battlemapnotice.hpp"
#include "gchallengerinfo"
#include "gchallengerinfolist"
#include "groleinventory"
#include "citywar"
namespace GNET
{   

	class BattleManager : public IntervalTimer::Observer
	{
	public:
		enum
		{
			ST_OPEN      = 0x0001,  // 城战功能是否开启
			ST_BIDDING   = 0x0002,  // 领土竞价开始
			ST_FIGHTING  = 0x0004,  // 城战进行中
			ST_DATAREADY = 0x0008,  // 城战数据是否已经初始化
			ST_BONUSOK   = 0x0010,  // 本周领土收益已经发放
			ST_SETTIME   = 0x0020,  // 城战时间尚未设定
			//ST_SPECIALOK = 0x0040,
		};
		enum
		{
			CS_FIGHTING      = 0x0001,  // 战争进行中
			CS_BIDDING       = 0x0002,  // 其他帮派已经出价,正在进行数据库操作
			CS_SENDSTART     = 0x0004,  // 开始城战协议已经发出
			CS_BONUSOK       = 0x0008,  // 收益分配完毕
			CS_BATTLECANCEL  = 0x0010,  // 城战取消
		};
		enum
		{
			TERRITORY_NUMBER   = 52,        // 城市数目
			BEGIN_BID_TIME     = 327600,    // 宣战开始,星期三,19:00
			BEGIN_REWARD_TIME  = 475200,    // 发放收益,星期五,12:00
			DEFAULT_BID_TIME   = 86400,     // 宣战默认持续时间
			MAX_REWARD_TIME    = 21600,     // 战场收益发放时间
			MAX_BATTLE_TIME    = 10800,     // 城战持续时间
			MAX_BONUS          = 2000000000,// 最大领土收益 
			MAX_DELAY          = 2400,      // 约战最多延迟时间
		};

		typedef std::vector<GTerritoryDetail>  TVector;
		typedef std::map<int,int> ServerMap;
	private:
		time_t BidBeginTime() { return t_base + BEGIN_BID_TIME;}
		time_t BidDefDuration() { return t_base + BEGIN_BID_TIME + DEFAULT_BID_TIME;}
		time_t BidMaxDuration() { return t_base + BEGIN_BID_TIME + DEFAULT_BID_TIME + MAX_DELAY;}
		time_t RewardBeginTime() { return t_base + BEGIN_REWARD_TIME;}

	protected:
		ServerMap servers;
		Thread::RWLock locker;
		int status;   
		
		unsigned int bonusid;
		int countoflevel1;
		int countoflevel2;
		int countoflevel3;
		int max_bonus_count;
		int proctype;
		unsigned int specialid;
		int countofspecial;
		int specialproctype;
		int max_count_special;

		time_t t_base;      // 本周星期日,00:00
		time_t t_endbid;    // 挑战结束时间
		time_t t_forged;   

		int rand_num;
		
		BattleManager() : locker("BattleManager::locker"), status(0),rand_num(0)
		{ 
			t_base = 0;
			t_forged = 0;
			t_endbid = 0;
		}  
	public:
		TVector cities;

		~BattleManager() { }

		static BattleManager* GetInstance() { static BattleManager instance; return &instance;}
		bool SendMap(int roleid, unsigned int sid, unsigned int localsid);
		bool SendStatus(int roleid,unsigned int sid, unsigned int linkid);
		bool Initialize();
		int Challenge(const Protocol&, Protocol&);
		bool RegisterServer(int server, int world);
		int  FindServer(int map) { return servers[map]; }
		int  GetMapType(int id);
		bool LoadMap(std::vector<GTerritoryDetail>& v);
		bool SyncChallenge(std::vector<GTerritoryDetail>& v);

		bool OnChallenge(short result,int challenge_res,short cid,unsigned int deposit,unsigned int maxbonus,int fid,int ctime,Protocol& reply);
		bool OnBattleEnd(int id, int result, int defender, int attacker);
		bool OnBattleStart(int battleid, int retcode);
		bool OnDBConnect(Protocol::Manager *manager, int sid);
		bool OnDelFaction(unsigned int factionid);
		bool Update();
		void UpdateBid(time_t now);
		void UpdateBattle(time_t now);

		bool ChallengeMap(int roleid,int factionid);
		bool SendPlayer(int roleid, const Protocol& proto, unsigned int& localsid, unsigned int& linkid);
		bool FindSid(int roleid, unsigned int& localsid, unsigned int& linkid, unsigned int& gsid);
		char SelectColor(unsigned int factionid);
		bool GetMapNotice(BattleMapNotice&);
		bool SyncMapNotice();
		bool SyncMapNotice(short);
		bool SyncBattleFaction();
		void BeginChallenge();
		void BeginSendBonus();
		bool SendBonus();
		bool SendSpecialBonus();
		bool OnSendBonus(short ret, unsigned int fid,GRoleInventory &item, unsigned int money);
		void EndChallenge();
		bool ArrangeBattle();
		void TestBattle(int id, int challenger);
		void SetOwner(int id, int factionid);
		time_t UpdateTime();
		time_t GetTime();
		void SetForgedTime(time_t forge);
		bool IsAdjacent(short cityid, unsigned int fid);
		bool GetCityInfo(CityWar & cw);	
	//	ItVector::iterator GetCombination(const ItVector& list,int num);
	};
};
#endif

