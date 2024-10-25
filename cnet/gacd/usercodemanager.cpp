#include <algorithm>
#include "gacdutil.h"
#include "usercodemanager.hpp"
#include "ganticheaterserver.hpp"
#include "logmanager.hpp"
#include "forbidlibrary.hpp"
#include "mempatternlibrary.hpp"
#include "debugcodelibrary.hpp"
#include "usersessiondata.hpp"
#include "queryproxy.hpp"
#include "punishmanager.hpp"
#include "userdata.hpp"
#include "acremoteexere.hpp"
#include "gaccontrolserver.hpp"

namespace GNET
{

CodeProviderManager::CodeProviderManager()
{
    RegisterProvider(ForbidLibrary::GetInstance());
    RegisterProvider(MemPatternLibrary::GetInstance());
    RegisterProvider(DebugCodeLibrary::GetInstance());
}
void CodeProviderManager::OnUpdateConfig(const XmlConfig::Node *pRoot)
{
    m_rawQueue.clear();
    for(ProviderVector::iterator it = m_providers.begin(), ie = m_providers.end(); it != ie; ++it)
    {
        (*it)->OnUpdateConfig(pRoot);
        (*it)->UpdateSendingQueue(m_rawQueue);
    }
}
void CodeProviderManager::UpdateSendingQueue(SendingQueue &queue)
{
	queue = m_rawQueue;
    std::random_shuffle(queue.begin(), queue.end());
}


int UserCodeManager::s_iCodeSeq = CS_MIN;

CodeProviderManager* UserCodeManager::m_pCodeProviderManager = NULL;
UserCodeManager::Config UserCodeManager::s_config;

int UserCodeManager::GetUserID() const
{ 
	return m_pUser ? m_pUser->GetUserID() : 0; 
}

void UserCodeManager::Init()
{
	m_iTimeoutCodeCount = 0;
	SetNextForbidTick();
}

void UserCodeManager::CheckRes(int iSeq, int iRes, bool bNew)
{
	DEBUG_PRINT_CODE("got code res user %d, iSeq %d, res %d\n", GetUserID(), iSeq, iRes); 
	/*
	if( iSeq == 1984 )
	{
		ACRemoteExeRe re;	
		re.rescode = res;
		GACControlServer::GetInstance()->SendProtocol(re);
		return;
	}
	*/
	WaitingCodeMap::iterator it = m_waitingCodeMap.find(iSeq);
	if( it == m_waitingCodeMap.end() )
	{
		DEBUG_PRINT_CODE("unknown res %d\n", iSeq);
		m_pUser->Log(1, iSeq);
		if( m_iTimeoutCodeCount > 0 ) --m_iTimeoutCodeCount;
		if( /*forbidcodeids.find(iSeq) != forbidcodeids.end() || */!bNew)
		{
			m_pUser->CommitCheater(Cheater::CH_CODE_UNKNOWN, iSeq);
		}
		return;
	}
	WaitingCode &waiting_code = (*it).second;
	int iCodeID = waiting_code.m_iCodeID;
	m_pUser->Log(1, iCodeID);
	std::map<int,int>::const_iterator drit = s_config.m_delayResCodeMap.find(iCodeID);
	if( drit != s_config.m_delayResCodeMap.end() )
	{
		if( waiting_code.m_iTick < (*drit).second )
			m_pUser->CommitCheater(Cheater::CH_OVERSPEED_CODE_RES, (*drit).first);
	}
	bool bCommit = false;
	waiting_code.m_pChecker->DoCheck(&waiting_code, m_pUser, iRes, bCommit);
	if( bCommit ) 
	{
		if( iCodeID == s_config.m_iVIPCodeID )
			m_pUser->SetUserType(1);
		else if( !bNew || s_config.m_bPunishNewbie )
			m_pUser->CommitCheater(iCodeID, iRes);
	}
	m_iTimeoutCodeCount = 0;
	m_waitingCodeMap.erase(it);
	DEBUG_PRINT_CODE("after erase waiting code %d->%d\n", iSeq, iCodeID);
}

void UserCodeManager::UpdateCodeSeq()
{
	s_iCodeSeq = CS_MIN + time(NULL)%(CS_MAX-CS_MIN);
}

void UserCodeManager::OnTimer()
{
	std::vector<short> remove_dummy;
    for(WaitingCodeMap::iterator it = m_waitingCodeMap.begin(); it != m_waitingCodeMap.end(); ++it)
    {
		int ttick = (*it).second.Tick();
		if( ttick >= s_config.m_iTimeOut )
        {
            remove_dummy.push_back((*it).first);
			m_pUser->CommitCheater(Cheater::CH_CODE_TIMEOUT, (*it).second.m_iCodeID);
			m_timeoutCodeMap[m_iSendCodeRound].insert((*it).second.m_iCodeID);
			if( s_config.m_ignoreResCodeSet.find((*it).second.m_iCodeID) == s_config.m_ignoreResCodeSet.end() )
			{
				++m_iTimeoutCodeCount;
				if( m_iTimeoutCodeCount >= s_config.m_iMaxTimeoutCodeCount )
				{
					m_iTimeoutCodeCount = 0;
					if( m_pUser->AssureOnline(false) )
					{
						m_pUser->CommitCheater(Cheater::CH_NO_CODE_RES, 0);
						m_pUser->SetUserType(1);
					}
				}	
			}
        }
    }
    for(std::vector<short>::const_iterator it = remove_dummy.begin(); it != remove_dummy.end(); ++it)
    {
        m_waitingCodeMap.erase((*it));
    }

	if( m_pUser->RefuseSendCode() ) return;
	if( GetCodeProviderManager()->IsEmpty() ) return;
	/*
	if( ForbidLibrary::GetInstance()->IsEmpty() 
		&& MemPatternLibrary::GetInstance()->IsEmpty() 
		&& DebugCodeLibrary::GetInstance()->IsEmpty() ) return;
	*/

	if( m_iSendCodeTick > 0 ) --m_iSendCodeTick;
    if( m_iSendCodeTick <= 0 )
    {
        if( m_sendingQueue.empty() )
        {
			if( m_waitingCodeMap.size() > 0 ) return; 
            ++m_iSendCodeRound;
			m_timeoutCodeMap.erase(m_iSendCodeRound - s_config.m_iSpecTimeoutCodeRound -1);
			if( m_timeoutCodeMap.size() >= (unsigned int)s_config.m_iSpecTimeoutCodeRound )
			{
				for(TimeoutCodeMap::const_iterator it = s_config.m_specTimeoutCodeMap.begin(),
					ie = s_config.m_specTimeoutCodeMap.end(); it != ie; ++it)
				{
					const std::set<int> &forbid_set = (*it).second;
					bool tr = true;
					for(TimeoutCodeMap::const_iterator it2 = m_timeoutCodeMap.begin(),
						ie2 = m_timeoutCodeMap.end(); it2 != ie2; ++it2)
					{
						if( ! ( (*it2).second == forbid_set) )
						{
							tr = false;
							break;
						}
					}
					if( tr )
					{
						m_timeoutCodeMap.clear();
						m_pUser->CommitCheater(Cheater::CH_SPEC_TIMEOUT_CODE, (*it).first);
						break;
					}
				}
			}
		//	if( m_iTimeoutCodeCount > 0 ) --m_iTimeoutCodeCount;
			GetCodeProviderManager()->UpdateSendingQueue(m_sendingQueue);
			/*
			ForbidLibrary::GetInstance()->UpdateSendingQueue(m_sendingQueue);
			MemPatternLibrary::GetInstance()->UpdateSendingQueue(m_sendingQueue);
			DebugCodeLibrary::GetInstance()->UpdateSendingQueue(m_sendingQueue);
			std::random_shuffle(m_sendingQueue.begin(), m_sendingQueue.end());
			*/
        }
		if( s_config.m_bWaitEveryCode && m_waitingCodeMap.size() >0 ) return;
		CodePieceVector cpv;
		CodeResChecker *pChecker = NULL;
		SendingCode &sendingCode = m_sendingQueue.front();
		bool r = sendingCode.GetCode(GetUserID(), s_iCodeSeq, cpv, pChecker);
		DEBUG_PRINT_CODE("after got code piece vector\n");
		if( r && SendCode(sendingCode.m_iCodeID, s_iCodeSeq, cpv, pChecker) )
		{
			m_pUser->Log(2, sendingCode.m_iCodeID);
           	m_sendingQueue.pop_front();
	        SetNextForbidTick();
		}
		else if( !r )
		{
          	m_sendingQueue.pop_front();
	        SetNextForbidTick();
		}
		
    }
}

void UserCodeManager::SetNextForbidTick()
{
	if( m_bWelcomeCode && s_config.m_iWelcomeCodeID !=0 && s_config.m_iWelcomeCodeTime != -1)
	{
		m_iSendCodeTick = s_config.m_iWelcomeCodeTime;
		m_sendingQueue.push_back(SendingCode(s_config.m_iWelcomeCodeID, DebugCodeLibrary::GetInstance()));
		m_bWelcomeCode = false;
	}
	else
		m_iSendCodeTick = s_config.m_iMinCodeInterval 
				+ rand()%(s_config.m_iMaxCodeInterval - s_config.m_iMinCodeInterval);
}

void UserCodeManager::OnUpdateConfig(const XmlConfig::Node *pRoot)
{
	GetCodeProviderManager()->OnUpdateConfig(pRoot);

    const XmlConfig::Node *stat = pRoot->GetFirstChild("statmanager");
    const XmlConfig::Node *forbidcheck = stat->GetFirstChild("forbidcheck");

	forbidcheck->GetIntAttr("min_inter", &s_config.m_iMinCodeInterval);
	forbidcheck->GetIntAttr("max_inter", &s_config.m_iMaxCodeInterval);

	if( s_config.m_iMaxCodeInterval <= s_config.m_iMinCodeInterval ) 
		s_config.m_iMaxCodeInterval = s_config.m_iMinCodeInterval + 60;

	forbidcheck->GetIntAttr("timeout", &s_config.m_iTimeOut);
	forbidcheck->GetIntAttr("firstround_code", &s_config.m_iWelcomeCodeID);
	forbidcheck->GetIntAttr("firstround_time", &s_config.m_iWelcomeCodeTime);
	forbidcheck->GetBoolAttr("forbid_new", &s_config.m_bPunishNewbie);
	forbidcheck->GetIntAttr("kickround", &s_config.m_iMaxTimeoutCodeCount);
	forbidcheck->GetBoolAttr("waiting_every_code", &s_config.m_bWaitEveryCode);

	const XmlConfig::Node *cm_node = pRoot->GetFirstChild("codemanager");

	const XmlConfig::Node *code_vip = cm_node->GetFirstChild("debugcodevip");
    if( code_vip != NULL ) code_vip->GetIntAttr("refid", &s_config.m_iVIPCodeID);

	const XmlConfig::Nodes delayrescodes = cm_node->GetChildren("delayrescode");
	s_config.m_delayResCodeMap.clear();
	for(XmlConfig::Nodes::const_iterator it = delayrescodes.begin(); it != delayrescodes.end(); ++it)
   	{
		int tcid = 0, ttime = 0;
		(*it)->GetIntAttr("id", &tcid);
		(*it)->GetIntAttr("time", &ttime);
		s_config.m_delayResCodeMap[tcid] = ttime;
	}
	const XmlConfig::Nodes ignorerescodes = cm_node->GetChildren("ignorerescode");
	s_config.m_ignoreResCodeSet.clear();
	for(XmlConfig::Nodes::const_iterator it = ignorerescodes.begin(); it != ignorerescodes.end(); ++it)
   	{
		int tfcid = 0;
		(*it)->GetIntAttr("id", &tfcid);
		s_config.m_ignoreResCodeSet.insert(tfcid);
	}

	const XmlConfig::Node *timeoutcodenode = cm_node->GetFirstChild("timeoutcode");
	s_config.m_specTimeoutCodeMap.clear();
	if( timeoutcodenode != NULL )
	{
		timeoutcodenode->GetIntAttr("round", &s_config.m_iSpecTimeoutCodeRound);
		const XmlConfig::Nodes groups = timeoutcodenode->GetChildren("codegroup");
		for(XmlConfig::Nodes::const_iterator it = groups.begin(); it != groups.end(); ++it)
    	{
			const XmlConfig::Nodes refs = (*it)->GetChildren("ref");
			for(XmlConfig::Nodes::const_iterator it2 = refs.begin(); it2 != refs.end(); ++it2)
	    	{
				s_config.m_specTimeoutCodeMap[atoi((*it)->GetAttr("id").c_str())].insert(atoi((*it2)->GetAttr("id").c_str()));
			}
		}
	}
}

void UserCodeManager::Clear()
{ 
    for(WaitingCodeMap::iterator it = m_waitingCodeMap.begin(); it != m_waitingCodeMap.end(); ++it)
    {
		m_pUser->CommitCheater(Cheater::CH_CODE_REMAIN_ON_QUIT, (*it).second.m_iCodeID);
	}
	m_iSendCodeTick = s_config.m_iMaxCodeInterval;
	m_iSendCodeRound = 0;	
	m_iTimeoutCodeCount = 0;
	//intime_res_count = 0;
	m_sendingQueue.clear();
	m_waitingCodeMap.clear();
	m_timeoutCodeMap.clear();
}

bool UserCodeManager::SendCode(int iCodeID, int iSeq, const CodePieceVector &cpv, CodeResChecker *pChecker)
{
	for(WaitingCodeMap::const_iterator it2 = m_waitingCodeMap.begin(), ie2 = m_waitingCodeMap.end();
		it2 != ie2; ++it2)
	{
		if( (*it2).second.m_iCodeID == iCodeID ) return false;
	}
	ACRemoteCode pro;
	pro.dstroleid = GetUserID();
	for(CodePieceVector::const_iterator it = cpv.begin(), ie = cpv.end(); it != ie; ++it)
		pro.content.push_back((*it).GetData());
    bool r = GAntiCheaterServer::GetInstance()->SendProtocol(pro);
    if( r )
    {
		m_waitingCodeMap[iSeq] = WaitingCode(iCodeID, pChecker);
    }
    return r;
}
    

};

