#ifndef __GNET_USERDATA_HPP
#define __GNET_USERDATA_HPP

#include "usersessiondata.hpp"
#include "cheater.hpp"

namespace GNET
{

class UserData
{
	int m_iUserID;

	enum { LT_LOGINOUT = 0 };
	enum { LST_LOGIN = 0, LST_LOGOUT = 1 };
	struct LogItem
	{
		int m_iUserID;
		int m_iSubID;
		LogItem(int iUserID, int iSubID) : m_iUserID(iUserID), m_iSubID(iSubID) { }
		int GetLogType() const { return Cheater::CH_LOGINOUT; }
		int GetUserID() const { return m_iUserID; }
		int GetSubID() const { return m_iSubID; }
		unsigned int GetMaxSize() const { return 0x10000; }
		unsigned int GetMaxCount() const { return 16; }
	};
	enum { LC_MAX_COUNT = 4 };
	std::deque<ACLogInfo> m_logs;
	std::map<short, short> m_cheatTimesMap;
	std::deque<time_t> m_flashTimes;

	UserSessionData *m_pSessionData;

	UserData& operator=(const UserData &ud);
	bool operator==(const UserData &ud) const;
	UserData(const UserData &ud);
public:
	UserData(int iUserID);
	~UserData();

	short QueryCheatTimes(short sCheatID);

	int GetUserID() const { return m_iUserID; }

	unsigned int GetLoginTime() const;
	int GetLoginIP() const;
	ACLogInfoVector GetLogs() const; 
	UserSessionData* InitSessionData(int iIP);
	void ClearSessionData();
	bool IsOnline() const { return m_pSessionData != NULL; }

	void Log(int iLogType, int iSubID);

};

};

#endif
