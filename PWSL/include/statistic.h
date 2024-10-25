/*
 		
		统计类实现 ，这个类只负责存储各种信息，保存这些信息的模块目前在Log模块中
***************************************************************************************************
		
	统计类的使用。统计信息会通过syslogd每隔5分钟自动保存到/var/log/statinfo文件中，
	并通过cricket在浏览器中显示，查看统计结果。

	如下所示，统计QQ的消息个数，注意"QQ.Msg"是该统计的标识，应该唯一。

	Statistic * pstat = GNET::Statistic::GetInstance(std::string("QQ.Msg"));

	可以将pstat保存下来，每次发送消息时，调用
		pstat->update( 1 ); // 1为本次发送的消息个数
		或者 GNET::Statistic::GetStatMin5(std::string("QQ.Msg"))->update(1);

 */

#ifndef __GNET_STATISTIC_H__
#define __GNET_STATISTIC_H__

#include <iostream>
#include <string>
#include <map>
#include <pthread.h>

#include "mutex.h"
#include "log.h"
#include "gnet_timer.h"
#ifndef _NOSTAT
#define STAT_MIN5(ds,value)	GNET::Statistic::GetStatMin5(std::string(ds))->update(value)
#define STAT_HOUR(ds,value)	GNET::Statistic::GetStatHour(std::string(ds))->update(value)
#define STAT_DAY(ds,value)	GNET::Statistic::GetStatDay(std::string(ds))->update(value)
#else
#define STAT_MIN5(ds,value)
#define STAT_HOUR(ds,value)
#define STAT_DAY(ds,value)
#endif

namespace GNET
{
	class Statistic
	{
	public:
		enum StatInterval
		{
			any = 0,
			min5  = 1,
			hour = 2,
			day  = 3
		};
	private:

		struct stringcasecmp
		{
			bool operator() (const std::string &x, const std::string &y) const
			{ return strcasecmp(x.c_str(), y.c_str()) < 0; }
		};
		
		typedef std::map<const std::string,Statistic,stringcasecmp> Map;
		static Map _map[day+1];
		static GNET::Mutex _mutex[day+1];
	

	public:
		int		m_interval;

		// stat data
		int64_t m_max;
		int64_t m_min;
		int64_t m_cur;
		int64_t m_cnt;
		int64_t m_sum;

		Statistic() : m_interval(min5), m_max(0), m_min(0), m_cur(0), m_cnt(0), m_sum(0) { }
		~Statistic() { }
		void reset( )
		{
			m_cur = 0;
			m_cnt = 0;
			m_sum = 0;
			m_max = 0;
			m_min = 0;
		}

		void update( int64_t __delta )
		{
			m_cur = __delta;
			m_cnt ++;
			m_sum += __delta;
			m_max = std::max((int64_t)__delta,m_max);
			m_min = (0 == m_min ? __delta : std::min((int64_t)__delta,m_min));
		}

public:

		static Statistic * GetStatMin5( const std::string& __name);
		static Statistic * GetStatHour( const std::string& __name);
		static Statistic * GetStatDay( const std::string& __name);
		static bool enumdefault( const std::string & __name, const Statistic * __pstat );
		static void enumerate( bool (*f)(const std::string &,const Statistic *), StatInterval __interval = any );
		static void resetall( StatInterval __interval = any );

	};

	class UpdateStatToLog  : public Timer::Observer		//定期刷新数据到日志中的类，需要手动的创建这个类， 会自动Attach到Timer上 
	{
		private:
			Timer   timer;
			long    t_hour_last;
			long    t_day_last;

		public:
			UpdateStatToLog() : t_hour_last(-1), t_day_last(-1) {Timer::Attach(this);}
			void Update()
			{
				if( timer.Elapse() >= 299 )
				{
					int seconds = Timer::GetTime() % 300;
					if( seconds < 180 || seconds > 210 )
						return;
					timer.Reset();

					// min5
					Log::StatEnumerate( Statistic::min5 );

					// hour
					time_t now = Timer::GetTime();
					if( t_hour_last < 0 || now - t_hour_last > 3000 )
					{
						struct tm * t = localtime( &now );
						if( t->tm_min >= 0 && t->tm_min <= 8 )
						{
							Log::StatEnumerate( Statistic::hour );
							Log::StatEnumerate( Statistic::day );
							t_hour_last = now;
						}
					}
				}
			}
			
	};
	
}

#endif // __GNET_STATISTIC_H__

