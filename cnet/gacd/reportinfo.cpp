#include "gacdutil.h"
#include "reportinfo.hpp"
#include "patternmanager.hpp"
#include "userdatamanager.hpp"
#include "compress.h"

namespace GNET
{

//Thread::Mutex StackBriefInfo::locker("reportinfo locker");
//std::map<int, std::pair<int, std::pair<size_t, int> > > StackBriefInfo::map;

void ReportInfo::operator() (UserSessionData *data)
{
	if( m_cInfoType != INFO_APILOADER )
	data->Log(1, m_cInfoType);
	VisitData(data);
}

void ReportInfo::VisitData(UserSessionData *data)
{
}
/*
int StackBriefInfo::brief(int caller, int pattern, size_t size )
{
    int p = ((caller << 12) | size) ^ pattern;
	{
		Thread::Mutex::Scoped l(locker);
		if( map.find(p) == map.end() )
		{
			map[p] = std::make_pair(caller, std::make_pair(size, pattern));
		}
	}
	return p;
}
*/
void ReportInfo::DeliverReport(int iUserID, Octets &oss)
{
    ReportInfo *pInfo = NULL;
	bool bBad = ( oss.size() == 0 );
	if( !bBad ) {
    try
    {
		//DEBUG_PRINT_CLIENTINFO("report size %d,\t", oss.size());
        Octets ossrc;
        Uncompress(oss,ossrc);
		//DEBUG_PRINT_CLIENTINFO("after uncompress %d\n", ossrc.size());
        Marshal::OctetsStream os(ossrc);
        char cInfoType;
        os >> cInfoType;

		DEBUG_PRINT_CLIENTINFO("infotype %d, roleid %d\n", cInfoType, iUserID);
        switch( cInfoType )
        {
            case INFO_STACK :
                //info = new StackInfo();
                //break;
     //       case INFO_STACKBRIEF :
                //info = new StackBriefInfo();
				pInfo = new StackPatternInfo();
                break;
            case INFO_MOUSE :
                pInfo = new MouseInfo();
                break;
            case INFO_MEMORY :
                pInfo = new MemoryInfo();
                break;
            case INFO_PROCESSTIMES :
                pInfo = new ProcessTimesInfo();
                break;
            case INFO_THREADSTIMES :
                pInfo = new ThreadTimesInfo();
                break;
            case INFO_PLATFORMVERSION :
                pInfo = new PlatformInfo();
                break;
            case INFO_APILOADER :
                pInfo = new APIResInfo();
                break;
            case INFO_MODULES :
				pInfo = new ModuleInfo();
				break;
            case INFO_PROCESSLIST :
            case INFO_WINDOWLIST :
                pInfo = new StringInfo();
                break;
            default :
				bBad = true;
                break;
        }
		if( !bBad && pInfo != NULL )
		{
    	    pInfo->m_cInfoType = cInfoType;
	        pInfo->Decode(os);
		}
		if( !os.eos() )
			bBad = true;
    }
    catch( ... )
    {
		DEBUG_PRINT_CLIENTINFO("create err\n");
		bBad = true;
    } }

	if( bBad && pInfo != NULL )
	{
		delete pInfo;
		pInfo = NULL;
	}

	if( pInfo == NULL )
	{
		pInfo = new ReportInfo();
	}

	if( pInfo != NULL )
	{
    	pInfo->m_iUserID = iUserID;
		UserDataManager::GetInstance()->VisitOnline(iUserID, pInfo);
    	delete pInfo;
	}
}
/*
void StackBriefInfo::VisitData(UserSessionData *data)
{
	//DEBUG_PRINT_CLIENTINFO("StackBriefInfo::VisitData()\n");
}
*/
void StackPatternInfo::VisitData(UserSessionData *data)
{
	PatternManager::GetInstance()->AppendPattern(pattern_map);
	data->AppendPattern(pattern_map);
}

void PlatformInfo::VisitData(UserSessionData *data)
{
	data->SetPlatform(platform);
	data->SetCPUInfo(cpuinfo);
	data->SetMemInfo(meminfo);
	data->SetAdapterInfo(adapterinfo);
}

void APIResInfo::VisitData(UserSessionData *data)
{
	DEBUG_PRINT_CLIENTINFO("got api res\n");
	for(std::vector<std::pair<short,int > >::const_iterator it = apiRes.begin(), ie = apiRes.end();
		it != ie; ++it)
	{
		DEBUG_PRINT_CLIENTINFO("before check res code %d, res %d\n", (*it).first, (*it).second);
		data->CheckCodeRes((*it).first, (*it).second);	
	}
}

void StackPatternInfo::Decode(Marshal::OctetsStream &os)
{
	size_t size = 0;
	os >> CompactUINT(size);

	//if( m_cInfoType == INFO_STACK )
	//{
	//	m_cInfoType = INFO_STACKBRIEF;
	//	size_t tuint2 = 0;
	//	int tint2 = 0;
	//	int tint = 0;
	//	size_t tuint = 0;

		unsigned int uCount;

		for(size_t i = 0; i<size; ++i)
		{
			StackPattern sp;
			os >> sp.m_uCaller;
			os >> CompactUINT(uCount);
			os >> CompactUINT(sp.m_uSize);
			os >> sp.m_uPattern;

			DEBUG_PRINT_CLIENTINFO("pattern caller %x, count %d, size %d, pattern %x\n", 
				sp.m_uCaller, uCount, sp.m_uSize, sp.m_uPattern);
			pattern_map[sp] += uCount;
		}
	//}
	/*
	else
	{
		for(size_t i = 0; i<size; ++i)
		{
			os >> tint;
			os >> CompactUINT(tuint);
			pattern_map[StackPattern(tint)] += tuint;
		}
	}
	*/
}

/*
void StackBriefInfo::Decode(Marshal::OctetsStream &os)
{
	size_t size = 0;
	int tint = 0;
	size_t tuint = 0;
	os >> CompactUINT(size);

	if( m_cInfoType == INFO_STACK )
	{
		m_cInfoType = INFO_STACKBRIEF;
		size_t tuint2 = 0;
		int tint2 = 0;
		for(size_t i = 0; i<size; ++i)
		{
			os >> tint;
			os >> CompactUINT(tuint);
			os >> CompactUINT(tuint2);
			os >> tint2;
			//DEBUG_PRINT_CLIENTINFO("pattern caller %x, count %d, size %d, pattern %x\n", tint, tuint, tuint2, tint2);
			patterns[brief(tint, tint2, tuint2)] += tuint;
		}
	}
	else
	{
		for(size_t i = 0; i<size; ++i)
		{
			os >> tint;
			os >> CompactUINT(tuint);
			patterns[tint] += tuint;
		}
	}
}
*/
void PlatformInfo::Decode(Marshal::OctetsStream &os)
{
	os >> CompactUINT(platform.id);
    os >> CompactUINT(platform.majorversion);
    os >> CompactUINT(platform.minorversion);
    os >> CompactUINT(platform.buildnumber);
	if( ! (os.eos()) )
	{
		unsigned int tct;
		os >> tct;
		meminfo.SetCT(tct);
		os >> cpuinfo.arch;
		os >> cpuinfo.level;
		os >> tct;
		cpuinfo.SetCT(tct);
		Octets tos;
		os >> tos;
		adapterinfo.SetDes(std::string((char*)tos.begin(), tos.size()));
	}
}

void MouseInfo::VisitData(UserSessionData *data)
{
	data->AppendMouseInfo(mouse);
}

void MouseInfo::Decode(Marshal::OctetsStream &os)
{
	os >> CompactUINT(mouse.lbuttondownfreq);
    os >> CompactUINT(mouse.rbuttondownfreq);
    os >> CompactUINT(mouse.mousemovefreq);
    os >> mouse.mousemovevelo;
	size_t i = 0;
    for( i=0; i<Mouse::RegionCount; i++ )
        os >> CompactUINT(mouse.g_counts[i]);
    for( i=0; i<Mouse::RegionCount; i++ )
        os >> CompactUINT(mouse.g_times[i]);
}

void APIResInfo::Decode(Marshal::OctetsStream &os)
{
	size_t size = 0;
	short tshort = 0;
	int tint = 0;
	os >> CompactUINT(size);
	for(size_t i = 0; i<size; ++i)
	{
		os >> tshort;
		os >> tint;
		apiRes.push_back(std::make_pair(tshort, tint));
	}
}

void ProcessTimesInfo::Decode(Marshal::OctetsStream &os)
{
	size_t ta, tk, tu;
	os >> CompactUINT(ta);
    os >> CompactUINT(tk);
    os >> CompactUINT(tu);
	process_time = ThreadTime(0, ta, tk, tu);
}

void ThreadTimesInfo::Decode(Marshal::OctetsStream &os)
{
	thread_times.clear();
	size_t size = 0, ti, ta, tk, tu;
	os >> CompactUINT(size);
	for(size_t i = 0; i<size/4; ++i)
	{
		os >> CompactUINT(ti);
		os >> CompactUINT(ta);
		os >> CompactUINT(tk);
		os >> CompactUINT(tu);
		thread_times.push_back(ThreadTime(ti, ta, tk, tu));
	}
}

void ProcessTimesInfo::VisitData(UserSessionData *data)
{
	data->SetProcessTime(process_time);
}

void ThreadTimesInfo::VisitData(UserSessionData *data)
{
	data->SetThreadTimes(thread_times);
}

void ModuleInfo::VisitData(UserSessionData *data)
{   
	data->UpdateModuleStr(more, less);
}

void StringInfo::VisitData(UserSessionData *data)
{   
	if( m_cInfoType == INFO_PROCESSLIST )
		data->UpdateProcessStr(more, less);
	else if( m_cInfoType == INFO_WINDOWLIST )
		data->UpdateWindowStr(more, less);
}

void ModuleInfo::Decode(Marshal::OctetsStream &os)
{
	size_t size = 0;
	size_t i = 0;
	std::string tstr;
	int tint;
	os >> CompactUINT(size);
	for(i = 0; i<size; ++i)
	{
		os >> tstr;
		os >> tint;
		less.push_back(std::pair<std::string, int>(tstr, tint));
	}
	os >> CompactUINT(size);
	for(i = 0; i<size; ++i)
	{
		os >> tstr;
		os >> tint;
		more.push_back(std::pair<std::string, int>(tstr, tint));
	}
}

void StringInfo::Decode(Marshal::OctetsStream &os)
{
	size_t size = 0;
	size_t i = 0;
	std::string tstr;
	os >> CompactUINT(size);
	for(i = 0; i<size; ++i)
	{
		os >> tstr;
		std::string::size_type last_sep = tstr.find_last_of("\\");
        if( last_sep != std::string::npos )
            tstr = std::string(tstr, last_sep+1, tstr.size());
		less.push_back(tstr);
	}
	os >> CompactUINT(size);
	for(i = 0; i<size; ++i)
	{
		os >> tstr;
		std::string::size_type last_sep = tstr.find_last_of("\\");
        if( last_sep != std::string::npos )
            tstr = std::string(tstr, last_sep+1, tstr.size());
		DEBUG_PRINT_CLIENTINFO("%s\n", tstr.c_str());
		more.push_back(tstr);
	}
}
/*

void ReportInfo::Uncompress( Octets & os_com, Octets & os_src )
{
	Marshal::OctetsStream os;
	os_com.swap( os );
	size_t len_src, len_com;
	os >> CompactUINT(len_src) >> Marshal::Begin >> CompactUINT(len_com);
	os >> Marshal::Rollback;
	os >> os_com;

	if( len_com < len_src )
	{
		((Octets&)os_src).reserve( len_src );
		bool success = ( (len_src<=8192 && 0 == mppc::uncompress((unsigned char*)os_src.begin(),(int*)&len_src,
						(const unsigned char*)os_com.begin(),os_com.size()))
			|| (len_src>8192 && 0 == mppc::uncompress2((unsigned char*)os_src.begin(),(int*)&len_src,
								(const unsigned char*)os_com.begin(),os_com.size())) );
		os_src.resize( len_src );
		if( !success )
			os_src.swap( os_com );
	}
	else
	{
		os_src.swap( os_com );
	}
}
*/

}


