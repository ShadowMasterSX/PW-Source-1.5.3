#ifndef __RW_LOCK_CM_H__
#define __RW_LOCK_CM_H__

#include "spinlock.h"
#include "interlocked.h"
#include <unistd.h>

namespace abase 
{
class RWLock
{
private:
	int lock;
	int counter;
private:
	inline void Lock() { mutex_spinlock2(&lock); }
	inline void Unlock() { mutex_spinunlock(&lock); }
	inline int Inc() { return interlocked_increment(&counter);}
	inline int Dec() { return interlocked_decrement(&counter);}

public:
	inline RWLock():lock(0),counter(0){}
	inline void ReadLock() { Lock(); Inc(); Unlock(); }
	inline void ReadUnlock() { Dec(); } 
	inline void WriteLock() { Lock(); while(counter){usleep(2);} }
	inline void WriteUnlock() {Unlock(); }
	inline void WriteLockToRead() { Inc(); Unlock(); }
	inline void ReadLockToWrite() { Dec(); WriteLock();}

public:
	class Keeper
	{
		RWLock & _ref;
		bool _lock_write;
		bool _lock_read;
		Keeper & operator=(Keeper & );
		public:
		Keeper(RWLock & ref):_ref(ref),_lock_write(false),_lock_read(false) {}
		~Keeper() { Unlock();}
		void LockWrite()
		{
			if(_lock_write)  return;
			if(_lock_read)
			{
				_ref.ReadLockToWrite();
				_lock_read = false;
			}
			else
			{
				_ref.WriteLock();
			}
			_lock_write = true;
		}

		void LockRead()
		{       
			if(_lock_read)  return;
			if(_lock_write)
			{       
				_ref.WriteLockToRead();
				_lock_write = false;
			}
			else
			{       
				_ref.ReadLock();
			}
			_lock_read = true;
		}

		void Unlock()
		{       
			if(_lock_write)
			{       
				_ref.WriteUnlock();
			}
			else if(_lock_read)
			{       
				_ref.ReadUnlock();
			}
			_lock_read = false;
			_lock_write = false;
		}
	};
};
}

#endif

