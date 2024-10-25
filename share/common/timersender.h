#ifndef __TIMER_SENDER_H
#define __TIMER_SENDER_H

#include "protocol.h"
#include "itimer.h"
#include "thread.h"
#include <deque>
namespace GNET
{
	class TimerSender : public IntervalTimer::Observer
	{
		struct Protocol_Pack
		{
			unsigned int	sid;
			Protocol*		p;
			Protocol_Pack(unsigned int _sid,Protocol* _p) : sid(_sid),p(_p) {}
		};
		enum {BUF_SEND=0, BUF_DISCARD=1};
		Protocol::Manager* manager;
		std::deque<Protocol_Pack> protocol_deque;
		//std::map<unsigned int,ProtocolBinder> binderMap;
		Thread::Mutex  locker_protocol;
		size_t tick_time;
		size_t size_threshold;
		size_t buf_size;
		
		bool SizePolicy(size_t size) { return size_threshold!=0 && size>=size_threshold;}
		void FlushBuffer()
		{
			
			while (manager!=NULL && protocol_deque.size() != 0)
			{
				manager->Send(protocol_deque.front().sid,protocol_deque.front().p);
				protocol_deque.front().p->Destroy();
				protocol_deque.pop_front();
			}
			/*
			while (NULL!=manager && protocol_deque.size())
			{
				binderMap[protocol_deque.front().sid].bind(protocol_deque.front().p);
				protocol_deque.front().p->Destroy();	
				protocol_deque.pop_front();
			}	
			if (binderMap.size())
			{
				std::map<unsigned int,ProtocolBinder>::iterator it=binderMap.begin();
				for(;it!=binderMap.end();it++)
					manager->Send((*it).first,(*it).second);
				binderMap.clear();
			}
			
			if (NULL!=manager && binderMap.size())
			{
				std::map<unsigned int,ProtocolBinder>::iterator it=binderMap.begin();
				for(;it!=binderMap.end();it++)
					manager->Send((*it).first,(*it).second);
			}
			binderMap.clear();
			
			buf_size=0;
			*/
			PollIO::WakeUp();
		}
	public:
		explicit TimerSender(Protocol::Manager* m): manager(m),
													locker_protocol("TimerSender::locker_protocol"),
													tick_time(1),size_threshold(0),buf_size(0)
		{
		}
		~TimerSender()
		{
			Thread::Mutex::Scoped l(locker_protocol);
			FlushBuffer();
		} 
		void SetSizeLimit(size_t size_limit)
		{
			size_threshold=size_limit >=0 ? size_limit : 0;
		}
		void Run(size_t ticks=1)
		{
			static bool Running=false;
			if (!Running)
			{
				if (ticks>=1) tick_time=ticks;
				IntervalTimer::Attach(this,tick_time);
				Running=true;
			}
		}
		bool Update()	
		{
			Thread::Mutex::Scoped l(locker_protocol);
			FlushBuffer();
			return true;
		}
		void SendProtocol(Protocol::Manager::Session::ID sid,Protocol* p)
		{
			Thread::Mutex::Scoped l(locker_protocol);
			protocol_deque.push_back(Protocol_Pack(sid,p));
			if (SizePolicy(protocol_deque.size()))
			{
				FlushBuffer();
			}
			/*
			binderMap[sid].bind(p);
			
			if (SizePolicy(buf_size++))
			{
				FlushBuffer();
			}
			*/
		}
	};
};
#endif
