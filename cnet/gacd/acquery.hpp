
#ifndef __GNET_ACQUERY_HPP
#define __GNET_ACQUERY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acq"
#include "strinfo.h"
#include "gacdutil.h"

#include "acquseronline.hpp"
#include "acquserstrs.hpp"
#include "acqplatforminfo.hpp"
#include "accheaters.hpp"
#include "acqloginfo.hpp"
#include "acqmouseinfo.hpp"
#include "acqthreadtimes.hpp"
#include "acqprotocolstats.hpp"
#include "acversion.hpp"
#include "configmanager.hpp"
#include "acqstrowner.hpp"
#include "queryproxy.hpp"
#include "acqpatterns.hpp"
#include "patternmanager.hpp"
#include "punishmanager.hpp"
#include "logmanager.hpp"

namespace GNET
{

typedef std::map<std::string, int, stringcasecmp> StringMap;
struct StringVisitor
{
	StringMap window_map;
	StringMap process_map;
	StringMap module_map;
	void operator() (const UserSessionData *data)
	{
#ifdef GACD_DEBUG_PRINT_DEBUG
		if( data == NULL )
		{
			GACD_STD_DEBUG_PRINT("string visitor visit null session data pointer\n");
			exit(0);
		}
#endif
		const StrInfo *info = data->GetStrInfo();
		if( info == NULL ) 
		{
#ifdef GACD_DEBUG_PRINT_DEBUG
			GACD_STD_DEBUG_PRINT("string visitor visit null string  pointer\n");
			exit(0);
#endif
			return;
		}
		else
		{
			DEBUG_PRINT_DEBUG("string pointer is %08x\n", info);
		}
		for(VecStr::const_iterator it = info->process_str.begin(), ie = info->process_str.end(); it != ie; ++it)
			process_map[(*it)]++;
		for(VecStr::const_iterator it = info->window_str.begin(), ie = info->window_str.end(); it != ie; ++it)
			window_map[(*it)]++;
		for(VecStrI::const_iterator it = info->module_str.begin(), ie = info->module_str.end(); it != ie; ++it)
			module_map[(*it).first]++;
	}
};

struct StringInterVisitor
{
	std::set<int> users;
	bool start;
	bool end;
	StrInfo info;
	StringInterVisitor() : start(false), end(false) { }
	void operator() (const UserSessionData *data)
	{
		if( end ) return;
		if( users.find(data->GetUserID()) == users.end() ) return;
		if( !start ) 
		{ 
			const StrInfo *pStrInfo = data->GetStrInfo();
			if( pStrInfo )
			{
				info = *pStrInfo; 
				info.Unique(); 
				start = true; 
			}
		}
		else 
		{
			const StrInfo *pStrInfo = data->GetStrInfo();
			if( pStrInfo )
			{
				info = info + *pStrInfo; 	
			}
		}
		end = info.IsEmpty();
		#ifdef GACD_DEBUG_PRINT_QUERY
		DEBUG_PRINT_QUERY("\tafter search %d\n", data->GetUserID());
		info.Dump();
		DEBUG_PRINT_QUERY("\n");
		#endif
	}
};

struct PatternVisitor
{
	PatternMap pattern_map;
	void operator() (const UserSessionData *data)
	{
		pattern_map = data->GetPatterns();
	}
};

typedef std::map<Platform, int> PlatformMap;
typedef std::map<CPUInfo, int> CPUMap;
typedef std::map<MemInfo, int> MemMap;
typedef std::map<AdapterInfo, int> AdapterMap;
struct PlatformVisitor
{
	PlatformMap platform_map;
	CPUMap cpu_map;
	MemMap mem_map;
	AdapterMap adapter_map;
	void operator() (const UserSessionData *data)
	{
		const Platform &pl = data->GetPlatform();
		if(! pl.IsEmpty() ) platform_map[pl]++;
		const CPUInfo &pl1 = data->GetCPUInfo();
		if(! pl1.IsEmpty() ) cpu_map[pl1]++;
		const MemInfo &pl2 = data->GetMemInfo();
		if(! pl2.IsEmpty() ) mem_map[pl2]++;
		const AdapterInfo &pl3 = data->GetAdapterInfo();
		if(! pl3.IsEmpty() ) adapter_map[pl3]++;
	}
};

struct PatternOwnerVisitor
{
	StackPattern sp;
	std::vector<int> v;
	void operator() (const UserSessionData *data)
	{
		const PatternMap &pm = data->GetPatterns();
		if( pm.find(sp) != pm.end() )
			v.push_back(data->GetUserID());
	}
};

struct IPOwnerVisitor
{
	int ip;
	std::vector<int> v;
	void operator() (const UserSessionData *data)
	{
		if( data->GetIP() == ip )
			v.push_back(data->GetUserID());
	}
};

struct StrOwnerVisitor
{
	std::string str;
	int type;
	std::vector<int> v;
	void operator() (const UserSessionData *data)
	{
		const StrInfo *info = data->GetStrInfo();
		if( info == NULL ) return;
		if( type == 6 )
		{
			for(VecStrI::const_iterator it = info->module_str.begin(), ie = info->module_str.end()
				; it != ie; ++it)
            {
                if( 0 == strcasecmp( (const char*)(str.c_str()), (*it).first.c_str()))
                {
                    v.push_back(data->GetUserID());
					DEBUG_PRINT_QUERY("find str %s at user %d\n", str.c_str(), data->GetUserID());
                    break;
                }
            }
		}
		else if( type == 8 )
		{
			for(VecStr::const_iterator it = info->process_str.begin(), ie = info->process_str.end()
				; it != ie; ++it)
            {
                if( 0 == strcasecmp( (const char*)(str.c_str()), (*it).c_str()))
                {
                    v.push_back(data->GetUserID());
					DEBUG_PRINT_QUERY("find str %s at user %d\n", str.c_str(), data->GetUserID());
                    break;
                }
            }
		}
		else if( type == 9 )
		{
			for(VecStr::const_iterator it = info->window_str.begin(), ie = info->window_str.end()
				; it != ie; ++it)
            {
                if( 0 == strcasecmp( (const char*)(str.c_str()), (*it).c_str()))
                {
                    v.push_back(data->GetUserID());
					DEBUG_PRINT_QUERY("find str %s at user %d\n", str.c_str(), data->GetUserID());
                    break;
                }
            }
		}
	}
};

class ACQuery : public GNET::Protocol
{
	#include "acquery"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if( acq.query_type == AC_QUERY_WHO )
		{
			ACQUserOnline re;
			re.acq = acq;
			if( params.size() == 1 )
			{
				re.users.push_back(IntData(UserDataManager::GetInstance()->GetUserNumber()));
				re.users.push_back(IntData(UserDataManager::GetInstance()->GetOnlineNumber()));
			}
			else
			{
				UserDataManager::GetInstance()->TravelOnline(&re);
				re.users.push_back(IntData(UserDataManager::GetInstance()->GetUserNumber()));
			}
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_SHUTDOWN )
		{
			exit(0);
		}
		else if( acq.query_type == AC_QUERY_STR )
		{
			ACQUserStrs re;
			re.acq = acq;
			StringVisitor sv;
			if( acq.roleid == -1 )
				UserDataManager::GetInstance()->TravelOnline(&sv);
			else
				UserDataManager::GetInstance()->VisitOnline(acq.roleid, &sv);
			DEBUG_PRINT_DEBUG("after string visitor visit user %d\n", acq.roleid);
			for(StringMap::const_iterator it = sv.window_map.begin(); it != sv.window_map.end(); ++it)
			{
				const std::string &str = (*it).first;
				if( str.rfind("QQMusic_SmallClient>") == std::string::npos
					&& str.rfind(":<>") == std::string::npos 
					&& str.rfind("Tencent VoIP>") == std::string::npos )
		        re.windows.push_back(IntOctets((*it).second , Octets(str.c_str(), str.size())));
			}
			for(StringMap::const_iterator it = sv.process_map.begin(); it != sv.process_map.end(); ++it)
		        re.processes.push_back(IntOctets((*it).second , Octets((*it).first.c_str(), (*it).first.size())));
			for(StringMap::const_iterator it = sv.module_map.begin(); it != sv.module_map.end(); ++it)
		        re.modules.push_back(IntOctets((*it).second , Octets((*it).first.c_str(), (*it).first.size())));
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_INTER_STR )
		{
			ACQUserStrs re;
			re.acq = acq;
			StringInterVisitor siv;
			for(size_t i=0; i< params.size(); ++i)
				siv.users.insert(params[i].m_int);
			UserDataManager::GetInstance()->TravelOnline(&siv);
			for(VecStr::const_iterator it = siv.info.window_str.begin(); it != siv.info.window_str.end();++it)
		        re.windows.push_back(IntOctets(1 , Octets((*it).c_str(), (*it).size())));
			for(VecStr::const_iterator it = siv.info.process_str.begin();it != siv.info.process_str.end();++it)
		        re.processes.push_back(IntOctets(1, Octets((*it).c_str(), (*it).size())));
			for(VecStrI::const_iterator it = siv.info.module_str.begin(); it != siv.info.module_str.end();++it)
		        re.modules.push_back(IntOctets((*it).second , Octets((*it).first.c_str(), (*it).first.size())));
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_PLATFORM_INFO )
		{
			ACQPlatformInfo re;
			re.acq = acq;
			PlatformVisitor pv;
			if( acq.roleid == -1)
				UserDataManager::GetInstance()->TravelOnline(&pv);
			else
				UserDataManager::GetInstance()->VisitOnline(acq.roleid, &pv);
			for(PlatformMap::const_iterator it = pv.platform_map.begin(); it != pv.platform_map.end(); ++it)
    		{
				const Platform &pl = (*it).first;
				re.platform_info.push_back(ACPlatformInfo(pl.id, pl.majorversion, pl.minorversion,
					pl.buildnumber, (*it).second));
		    }
			for(CPUMap::const_iterator it = pv.cpu_map.begin(); it != pv.cpu_map.end(); ++it)
    		{
				const CPUInfo &pl = (*it).first;
				re.cpu_info.push_back(ACCPUInfo(pl.arch, pl.level, pl.ct, (*it).second));
		    }
			for(MemMap::const_iterator it = pv.mem_map.begin(); it != pv.mem_map.end(); ++it)
    		{
				const MemInfo &pl = (*it).first;
				re.mem_info.push_back(ACMemInfo(pl.ct, (*it).second));
		    }
			for(AdapterMap::const_iterator it = pv.adapter_map.begin(); it != pv.adapter_map.end(); ++it)
    		{
				const AdapterInfo &pl = (*it).first;
				re.adapter_info.push_back(ACAdapterInfo(
					Octets(pl.des.c_str(), pl.des.size()), (*it).second));
		    }
				
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_CHEATER )
		{
			ACCheaters re;
			LogManager::GetInstance()->QueryCheaters(re.cheaters);
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_THREADTIMES )
		{
			ACQThreadTimes re;
			re.acq = acq;
			UserDataManager::GetInstance()->VisitOnline(acq.roleid, &re);
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_PROTOSTATS )
		{
			ACQProtocolStats re;
			re.acq = acq;
			UserDataManager::GetInstance()->VisitOnline(acq.roleid, &re);
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_FORBID )
		{
			if( acq.roleid != -1 && params.size() > 0 )
			{
				IntOctets &data = *(params.begin());
			
				std::string str((const char*)(data.m_octets.begin()), data.m_octets.size());
				if( params.size() == 1 )
					PunishManager::GetInstance()->RunPunisher(new KickPunisher(acq.roleid, true, data.m_int, str));
				else if( params.size() == 2 )
					PunishManager::GetInstance()->RunPunisher(new KickPunisher(acq.roleid, false, 0 , str));
			}
		}
		else if( acq.query_type == AC_QUERY_LOGINOUT )
		{
			ACQLogInfo re;
			re.acq = acq;
			UserDataManager::GetInstance()->Visit(acq.roleid, &re);
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_MOUSEINFO )
		{
			ACQMouseInfo re;
			re.acq = acq;
			UserDataManager::GetInstance()->VisitOnline(acq.roleid, &re);
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_PATTERN )
		{
			ACQPatterns re;
			re.acq = acq;
			PatternVisitor pv;
			if( acq.roleid == -1 )
				pv.pattern_map = PatternManager::GetInstance()->GetPatterns();
			else
				UserDataManager::GetInstance()->VisitOnline(acq.roleid, &pv);
			for(PatternMap::const_iterator it = pv.pattern_map.begin(), ie = pv.pattern_map.end();
				it != ie; ++it)
			{
				const StackPattern &sp = (*it).first;
				const long long &cll = (*it).second;
				int t_count_ld = (int)(cll&0x00000000ffffffff);
				int t_count_hd = (int)((cll>>32)&0x00000000ffffffff);
				re.patterns.push_back(ACStackPattern(sp.m_uCaller, sp.m_uSize, sp.m_uPattern, 
					t_count_ld, t_count_hd));
			}
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_VERSION )
		{
			ACVersion re;
			re.version = ConfigManager::GetInstance()->GetVersion();
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_PATTERN_OWNER )
		{
			ACQStrOwner re;
			re.acq = acq;
			if( params.size() == 3 )
			{
				IntOctetsVector::iterator it = params.begin();
				int tcaller = (*it).m_int;
				++it;
				int tsize = (*it).m_int;
				++it;
				int tpattern = (*it).m_int;
				PatternOwnerVisitor sov;
				sov.sp = StackPattern(tcaller, tsize, tpattern);
				UserDataManager::GetInstance()->TravelOnline(&sov);
				for(std::vector<int>::const_iterator it = sov.v.begin(), ie = sov.v.end(); it != ie; ++it)
					re.owners.push_back(IntData((*it)));
		        ((GACControlServer*)manager)->SendProtocol(re, sid);
			}
		}
		else if( acq.query_type == AC_QUERY_IP_OWNER )
		{
			ACQStrOwner re;
			re.acq = acq;
			IntOctetsVector::iterator it = params.begin();
			if( it != params.end() )
			{
				IPOwnerVisitor sov;
				sov.ip = (*it).m_int;
				UserDataManager::GetInstance()->TravelOnline(&sov);
				for(std::vector<int>::const_iterator it = sov.v.begin(), ie = sov.v.end(); it != ie; ++it)
					re.owners.push_back(IntData((*it)));
		        ((GACControlServer*)manager)->SendProtocol(re, sid);
			}
		}
		else if( acq.query_type == AC_QUERY_STR_OWNER )
		{
			ACQStrOwner re;
			re.acq = acq;
			IntOctetsVector::iterator it = params.begin();
			if( it != params.end() )
			{
				StrOwnerVisitor sov;
				sov.type = (*it).m_int;
				sov.str = std::string((char*)((*it).m_octets.begin()), (*it).m_octets.size());
				UserDataManager::GetInstance()->TravelOnline(&sov);
				for(std::vector<int>::const_iterator it = sov.v.begin(), ie = sov.v.end(); it != ie; ++it)
					re.owners.push_back(IntData((*it)));
		        ((GACControlServer*)manager)->SendProtocol(re, sid);
			}
		}
		else if( acq.query_type == AC_QUERY_CODE )
		{
			QueryProxy::GetInstance()->SendQuery(acq, sid, params);
		}
		/*
		else if( acq.query_type == AC_QUERY_PATTERN )
		{
			StatManager::GetInstance()->QueryPatterns(acq.roleid);
		}
		else if( acq.query_type == AC_QUERY_PERIOD_INFO )
		{
			StatManager::GetInstance()->QueryPeriodInfo(acq.roleid);
		}
		else if( acq.query_type == AC_QUERY_FORBID_PROCESS )
		{
			ACForbidProcess re;
			re.forbid_process = StatManager::GetInstance()->QueryForbidProcess();
			((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		else if( acq.query_type == AC_QUERY_BRIEF_INFO )
		{
			ACBriefInfo re;
			re.brief_map = StackBriefInfo::GetBrief();
	        ((GACControlServer*)manager)->SendProtocol(re, sid);
		}
		*/
	}
};

};

#endif
