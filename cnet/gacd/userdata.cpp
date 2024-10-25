#include "userdata.hpp"
#include "logmanager.hpp"
#include "gacdutil.h"

namespace GNET
{


UserData::UserData(int iUserID) 
	: m_iUserID(iUserID), m_pSessionData(NULL) 
{  
}

UserData::~UserData() 
{ 
	delete m_pSessionData;
}

UserSessionData* UserData::InitSessionData(int iIP) 
{ 
	if( m_pSessionData != NULL )
	{
#ifdef GACD_DEBUG_PRINT_DEBUG
		DEBUG_PRINT_DEBUG("session data pointer is not null when init\n");
		exit(0);
		delete m_pSessionData;
#endif
	}
	register unsigned long ulIP;
    __asm__ ("bswap %0" : "=r"(ulIP) : "0"(iIP));
	Log(0, ulIP);
	LogManager::GetInstance()->Log(LogItem(m_iUserID, ulIP));
	m_pSessionData = new UserSessionData(this, ulIP);
	return m_pSessionData;
}

short UserData::QueryCheatTimes(short sCheatID)
{
	return 1 + m_cheatTimesMap[sCheatID]++;
}

unsigned int UserData::GetLoginTime() const
{
	return m_logs.back().log_time;
}

int UserData::GetLoginIP() const
{
	return m_logs.back().subid;
}

ACLogInfoVector UserData::GetLogs() const
{
	ACLogInfoVector v;
	std::vector<ACLogInfo> &tv = v.GetVector();
	std::copy(m_logs.begin(), m_logs.end(), std::back_inserter(tv));
	if( m_pSessionData != NULL )
		std::copy(m_pSessionData->GetLogs().begin(), m_pSessionData->GetLogs().end(), std::back_inserter(tv));
	return v;
}

void UserData::Log(int iLogType, int iSubID)
{
	if( iLogType != LT_LOGINOUT )
	{
#ifdef GACD_DEBUG_PRINT_DEBUG
		DEBUG_PRINT_ERROR("assert logtype failed in userdata\n");
		exit(0);
#endif
	}
	else
	{
		m_logs.push_back(ACLogInfo(LT_LOGINOUT, iSubID, time(NULL)));
		if( m_logs.size() > LC_MAX_COUNT )
			m_logs.pop_front();
	}
}

void UserData::ClearSessionData()
{ 
	if( m_pSessionData != NULL )
	{
		if( m_pSessionData->IsFlash() )
		{
			time_t tNow = time(NULL);
			if( m_flashTimes.empty() )
				m_flashTimes.push_back(tNow);
			else
			{
				if( (unsigned int)(tNow - m_flashTimes.back()) > UserSessionData::GetFlashInter() )
					m_flashTimes.clear();
				else
				{	
					if( m_flashTimes.size() > UserSessionData::GetFlashTimes() )
					{
						m_pSessionData->CommitCheater(Cheater::CH_FLASH_LOG, 0);
						m_flashTimes.clear();
					}
					else
						m_flashTimes.push_back(tNow);
				}
			}
		}
		else
			m_flashTimes.clear();
		delete m_pSessionData;
	}
	else
	{
#ifdef GACD_DEBUG_PRINT_DEBUG
	DEBUG_PRINT_DEBUG("session data pointer is null when clear\n");
	exit(0);
#endif
	}
	m_pSessionData = NULL;
	Log(LT_LOGINOUT, LST_LOGOUT);
	LogManager::GetInstance()->Log(LogItem(m_iUserID, LST_LOGOUT));
}


};

