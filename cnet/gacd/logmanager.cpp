
#include "logmanager.hpp"

namespace GNET
{

LogManager LogManager::s_instance;

void LogManager::QueryCheaters(CheatLogMap &logs)
{
	std::vector<CheatLog> vcl;
	for(LogMap::const_iterator it = m_logs.begin(), ie = m_logs.end(); it != ie; ++it)
	{
		vcl.clear();
		const LogList &list = (*it).second;
		for(LogList::const_iterator it2 = list.begin(), ie2 = list.end(); it2 != ie2; ++it2)
		{
			const LogVector &vector = (*it2);
			for(LogVector::const_iterator it3 = vector.begin(), ie3 = vector.end(); it3 != ie3; ++it3)
			{
				const LogItem &log = (*it3);	
				
				vcl.push_back(std::make_pair(log.m_iUserID, 
					//std::make_pair<int, time_t>(log.m_iSubID, log.m_time)));
					std::make_pair(log.m_iSubID, log.m_time)));
			}
		}
		logs.insert(std::make_pair((*it).first, vcl));
	}
    m_logs.clear();
}

};

