
#include "gacdutil.h"
#include "userdatamanager.hpp"

namespace GNET
{

UserDataManager UserDataManager::s_instance;

bool UserDataManager::UserLogin(int iUserID, int iIP)
{
    UserSessionMap::iterator it = m_userSessionMap.find(iUserID);
    if( it != m_userSessionMap.end() ) return false;

	UserData *pUserData = NULL;
    UserMap::iterator it2 = m_userMap.find(iUserID);
    if( it2 != m_userMap.end() )
    {
		pUserData  = (*it2).second;
    }
	else
	{
		pUserData = new UserData(iUserID);
		m_userMap.insert(std::make_pair(iUserID, pUserData));
	}
	UserSessionData *pSessionData = pUserData->InitSessionData(iIP);
	m_userSessionMap.insert(std::make_pair(iUserID, pSessionData));

    return true;
}

void UserDataManager::OnTimer()
{
	if( !m_bRunning ) return;
	for(std::vector<int>::const_iterator it = m_scheduledLogoutUsers.begin(),ie = m_scheduledLogoutUsers.end();
			it != ie; ++it)
		UserLogout((*it));
	for(UserSessionMap::iterator it = m_userSessionMap.begin(), ie = m_userSessionMap.end(); it != ie; ++it)
		(*it).second->OnTimer();
}

void UserDataManager::ScheduleLogout(int iUserID)
{
	m_scheduledLogoutUsers.push_back(iUserID);
}

bool UserDataManager::UserLogout(int iUserID)
{
    UserSessionMap::iterator it = m_userSessionMap.find(iUserID);
    if( it == m_userSessionMap.end() ) return false;
    (*it).second->GetUser()->ClearSessionData();
    m_userSessionMap.erase(it);
    return true;
}

void UserDataManager::Run()
{   
    m_bRunning = true;
    DEBUG_PRINT_INIT("UserDataManager::Run()\n");
}

void UserDataManager::Stop()
{
    m_bRunning = false;
    ClearSession();
    DEBUG_PRINT_INIT("UserDataManager::Stop()\n");
} 

void UserDataManager::ClearSession()
{
	for(UserSessionMap::iterator it = m_userSessionMap.begin(), ie = m_userSessionMap.end(); it != ie; ++it)
		(*it).second->GetUser()->ClearSessionData();
	m_userSessionMap.clear();
	m_scheduledLogoutUsers.clear();
}

void UserDataManager::Clear()
{
	ClearSession();
	for(UserMap::iterator it = m_userMap.begin(), ie = m_userMap.end(); it != ie; ++it)
		delete it->second;
    m_userMap.clear();
}

};

