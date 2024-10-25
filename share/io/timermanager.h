#ifndef __TIMER_MANAGER_H
#define __TIMER_MANAGER_H

#include "map.h"

#include "conf.h"
#include "itimer.h"
#include "thread.h"
#include "binder.h"
#include "maperaser.h"

namespace GNET
{

	class TimerManager : public Protocol::Manager
	{
		typedef	gnet_map<Session::ID, AbstractBinder*>	BinderMap;
		BinderMap		bindermap;
		Thread::Mutex	locker;

		bool	compress;
		size_t	size_threshold;
		int		flush_count;

		bool SizePolicy(size_t size) { return 0 == size_threshold || size<size_threshold; }
		void FlushBuffer()
		{
			Thread::Mutex::Scoped l(locker);
			if( ++ flush_count == 1000 )
			{
				flush_count = 0;
				MapEraser<BinderMap> e(bindermap);
				for( BinderMap::iterator it = bindermap.begin(), ie = bindermap.end(); it != ie; ++it )
				{
					if( (*it).second->size() > 0 && Send( (*it).first, (*it).second) )
					{
						(*it).second->clear();
					}
					else
					{
						(*it).second->Destroy();
						e.push(it);
					}
				}
			}
			else
			{
				MapEraser<BinderMap> e(bindermap);
				for( BinderMap::iterator it = bindermap.begin(), ie = bindermap.end(); it != ie; ++it )
				{
					if( (*it).second->size() > 0 )
					{
						if( Send( (*it).first, (*it).second) )
						{
							(*it).second->clear();
						}
						else
						{
							(*it).second->Destroy();
							e.push(it);
						}
					}
				}
			}
		}

		class Timer : public IntervalTimer::Observer
		{
		public:
			TimerManager * manager;

			Timer(TimerManager * m) : manager(m) { }

			bool Update( )
			{
				manager->FlushBuffer();
				return true;
			}
		};
		TimerManager::Timer	timer;
	public:
		explicit TimerManager() : locker("TimerManager::locker"), compress(false), size_threshold(0), flush_count(0), timer(this)
		{
		}

		~TimerManager()
		{
			FlushBuffer();
		} 

		void SetCompress( bool b ) { compress = b; }

		void SetTimerSenderSize(size_t size_limit) { size_threshold = size_limit >=0 ? size_limit : 0; }

		void RunTimerSender( size_t ticks=1 )
		{
			IntervalTimer::Attach( &timer, ticks );
		}
		template<typename _Iterator, typename _Function>
		void AccumulateSend( _Iterator __first, _Iterator __last, const Protocol* p, _Function __f)
		{
			Octets oct = p->Encode();
			Thread::Mutex::Scoped l(locker);
			for (_Iterator _it=__first; _it!=__last; ++_it )
			{
				Protocol::Manager::Session::ID sid = __f(*_it);
				BinderMap::iterator it = bindermap.find(sid);
				if( bindermap.end() == it )
					it = bindermap.insert(std::make_pair(sid, 
							(AbstractBinder*) Protocol::Create( compress ? PROTOCOL_COMPRESSBINDER : PROTOCOL_BINDER ))).first;
				if (!SizePolicy((*it).second->size()+oct.size()+64))
				{
					Send( (*it).first, (*it).second);
					(*it).second->clear();
				}
				(*it).second->bind(oct);
			}
		}
		template<typename _Iterator, typename _Function>
		void AccumulateSend( _Iterator __first, _Iterator __last, const Protocol& p, _Function __f)
		{
			return AccumulateSend(__first,__last,&p,__f);
		}
		bool AccumulateSend( Protocol::Manager::Session::ID sid,const Protocol* p )
		{
			Octets oct = p->Encode();

			Thread::Mutex::Scoped l(locker);
			BinderMap::iterator it = bindermap.find(sid);

			if( bindermap.end() == it )
				it = bindermap.insert(std::make_pair(sid,
							(AbstractBinder*) Protocol::Create( compress ? PROTOCOL_COMPRESSBINDER : PROTOCOL_BINDER ))).first;
			if (!SizePolicy((*it).second->size() + oct.size() + 64))
			{
				Send( (*it).first, (*it).second);
				(*it).second->clear();
			}
			(*it).second->bind(oct);
			return true;
		}

		bool AccumulateSend( Protocol::Manager::Session::ID sid,const Protocol & p )
		{
			return AccumulateSend( sid, &p );
		}

		bool CompressSend( Protocol::Manager::Session::ID sid,const Protocol * p )
		{
			if( compress )
			{
				CompressBinder binder;
				binder.bind( p );
				return Send( sid, &binder );
			}
			else
				return Send( sid, p );
		}

		bool CompressSend( Protocol::Manager::Session::ID sid,const Protocol & p )
		{
			return CompressSend(sid, &p);
		}
	};

};
#endif
