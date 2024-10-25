/*
 * implementing bench mark.
 */

#ifndef __GNET_BENCHMARK_H__
#define __GNET_BENCHMARK_H__

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <iterator>

namespace GNET
{

template<typename T>
class TickData
{
private:
	size_t maxsize;
	size_t curpos;
	std::vector<T> m_ticks;

public:
	TickData(size_t size=16) : maxsize(size), curpos(0), m_ticks(size,T(0))
	{ }

	void clear( )
	{
		m_ticks.clear();
	}
  
	void tick( T span )
	{
		m_ticks[curpos] = span;
		curpos = (curpos+1) % maxsize;
	}

	TickData & operator = ( T span )
	{
		m_ticks[curpos] = span;
		return *this;
	}

	TickData & operator ++ ( )
	{
		curpos = (curpos+1) % maxsize;
		return *this;
	}

	void outputstat( std::ostream & out )
	{
		out << std::endl;
		out << "    The Latest " << m_ticks.size() << " ticks:" << std::endl;
		out << "\t";
		copy(m_ticks.begin(), m_ticks.end(), std::ostream_iterator<T>(out," "));
		out << std::endl;

		if( m_ticks.size() > 0 )
		{
			out << "\tMAX: " << *std::max_element(m_ticks.begin(), m_ticks.end());
			out << "  MIN: " << *std::min_element(m_ticks.begin(), m_ticks.end());
			out << "  AVG: " << (double)std::accumulate(m_ticks.begin(),
				m_ticks.end(), 0) / m_ticks.size();
			out << std::endl;
		}
		out.flush();
	}
};

class Benchmark
{
private:
	// time intervals
	struct timezone m_tz;
	struct timeval  m_tv_begin;
	struct timeval  m_tv_last;

	// tick
	TickData<u_int32_t> m_ticks;

	// historical
	u_int32_t m_tick_max;
	u_int32_t m_tick_min;
	u_int32_t m_tick_last;
	u_int32_t m_tick_count;
	u_int64_t m_tick_sum;

public:

	Benchmark() : m_tick_max(0), m_tick_min(0), m_tick_last(0),
		m_tick_count(0), m_tick_sum(0) 
	{
		m_tz.tz_minuteswest = 0;
		m_tz.tz_dsttime = 8;
		gettimeofday( &m_tv_begin, &m_tz );
		m_tv_last = m_tv_begin;
	}

	virtual ~Benchmark() { }

	void restart( )
	{
		m_tick_max = 0;
		m_tick_min = 0;
		m_tick_last = 0;
		m_tick_count = 0;
		m_tick_sum = 0;

		m_ticks.clear();

		gettimeofday( &m_tv_begin, &m_tz );
		m_tv_last = m_tv_begin;
	}

	// time interval functions
	inline int tickbegin( )
	{
		return gettimeofday( &m_tv_last, &m_tz );
	}

	inline u_int32_t tickend( )
	{
		struct timeval tv_now;
		if( 0 == gettimeofday( &tv_now, &m_tz ) )
		{
			// get values
			u_int32_t span = 0;
			span = ((long)tv_now.tv_usec - (long)m_tv_last.tv_usec);
			if( tv_now.tv_sec - m_tv_last.tv_sec > 0 )
				span += (tv_now.tv_sec - m_tv_last.tv_sec)*1000000;

			// save values
			++m_ticks = span;

			if( span > m_tick_max )
				m_tick_max = span;
			if( span < m_tick_min || 0 == m_tick_min )
				m_tick_min = span;
			m_tick_last = span;
			m_tick_count ++;
			m_tick_sum += span;
			return span;
		}
		return 0;
	}

	u_int64_t wholetime()
	{
		struct timeval tv_now;
		if( 0 == gettimeofday( &tv_now, &m_tz ) )
		{
			u_int64_t span = 0;
			span = ((long)tv_now.tv_usec - (long)m_tv_begin.tv_usec);
			span += (tv_now.tv_sec - m_tv_begin.tv_sec)*1000000;
			return span;
		}
		return 0;
	}

	void outputtickstat(std::ostream &out)
	{
		m_ticks.outputstat( out );
	}

	void outputtickvalues(std::ostream &out)
	{
		out << std::endl;
		out << "    All of the " << m_tick_count << " ticks:" << std::endl;
		out << "\tMAX: " << m_tick_max << "  MIN: " << m_tick_min
			<< "  LAST: " << m_tick_last << "  SUM: " << m_tick_sum;
		out << std::endl;
		out.flush();
	}

	void outputusage(std::ostream &out)
	{
		struct rusage usage;
		if( 0 == getrusage(RUSAGE_SELF, &usage) )
		{
			out << std::endl;
			out << "\tUser time used: "
			    << usage.ru_utime.tv_sec + 1e-6*usage.ru_utime.tv_usec << " seconds." << std::endl;;
			out << "\tSystem time used: "
			    << usage.ru_stime.tv_sec + 1e-6*usage.ru_stime.tv_usec << " seconds." << std::endl;;
			out << "\tMaximum resident set size: " << usage.ru_maxrss << "." << std::endl;
			out << "\tIntegral shared memory size: " << usage.ru_ixrss << "." << std::endl;
			out << "\tIntegral unshared data size: " << usage.ru_idrss << "." << std::endl;
			out << "\tIntegral unshared stack size: " << usage.ru_isrss << "." << std::endl;
			out << "\tPage reclaims: " << usage.ru_minflt << "." << std::endl; 
			out << "\tPage faults: " << usage.ru_majflt << "." << std::endl;
			out << "\tSwaps: " << usage.ru_nswap << "." << std::endl;
			out << "\tBlock input operations: " << usage.ru_inblock << "." << std::endl;
			out << "\tBlock output operations: " << usage.ru_oublock << "." << std::endl;
			out << "\tMessage sent: " << usage.ru_msgsnd << "." << std::endl;
			out << "\tMessages received: " << usage.ru_msgrcv << "." << std::endl;
			out << "\tSignals received: " << usage.ru_nsignals << "." << std::endl;
			out << "\tVoluntary context switches: " << usage.ru_nvcsw << "." << std::endl;
			out << "\tInvoluntary context switches: " << usage.ru_nivcsw << "." << std::endl;
			out << std::endl;
			out.flush();
		}
	}

	void output(std::ostream &out=std::cout )
	{
		out << std::endl;
		out << "    The whole time is: " << wholetime() << " us." << std::endl;
		out.flush();
		outputtickstat( out );
		outputtickvalues( out );
		outputusage( out );
	}
};
}

#endif


