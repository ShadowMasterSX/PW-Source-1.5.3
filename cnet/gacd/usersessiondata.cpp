#include "gacdutil.h"
#include "usersessiondata.hpp"
#include "stringcache.hpp"
#include "punishmanager.hpp"
#include "userdatamanager.hpp"
#include "stringhelper.hpp"

namespace GNET
{

UserSessionData::Config UserSessionData::s_config;

UserSessionData::UserSessionData(UserData *pUserData, int iIP) 
	: m_pUserData(pUserData), m_bNewbie(true), m_iUserType(0), m_iIP(iIP), m_pStrInfo(NULL),
			m_codeManager(this), m_uTickOfLastProtoStat(0), m_uTick(1) , m_uClientInfoCountPerRound(0), 
				m_uTickOnLastClientInfo(0), m_uClientInfoCountForCheckType(0)
{  
	StringCache::GetInstance()->LoadString(GetUserID(), m_pStrInfo);
	if( s_config.IsIgnore(GetUserID(), m_iIP) ) m_iUserType = 1; 
	m_codeManager.Init();
}

UserSessionData::~UserSessionData() 
{ 
	m_codeManager.Clear();
	StringCache::GetInstance()->SaveString(GetUserID(), m_pStrInfo);
	m_pUserData = NULL;
}

unsigned int UserSessionData::GetLastProtoStatTime() const 
{ 
	return m_pUserData->GetLoginTime() + m_uTickOfLastProtoStat;
}

void UserSessionData::AppendProtoStat(const ProtoStat &ps)
{
	m_uTickOfLastProtoStat = m_uTick;
	proto_stats.push_back(ps);
	if(proto_stats.size()>5) proto_stats.pop_front();
}

bool UserSessionData::AssureOnline(bool check_game) const
{
	if( m_uTick - m_uTickOfLastProtoStat > 300 )
	{
		UserDataManager::GetInstance()->ScheduleLogout(GetUserID());
		return false;
	}
	if( !check_game ) return true;
	size_t ps_size = proto_stats.size();
	if( ps_size > 1 )
	{
		const ProtoStat &ps1 = proto_stats[ps_size-1];
		const ProtoStat &ps2 = proto_stats[ps_size-2];
		if( ps1.gamedatasend - ps2.gamedatasend > s_config.m_iMinGamedataCountForKeepAlive ||
			( ps1.gamedatasend == ps2.gamedatasend && ps1.acreport - ps2.acreport > s_config.m_iMinPublicchatCountForKeepAlive ) )
			return true;
	}
	return false;
}

bool UserSessionData::Config::IsIgnore(int iUserID, int iIP) 
{
    for(IgnoreUsers::const_iterator it = m_ignoreUsers.begin(), ie = m_ignoreUsers.end(); it != ie; ++it)
    {
        if( iUserID >= (*it).first && iUserID < (*it).second ) return true;
    }
    if( iIP == 0 ) return false;
    for(IgnoreIPs::const_iterator it = m_ignoreIPs.begin(), ie = m_ignoreIPs.end(); it != ie; ++it)
    {
        if( iIP >= (*it).first && iIP < (*it).second ) return true;
    }
    return false;
}

void UserSessionData::AppendPattern(const PatternMap &pm)
{
	for(PatternMap::const_iterator it = pm.begin(), ie = pm.end(); it != ie; ++it)
		pattern_map[(*it).first] += (*it).second;
}

void UserSessionData::CommitCheater(int iCheatID, int iSubID)
{
	if( RefusePunish() )
		PunishManager::GetInstance()->DeliverCheater(Cheater(GetUserID(), Cheater::CH_VIP_USER
		, iCheatID, 0));
	else
		PunishManager::GetInstance()->DeliverCheater(Cheater(GetUserID(), iCheatID
		, iSubID, m_pUserData->QueryCheatTimes(iCheatID)));
}

void UserSessionData::OnUpdateConfig(const XmlConfig::Node *pRoot)
{

	s_config.m_ignoreUsers.clear();
    s_config.m_ignoreIPs.clear();
    const XmlConfig::Node *manager = pRoot->GetFirstChild("punishmanager");
    XmlConfig::Nodes nodes = manager->GetChildren("ignoreids");
    for(XmlConfig::Nodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        std::string sf = (*it)->GetAttr("from");
        std::string st = (*it)->GetAttr("to");
        if( sf.empty() || st.empty() ) continue;
        int f = atoi(sf.c_str());
        int t = atoi(st.c_str());
        if( f < t )
        {
            DEBUG_PRINT_INIT("-=ignore ids from %d to %d\n", f, t);
            s_config.m_ignoreUsers.push_back(std::pair<int,int>(atoi(sf.c_str()), atoi(st.c_str())));
        }
    }
    nodes = manager->GetChildren("ignoreips");
    for(XmlConfig::Nodes::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        std::string sf = (*it)->GetAttr("from");
        std::string st = (*it)->GetAttr("to");
        if( sf.empty() || st.empty() ) continue;
        int f = StringHelper::ipstr_to_int(sf);
        int t = StringHelper::ipstr_to_int(st);
        if( f != 0 && t != 0 && f < t )
        {
            DEBUG_PRINT_INIT("-=ignore ips from %s to %s\n", sf.c_str(), st.c_str());
            s_config.m_ignoreIPs.push_back(std::pair<int,int>(f,t));
        }
    }
	
    const XmlConfig::Node *stat = pRoot->GetFirstChild("statmanager");
    
    size_t mininter = 1, maxinter = static_cast<size_t>(-1);
    stat->GetUIntAttr("mininter", &mininter);
    stat->GetUIntAttr("maxinter", &maxinter);
    stat->GetUIntAttr("std_info", &s_config.m_uClientInfoTypeCount);    
	stat->GetUIntAttr("info_inter_max", &s_config.m_uMaxIntervalOfClientInfo);
	stat->GetUIntAttr("info_inter_min", &s_config.m_uMinTimeOfFlash);
	stat->GetUIntAttr("flash_inter", &s_config.m_uMaxTimeBetweenFlash);
	stat->GetUIntAttr("flash_times", &s_config.m_uMaxFlashTimesPermit);
	stat->GetIntAttr("delta_gamedatasend", &s_config.m_iMinGamedataCountForKeepAlive);
	stat->GetIntAttr("delta_publicchat", &s_config.m_iMinPublicchatCountForKeepAlive);
	size_t t_timeout = 180;
	stat->GetUIntAttr("timeout", &t_timeout);
	s_config.m_iTimerCheckInterval = maxinter * 4 + mininter + t_timeout*2;
	s_config.m_uMaxClientInfoCountPerRound = (s_config.m_iTimerCheckInterval + t_timeout)/mininter + 1;

	const XmlConfig::Node *forbidstat = stat->GetFirstChild("forbidstat");
	if( forbidstat != NULL )
	{
		forbidstat->GetIntAttr("game_c", &s_config.m_iGamedataProCountBeingChecked);
		forbidstat->GetIntAttr("game_n", &s_config.m_iMaxGamedataCount);
		forbidstat->GetIntAttr("game_n_m", &s_config.m_iMaxGamedataCountPermit);
		forbidstat->GetIntAttr("mouse_c", &s_config.m_iMouseinfoProCountBeingChecked);
		forbidstat->GetIntAttr("mouse_n", &s_config.m_iMinMouseinfoCount);
	}
	s_config.m_forbidWindows.clear();
	const XmlConfig::Nodes fws = stat->GetChildren("forbidwin");
    for(XmlConfig::Nodes::const_iterator it = fws.begin(); it != fws.end(); ++it)
    {
		s_config.m_forbidWindows.push_back(StringHelper::utf8_to_iso1((*it)->GetAttr("str")));
	}
}

void UserSessionData::CheckCodeRes(short sCodeID, int iRes)
{
	m_codeManager.CheckRes(sCodeID, iRes, m_bNewbie);
}

UserCodeManager& UserSessionData::GetCodeManager() { return m_codeManager; }

bool UserSessionData::IsFlash() const { return m_uTickOnLastClientInfo == 0 && m_uTick > s_config.m_uMinTimeOfFlash; }

void UserSessionData::AppendMouseInfo(const Mouse &mouse)
{
	mouse_info.push_back(mouse);
	if(mouse_info.size()>10) mouse_info.pop_front();
}

void UserSessionData::CheckGameFreq()
{
	if( s_config.m_iGamedataProCountBeingChecked < 1 || s_config.m_iGamedataProCountBeingChecked > 5 
		|| s_config.m_iMouseinfoProCountBeingChecked < 0 || s_config.m_iMouseinfoProCountBeingChecked > 10 
		|| s_config.m_iMaxGamedataCount < 100 
	) return;
	int proto_n = proto_stats.size(), mouseinfo_n = mouse_info.size(), i = 0;
	if( proto_n < s_config.m_iGamedataProCountBeingChecked + 1 
		|| s_config.m_iMouseinfoProCountBeingChecked > 0 
		&& mouseinfo_n < s_config.m_iMouseinfoProCountBeingChecked + 1 
	) return;
	for(i = proto_n - 1; i > proto_n - 1 - s_config.m_iGamedataProCountBeingChecked; --i)
	{
		int iGamedataCount = proto_stats[i].gamedatasend - proto_stats[i-1].gamedatasend;
		if( iGamedataCount < s_config.m_iMaxGamedataCount ) return;
		if( iGamedataCount > s_config.m_iMaxGamedataCountPermit )
		{
			CommitCheater(Cheater::CH_GAMEDATA_FREQ_ERR, 1);
			return;
		}
	}
	if( s_config.m_iMouseinfoProCountBeingChecked > 0 )
	{
		for(i = mouseinfo_n -1; i > mouseinfo_n -1 - s_config.m_iMouseinfoProCountBeingChecked; --i)
		{
			if( mouse_info[i].GetAllCounts() - mouse_info[i-1].GetAllCounts() 
					> s_config.m_iMinMouseinfoCount ) return;
		}
	}
	CommitCheater(Cheater::CH_GAMEDATA_FREQ_ERR, 0);
}


void UserSessionData::OnTimer()
{
	m_codeManager.OnTimer();
	++m_uTick;
	if( m_uTick > m_uTickOnLastClientInfo + s_config.m_uMaxIntervalOfClientInfo && AssureOnline() )
	{
		CommitCheater(Cheater::CH_CLIENTINFO_FREQ_ERR, 0);
		m_uTickOnLastClientInfo = m_uTick;
	}
	if( m_uTick%s_config.m_iTimerCheckInterval ) return;
	CheckGameFreq();
	if( m_bNewbie ) 
	{ 
		m_bNewbie = false; 
		m_uClientInfoCountPerRound = 0; 
		return; 
	}

	if( m_uClientInfoCountPerRound > s_config.m_uMaxClientInfoCountPerRound )
		CommitCheater(Cheater::CH_CLIENTINFO_FREQ_ERR, m_uClientInfoCountPerRound);
	m_uClientInfoCountPerRound = 0;

	if( m_uTick/s_config.m_iTimerCheckInterval == 3 )
	{
		if( mouse_info.empty() && AssureOnline() )
			CommitCheater(Cheater::CH_CLIENTINFO_ABNORMAL, 3);
		/*
		else if( platform_info.buildnumber == 2800 || platform_info.buildnumber == 2900 )
		{
			CommitCheater(Cheater::CH_BAOSHI_USER, platform_info.buildnumber );
		}
		*/
		else
		{
			const VecStr &win_str = m_pStrInfo->window_str;
			for(VecStr::const_iterator it2 = s_config.m_forbidWindows.begin(), 
				ie2 = s_config.m_forbidWindows.end(); it2 != ie2; ++it2 )
			{
				for(VecStr::const_iterator it = win_str.begin(), ie = win_str.end(); it != ie; ++it)
				{
					if( (*it).find((*it2)) != std::string::npos )
					{
						CommitCheater(Cheater::CH_CLIENTINFO_ABNORMAL, 9);
						break;
					}
				}
			}
		}
	}
}

const std::deque<ACLogInfo>& UserSessionData::GetLogs() const
{
	return logs;
}

void UserSessionData::Log(int iLogType, int iSubID)
{
	if( iLogType == 1 && ( iSubID >= 0 && iSubID<50 && iSubID != 10)) // Client Info
	{
		if( iSubID == 0 )
			CommitCheater(Cheater::CH_ACPROTO_DECODE_ERR, 0);
		else
		{
			++m_uClientInfoCountPerRound;
			m_clientInfoTypeSet.insert(iSubID);
			if( ++m_uClientInfoCountForCheckType >= 2 * s_config.m_uClientInfoTypeCount )
			{
				if( (m_clientInfoTypeSet.size()) < s_config.m_uClientInfoTypeCount )
					CommitCheater(Cheater::CH_CLIENTINFO_ORDER_ERR, m_clientInfoTypeSet.size());
				m_uClientInfoCountForCheckType = 0;
				m_clientInfoTypeSet.clear();
			}
			m_uTickOnLastClientInfo = m_uTick;
		}
	}
	logs.push_back(ACLogInfo(iLogType, iSubID, time(0)));
	if( logs.size() > 50 )
		logs.pop_front();
}

void UserSessionData::UpdateProcessStr(VecStr &more, VecStr &less)
{
	VecStr &str = m_pStrInfo->process_str;
	for(VecStr::const_iterator it = less.begin(), ie = less.end(); it != ie; ++it)
        str.erase( std::remove(str.begin(), str.end(), (*it)), str.end());
    str.insert(str.end(), more.begin(), more.end());
}
void UserSessionData::UpdateWindowStr(VecStr &more, VecStr &less)
{
	VecStr &str = m_pStrInfo->window_str;
	for(VecStr::const_iterator it = less.begin(), ie = less.end(); it != ie; ++it)
        str.erase( std::remove(str.begin(), str.end(), (*it)), str.end());
    str.insert(str.end(), more.begin(), more.end());
        
}
void UserSessionData::UpdateModuleStr(VecStrI &more, VecStrI &less)
{
	VecStrI &str = m_pStrInfo->module_str;
	for(VecStrI::const_iterator it = less.begin(), ie = less.end(); it != ie; ++it)
        str.erase( std::remove(str.begin(), str.end(), (*it)), str.end());
    str.insert(str.end(), more.begin(), more.end());
}

int UserSessionData::GetUserID() const 
{
	return m_pUserData->GetUserID();
}

};

