#ifndef __GNET_USERDATAMANAGER_HPP
#define __GNET_USERDATAMANAGER_HPP

#include <map>
#include <algorithm>
#include "userdata.hpp"

namespace GNET
{

class UserDataManager
{
	bool m_bRunning;

    typedef std::map<int, UserData*> UserMap;
    UserMap m_userMap;

	typedef std::map<int, UserSessionData*> UserSessionMap;
	UserSessionMap m_userSessionMap;

	std::vector<int> m_scheduledLogoutUsers;

	static UserDataManager s_instance;

	UserDataManager() : m_bRunning(false) { }
	UserDataManager(const UserDataManager &udm);
	UserDataManager& operator=(const UserDataManager &udm);
	bool operator==(const UserDataManager &udm) const;

public:

	~UserDataManager() { Clear(); }

	static UserDataManager *GetInstance() { return &s_instance; }

	void Clear();
	void ClearSession();

	bool UserLogin(int iUserID, int iIP);
	bool UserLogout(int iUserID);

	void ScheduleLogout(int iUserID);

	void Run();
	void Stop();

	size_t GetUserNumber() const { return m_userMap.size(); }
	size_t GetOnlineNumber() const { return m_userSessionMap.size(); }

	void OnTimer();

	template<class T>
	bool Visit(int iUserID, T visitor)
	{
		UserMap::iterator it = m_userMap.find(iUserID);
		if( it == m_userMap.end() ) return false;
		visitor->operator()(it->second);
		return true;
	}

	template<class T>
	bool VisitOnline(int iUserID, T visitor)
	{
		UserSessionMap::iterator it = m_userSessionMap.find(iUserID);
		if( it == m_userSessionMap.end() ) return false;
		visitor->operator()(it->second);
		return true;
	}

    template<class T>
    void Travel(T visitor)
    {
		for(UserMap::iterator it = m_userMap.begin(), ie = m_userMap.end(); it != ie; ++it)
			visitor->operator()((it->second));
    }
	
    template<class T>
    void TravelOnline(T visitor)
    {
		for(UserSessionMap::iterator it = m_userSessionMap.begin(), ie = m_userSessionMap.end(); it != ie; ++it)
			visitor->operator()((it->second));
    }

};

};

#endif
