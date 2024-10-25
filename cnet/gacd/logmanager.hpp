#ifndef __GNET_LOGMANAGER_HPP
#define __GNET_LOGMANAGER_HPP

#include <map>
#include <vector>
#include <list>
#include <ctime>
#include "cheater.hpp"
#include <cstddef>
namespace GNET
{

class LogManager
{
	typedef std::pair<int, std::pair<int, time_t> > CheatLog;
    typedef std::map<int, std::vector<CheatLog> > CheatLogMap;

	struct LogItem
	{
		int m_iUserID;
		int m_iSubID;
		time_t m_time;
		LogItem() : m_iUserID(0), m_iSubID(0), m_time(0) { }
		LogItem(int iUserID, int iSubID) : m_iUserID(iUserID), m_iSubID(iSubID) { m_time = time(NULL); }
	};
	typedef std::vector<LogItem> LogVector;
	typedef std::list<LogVector> LogList;
	typedef std::map<int, LogList> LogMap;
	LogMap m_logs;


	static LogManager s_instance;
public:
	~LogManager() { }

	static LogManager *GetInstance() { return &s_instance; }

	void QueryCheaters(CheatLogMap& logs);

	template<class T>
	void Log(const T &log)
	{
		LogList &log_list = m_logs[log.GetLogType()];
		LogItem log_item(log.GetUserID(), log.GetSubID());
		if( !log_list.empty() && log_list.back().size() < log.GetMaxSize() )
			log_list.back().push_back(log_item);
		else
		{
			LogVector log_vector;
			log_vector.push_back(log_item);
			log_list.push_back(log_vector);
			if( log_list.size() > log.GetMaxCount() )
				log_list.pop_front();
		}
	}

private:
	LogManager() { }
	LogManager(const LogManager &lm);
	LogManager& operator=(const LogManager &lm);
	bool operator==(const LogManager &lm) const;
};

};

#endif
