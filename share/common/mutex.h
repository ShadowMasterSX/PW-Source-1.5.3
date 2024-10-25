#ifndef __MUTEX_H
#define __MUTEX_H

#if defined DEADLOCK_DETECT
#include <pthread.h>
#include <sys/time.h>
#include <map>
#include <vector>
#include <stdio.h>
#include <errno.h>
#include <string>
namespace GNET
{
namespace Thread
{

	class Condition;
	
	class Mutex
	{
		friend class Condition;

		pthread_mutex_t mutex;
		std::string     identification;

		Mutex(const Mutex& rhs) { }
	public:
		bool isrecursive;
		~Mutex () 
		{
			while (pthread_mutex_destroy(&mutex) == EBUSY) { Lock(); UNLock(); }
		}
		explicit Mutex(bool recursive=false)
		{
			identification="";
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
			pthread_mutex_init(&mutex, &attr);
			pthread_mutexattr_destroy(&attr);
		}

		explicit Mutex(const char* id,bool recursive=false) : isrecursive(recursive)
		{
			identification=id;
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
			pthread_mutex_init(&mutex, &attr);
			pthread_mutexattr_destroy(&attr);
		}
		const std::string& Identification() const { return identification; }
		std::string Identification() { return identification; }

		void Lock()   { pthread_mutex_lock(&mutex);   }
		void UNLock() { pthread_mutex_unlock(&mutex); }

		class Detector
		{
			typedef std::vector<Mutex *> Vector;
			typedef std::map<pthread_t, Vector>  SNAPSHOT;
			typedef std::map<pthread_t, Mutex *> PRQ; //request graph
			typedef std::map<Mutex *, pthread_t> GRQ; //ownership graph
			SNAPSHOT snapshot;
			PRQ  prq;
			GRQ  grq;
			pthread_mutex_t locker;
			void DeadLock()
			{
				FILE *fp = fopen(DEADLOCK_DETECT, "w+");
				fprintf(fp, "[SNAPSHOT]\n");
				for ( SNAPSHOT::iterator it = snapshot.begin(), ie = snapshot.end(); it != ie; ++it )
				{
					fprintf(fp, "\tTID[%08x]\n", (*it).first);
					for ( Vector::iterator i = (*it).second.begin(), e = (*it).second.end(); i != e; ++i )
						fprintf(fp, "\t\tMutex[%p](%s)\n", *i,(*i)->Identification().c_str() );
				}
				fprintf(fp, "[PRQ]\n");
				for ( PRQ::iterator it = prq.begin(), ie = prq.end(); it != ie; ++it )
					fprintf(fp, "\tTID[%08x]:Mutex[%p](%s)\n", (*it).first, (*it).second,(*it).second->Identification().c_str() );
				fclose(fp);
				abort();
			}
		public:
			Detector() 
			{
				pthread_mutexattr_t attr;
				pthread_mutexattr_init(&attr);
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
				pthread_mutex_init(&locker, &attr);
				pthread_mutexattr_destroy(&attr);
			}
			void Lock( Mutex *mutex )
			{
				pthread_t tid = pthread_self();
				pthread_t r = tid;
				pthread_mutex_lock(&locker);
				prq.insert( std::make_pair( tid, mutex ) );
				PRQ::iterator it_prq;
				GRQ::iterator it_grq;
				if ( mutex->isrecursive )
				{
					it_grq = grq.find( mutex );
					if ( it_grq != grq.end() && (*it_grq).second == r )
						goto recursive;
				}
				while ( (it_prq = prq.find(r)) != prq.end() && (it_grq = grq.find((*it_prq).second)) != grq.end() )
					if ( (r = (*it_grq).second) == tid )
						DeadLock();
recursive:
				pthread_mutex_unlock(&locker);

				mutex->Lock();

				pthread_mutex_lock(&locker);
				prq.erase( tid );
				grq.insert( std::make_pair( mutex, tid ) );
				snapshot[tid].push_back( mutex );
				pthread_mutex_unlock(&locker);
			}
			
			void UNLock( Mutex *mutex )
			{
				mutex->UNLock();
				pthread_mutex_lock(&locker);
				grq.erase( mutex );
				snapshot[pthread_self()].pop_back();
				pthread_mutex_unlock(&locker);
			}
			static Detector& GetInstance() { static Detector tmp; return tmp; }
		};

		class Scoped
		{
			Mutex *mx;
		public:
			~Scoped () { Detector::GetInstance().UNLock(mx); }
			explicit Scoped(Mutex& m) : mx(&m) { Detector::GetInstance().Lock(mx); }
		};

		typedef Scoped RDScoped;
		typedef Scoped WRScoped;
	};

	typedef Mutex SpinLock;
	typedef Mutex RWLock;

	class Condition
	{
		pthread_cond_t cond;
		Condition(const Condition& rhs) { }
	public:
		~Condition () 
		{
			while (pthread_cond_destroy(&cond) == EBUSY) { NotifyAll(); }
		}
		explicit Condition( ) { pthread_cond_init( &cond, NULL ); }
		int Wait( Mutex & mutex ) { return pthread_cond_wait( &cond, &mutex.mutex ); }
		int TimedWait( Mutex & mutex, int nseconds )
		{
			if( nseconds >= 0 )
			{
				struct timeval now;
				struct timespec timeout;
				struct timezone tz;
				tz.tz_dsttime = 8;
				gettimeofday(&now, &tz);
				timeout.tv_sec = now.tv_sec + nseconds;
				timeout.tv_nsec = now.tv_usec * 1000;
				return pthread_cond_timedwait( &cond, &mutex.mutex, &timeout );
			}
			return pthread_cond_wait( &cond, &mutex.mutex );
		}
		int NotifyOne( ) { return pthread_cond_signal( &cond ); }
		int NotifyAll( ) { return pthread_cond_broadcast( &cond ); }
	};

};
};

#elif defined _REENTRANT_
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

namespace GNET
{

namespace Thread
{
	class Condition;

	class Mutex
	{
		friend class Condition;

		pthread_mutex_t mutex;
		Mutex(const Mutex& rhs) { }
	public:
		~Mutex () 
		{
			while (pthread_mutex_destroy(&mutex) == EBUSY) { Lock(); UNLock(); }
		}
		explicit Mutex(bool recursive=false)
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			//pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
			pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK);
			pthread_mutex_init(&mutex, &attr);
			pthread_mutexattr_destroy(&attr);
		}

		explicit Mutex(const char* id,bool recursive=false)
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			//pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
			pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK);
			pthread_mutex_init(&mutex, &attr);
			pthread_mutexattr_destroy(&attr);
		}

		void Lock() { 
			pthread_mutex_lock(&mutex); 
		}
		void UNLock() 
		{ 
			pthread_mutex_unlock(&mutex);
			//int rst = pthread_mutex_unlock(&mutex); //assert(!rst);
		}
		bool TryLock() { return pthread_mutex_trylock(&mutex) == 0; }
		class Scoped
		{
			Mutex *mx;
		public:
			~Scoped () { mx->UNLock(); }
			explicit Scoped(Mutex& m) : mx(&m) { mx->Lock(); }
		};
	};

#if defined __GNUC__
	class SpinLock
	{
#if (__GNUC__ == 4)
		pthread_spinlock_t spin;
		SpinLock(const SpinLock& rhs) { }
	public:
		~SpinLock() { pthread_spin_destroy(&spin); }
		explicit SpinLock(const char* id) { pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE); }
		void Lock() { pthread_spin_lock(&spin); }
		void UNLock() { pthread_spin_unlock(&spin); }
#else
		volatile int locker;
		SpinLock(const SpinLock& rhs) { }
	public:
		~SpinLock() { }
		explicit SpinLock(const char* id) : locker(1) { }
		void Lock()
		{
			// XXX gcc 优化 BUG。会 coredump
			// 已确认版本 4.1.2 20070925 (Red Hat 4.1.2-33) 
			register int tmp;
			__asm__ __volatile__ (
				"1:		\n"
				"	cmp	$1, %0	\n"
				"	je	2f	\n"
				"	pause		\n"
				"	jmp	1b	\n"
				"2:		\n"
				"	xor	%1, %1	\n"
				"	xchg	%0, %1	\n"
				"	test	%1, %1	\n"
				"	je	1b	\n"
				: "=m"(locker), "=r"(tmp)
			);
		}
		void UNLock()
		{
			__asm__ __volatile__ (
				"	movl $1, %0		\n"
				: "=m"(locker)
			);
		}
#endif
		class Scoped
		{
			SpinLock *sl;
		public:
			~Scoped () { sl->UNLock(); }
			explicit Scoped(SpinLock& m) : sl(&m) { sl->Lock(); }
		};
	};
#else
	typedef Mutex SpinLock;
#endif

	class RWLock
	{
		pthread_rwlock_t locker;
		RWLock(const RWLock& rhs) { }
	public:
		~RWLock()
		{
			while (pthread_rwlock_destroy(&locker) == EBUSY) { WRLock(); UNLock(); }
		}
		explicit RWLock() { pthread_rwlock_init(&locker, NULL); }
		explicit RWLock(const char* id) { pthread_rwlock_init(&locker, NULL); }
		void WRLock() { pthread_rwlock_wrlock(&locker); }
		void RDLock() { pthread_rwlock_rdlock(&locker); }
		void UNLock() { pthread_rwlock_unlock(&locker); }
		class WRScoped
		{
			RWLock *rw;
		public:
			~WRScoped () { rw->UNLock(); }
			explicit WRScoped(RWLock &l) : rw(&l) { rw->WRLock(); }
		};
		class RDScoped
		{
			RWLock *rw;
		public:
			~RDScoped () { rw->UNLock(); }
			explicit RDScoped(RWLock &l) : rw(&l) { rw->RDLock(); }
		};
	};

	class Condition
	{
		pthread_cond_t cond;
		Condition(const Condition& rhs) { }
	public:
		~Condition () 
		{
			while (pthread_cond_destroy(&cond) == EBUSY) { NotifyAll(); }
		}
		explicit Condition( ) { pthread_cond_init( &cond, NULL ); }
		int Wait( Mutex & mutex ) { return pthread_cond_wait( &cond, &mutex.mutex ); }
		int TimedWait( Mutex & mutex, int nseconds )
		{
			if( nseconds >= 0 )
			{
				struct timeval now;
				struct timespec timeout;
				struct timezone tz;
				tz.tz_dsttime = 8;
				gettimeofday(&now, &tz);
				timeout.tv_sec = now.tv_sec + nseconds;
				timeout.tv_nsec = now.tv_usec * 1000;
				return pthread_cond_timedwait( &cond, &mutex.mutex, &timeout );
			}
			return pthread_cond_wait( &cond, &mutex.mutex );
		}
		int NotifyOne( ) { return pthread_cond_signal( &cond ); }
		int NotifyAll( ) { return pthread_cond_broadcast( &cond ); }
	};

};

};

#else

namespace GNET
{

namespace Thread
{
	struct Mutex
	{
		~Mutex() { }
		explicit Mutex() {}
		explicit Mutex(const char* id,bool recursive=false) { }
		void Lock()   { }
		void UNLock() { }
		bool TryLock() { return true; }
		struct Scoped { ~Scoped() { } explicit Scoped(Mutex& m) { } };
	};

#if defined __GNUC__
	struct SpinLock
	{
		~SpinLock() { }
		explicit SpinLock(const char* id) { }
		void Lock()   { }
		void UNLock() { }
		struct Scoped { ~Scoped() { } explicit Scoped(SpinLock& m) { } };
	};
#else
	typedef Mutex SpinLock;
#endif

	struct RWLock
	{
		~RWLock() { }
		void WRLock() { }
		void RDLock() { }
		void UNLock() { }
		RWLock() { }
		RWLock(const char* id) { }
		struct WRScoped { ~WRScoped() { } WRScoped(RWLock &) { } };
		struct RDScoped { ~RDScoped() { } RDScoped(RWLock &) { } };
	};

	class Condition
	{
	public:
		~Condition () { } 
		explicit Condition( ) { }
		int Wait( Mutex & mutex ) { return 0; }
		int TimedWait( Mutex & mutex, int nseconds ) { return 0; }
		int NotifyOne( ) { return 0; }
		int NotifyAll( ) { return 0; }
	};
};

};
#endif

#endif
