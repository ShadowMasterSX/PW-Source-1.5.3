#ifndef __GNET_REPORTINFO_HPP
#define __GNET_REPORTINFO_HPP

#include <vector>
#include <string>
#include <map>
#include <set>
#include "marshal.h"
#include "thread.h"
#include "acconstants.h"
#include "usersessiondata.hpp"
#include "platform.h"
#include "meminfo.h"
#include "cpuinfo.h"
#include "mouseinfo.h"
#include "threadtime.h"
#include "stackpattern.h"
#include "adapterinfo.h"

namespace GNET
{

struct ReportInfo
{
	enum InfoTypes { 
		INFO_NULL = 0,
		INFO_STACK = 1,					
		INFO_STACKBRIEF = 2,			
		INFO_MOUSE = 3,  				// TODO
		INFO_MEMORY = 4,				
		INFO_PROCESSTIMES = 5,			// TODO 
		INFO_MODULES = 6,				// 
		INFO_THREADSTIMES = 7,			// TODO
		INFO_PROCESSLIST = 8, 			// 
		INFO_WINDOWLIST = 9, 			// 
		INFO_APILOADER = 10,            // for reqeust, return immediately
		INFO_PLATFORMVERSION = 11,		// 
	};

	char m_cInfoType;
	int m_iUserID;

	ReportInfo() : m_cInfoType(INFO_NULL), m_iUserID(0) { }
	virtual ~ReportInfo() { }

	virtual void Decode(Marshal::OctetsStream &os) { };
	void operator() (UserSessionData *data);
	virtual void VisitData(UserSessionData *data);
	static void DeliverReport(int iUserID, Octets &oss);
	//static void Uncompress( Octets & os_com, Octets & os_src );
};

struct StackPatternInfo : public ReportInfo
{
	PatternMap pattern_map;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
};

/*

struct StackBriefInfo : public ReportInfo
{
	MapIUI patterns;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
	int brief(int caller, int pattern, size_t size );
	static Thread::Mutex locker;
	typedef std::map<int, std::pair<int, std::pair<size_t, int> > > BriefMap;
	static BriefMap map;
	static BriefMap GetBrief() { return map; }
};

*/

struct PlatformInfo : public ReportInfo
{
	Platform platform;
	CPUInfo cpuinfo;
	MemInfo meminfo;
	AdapterInfo adapterinfo;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
};

struct MouseInfo : public ReportInfo
{
	Mouse mouse;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
};

struct MemoryInfo : public ReportInfo
{
	void Decode(Marshal::OctetsStream &os) { }
};

struct APIResInfo : public ReportInfo
{
	std::vector<std::pair<short, int> > apiRes;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
};

struct ProcessTimesInfo : public ReportInfo
{
	ThreadTime process_time;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
};

struct ThreadTimesInfo : public ReportInfo
{
	std::vector<ThreadTime> thread_times;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
};

struct StringInfo : public ReportInfo
{
	std::vector<std::string> more;
	std::vector<std::string> less;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
};

struct ModuleInfo : public ReportInfo
{
	std::vector<std::pair<std::string, int> > more;
	std::vector<std::pair<std::string, int> > less;
	void Decode(Marshal::OctetsStream &os);
	void VisitData(UserSessionData *data);
};

	

};

#endif
