/*
	互斥对象封装头文件，封装了Mutex和RWLock结构
	作者：崔铭
	公司：完美时空
	日期：2009-06-08
*/

#ifndef __ONLINEGMAE_COMMON_MUTEX_H__
#define __ONLINEGMAE_COMMON_MUTEX_H__

#include "spinlock.h"
#include "rwlock.h"

namespace GNET
{
	class Mutex
	{
		int lock;
	public:
		Mutex(const char *):lock(0) {}
		Mutex():lock(0) {}
		void Lock()   { mutex_spinlock2(&lock); }
		void Unlock() { mutex_spinunlock(&lock); }
		void UNLock(){Unlock();}
		class Scoped
		{
			Mutex *mx;
		public:
			~Scoped () { if(mx) mx->Unlock(); }
			explicit Scoped(Mutex& m) : mx(&m) { mx->Lock(); }
			void Detach() { mx = NULL; }
			void Unlock() { mx->Unlock();}
			void Lock() {mx->Lock();}
		};

		class SmartScoped
		{
			Mutex *mx;
			bool locked;
		public:
			~SmartScoped () { if(mx && locked) mx->Unlock(); }
			explicit SmartScoped(Mutex& m,bool auto_lock = true) : mx(&m), locked(auto_lock) { if(auto_lock) mx->Lock(); }
			void Detach() { mx = NULL; }
			void Unlock() { if(!locked) return; mx->Unlock();locked =false;}
			void Lock() { if(locked) return; mx->Lock(); locked = true;}
		};
	};

	class RWLock : private abase::RWLock
	{
	public:
		RWLock() {}
		RWLock(const char *) {}
		void WRLock() { WriteLock(); } 
		void RDLock() { ReadLock(); }
		void WRUnlock() { WriteUnlock(); } 
		void RDUnlock() { ReadUnlock(); }
		class WRScoped
		{
			RWLock *rw;
		public:
			~WRScoped () { rw->WRUnlock(); }
			explicit WRScoped(RWLock &l) : rw(&l) { rw->WRLock(); }
		};
		class RDScoped
		{
			RWLock *rw;
		public:
			~RDScoped () { rw->RDUnlock(); }
			explicit RDScoped(RWLock &l) : rw(&l) { rw->RDLock(); }
		};
	};

};

#endif

