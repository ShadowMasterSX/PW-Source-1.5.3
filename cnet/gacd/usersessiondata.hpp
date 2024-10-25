#ifndef __GNET_USERSESSIONDATA_HPP
#define __GNET_USERSESSIONDATA_HPP

#include "strinfo.h"
#include "platform.h"
#include "cpuinfo.h"
#include "meminfo.h"
#include "mouseinfo.h"
#include "threadtime.h"
#include "stackpattern.h"
#include "adapterinfo.h"
#include "protostat.h"
#include "rpcdefs.h"
#include "acloginfo"
#include "usercodemanager.hpp"


namespace GNET
{
class UserData;
class UserSessionData
{
	UserData *m_pUserData;
	bool m_bNewbie;
	int m_iUserType;
	int m_iIP; 

	StrInfo* m_pStrInfo;
	
	UserCodeManager m_codeManager;

	Platform platform_info;
	CPUInfo cpu_info;
	MemInfo mem_info;
	AdapterInfo adapter_info;

	std::deque<Mouse> mouse_info;
	std::deque<ProtoStat> proto_stats;
	unsigned int m_uTickOfLastProtoStat;

	ThreadTime process_time;
	std::vector<ThreadTime> thread_times;

	PatternMap pattern_map;

	std::deque<ACLogInfo> logs;

	struct Config
	{
		int m_iTimerCheckInterval;
		unsigned int m_uMaxClientInfoCountPerRound;
		unsigned int m_uClientInfoTypeCount;

		int m_iMinGamedataCountForKeepAlive;
		int m_iMinPublicchatCountForKeepAlive;

		int m_iGamedataProCountBeingChecked;
		int m_iMaxGamedataCount;
		int m_iMouseinfoProCountBeingChecked;
		int m_iMinMouseinfoCount;
		int m_iMaxGamedataCountPermit;

		typedef std::vector<std::pair<int, int> > IgnoreUsers;
	    IgnoreUsers m_ignoreUsers;
    	typedef std::vector<std::pair<int, int> > IgnoreIPs;
	    IgnoreIPs m_ignoreIPs;

		bool IsIgnore(int iUserID, int iIP);

		VecStr m_forbidWindows;

		unsigned int m_uMaxIntervalOfClientInfo;

		unsigned int m_uMinTimeOfFlash;
	    unsigned int m_uMaxTimeBetweenFlash;
		unsigned int m_uMaxFlashTimesPermit;

		Config() : m_iTimerCheckInterval(270),
				   m_uMaxClientInfoCountPerRound(10),
				   m_uClientInfoTypeCount(8),

				   m_iMinGamedataCountForKeepAlive(50),
				   m_iMinPublicchatCountForKeepAlive(8),

				   m_iGamedataProCountBeingChecked(1000),
				   m_iMaxGamedataCount(1000),
				   m_iMouseinfoProCountBeingChecked(1000),
				   m_iMinMouseinfoCount(1000),
				   m_iMaxGamedataCountPermit(1000), 
					
				   m_uMaxIntervalOfClientInfo(600),
				   m_uMinTimeOfFlash(180),
				   m_uMaxTimeBetweenFlash(300),
				   m_uMaxFlashTimesPermit(5) 
		{ }
	};

	static Config s_config;

	unsigned int m_uTick;
	unsigned int m_uClientInfoCountPerRound;
	unsigned int m_uTickOnLastClientInfo;

	unsigned int m_uClientInfoCountForCheckType;
	std::set<int> m_clientInfoTypeSet;

	bool operator==(const UserSessionData &usd) const;
	UserSessionData(const UserSessionData &usd);
	UserSessionData& operator=(const UserSessionData &usd);
public:
	UserSessionData(UserData *pUserData, int iIP);
	~UserSessionData();
	
	const UserData* GetUser() const { return m_pUserData; }
	UserData* GetUser() { return m_pUserData; }

	bool AssureOnline(bool check_data = true) const;

	unsigned int GetLastProtoStatTime() const;

	bool IsNew() const { return m_bNewbie; }
	void SetNew(bool bNewbie) { m_bNewbie = bNewbie; }

	int GetUserType() const { return m_iUserType; }
	bool RefusePunish() const { return m_iUserType != 0; }
	bool RefuseSendCode() const { return m_iUserType != 0; }
	void SetUserType(int iUserType) { m_iUserType = iUserType; }
	int GetUserID() const;
	int GetIP() const { return m_iIP; }
	void SetIP(int iIP) { m_iIP = iIP; }

	static unsigned int GetFlashInter() { return s_config.m_uMaxTimeBetweenFlash; }
	static unsigned int GetFlashTimes() { return s_config.m_uMaxFlashTimesPermit; }

	void SetProcessTime(const ThreadTime &pt) { process_time = pt; }
	void SetThreadTimes(const std::vector<ThreadTime> &tt) { thread_times = tt; }

	const ThreadTime& GetProcessTime() const { return process_time; }
	const std::vector<ThreadTime>& GetThreadTimes() const { return thread_times; }

	UserCodeManager& GetCodeManager();

	const StrInfo* GetStrInfo() const { return m_pStrInfo; }
	const Platform& GetPlatform() const { return platform_info; }
	const CPUInfo& GetCPUInfo() const { return cpu_info; }
	const MemInfo& GetMemInfo() const { return mem_info; }
	const AdapterInfo& GetAdapterInfo() const { return adapter_info; }
	const std::deque<Mouse>& GetMouseInfo() const { return mouse_info; }
	const std::deque<ProtoStat>& GetProtoStats() const { return proto_stats; }
	const PatternMap& GetPatterns() const { return pattern_map; }	
	const std::deque<ACLogInfo>& GetLogs() const; 

	void SetPlatform(const Platform &pl) { platform_info = pl; }
	void SetCPUInfo(const CPUInfo &info) { cpu_info = info; }
	void SetMemInfo(const MemInfo &info) { mem_info = info; }
	void SetAdapterInfo(const AdapterInfo &info) { adapter_info = info; }

	void AppendMouseInfo(const Mouse &mouse);
	void AppendProtoStat(const ProtoStat &ps);
	void AppendPattern(const PatternMap &pm);
	void CheckCodeRes(short sCodeID, int iRes);
	void CommitCheater(int iCheatID, int iSubID);
	void OnTimer();
	static void OnUpdateConfig(const XmlConfig::Node *pRoot);

	void UpdateProcessStr(VecStr &more, VecStr &less);
	void UpdateWindowStr(VecStr &more, VecStr &less);
	void UpdateModuleStr(VecStrI &more, VecStrI &less);

	void Log(int iLogType, int iSubID);

	void CheckGameFreq();
	bool IsFlash() const;

};

};

#endif
