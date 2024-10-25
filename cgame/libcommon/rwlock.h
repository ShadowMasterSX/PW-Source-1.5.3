#ifndef __RW_LOCK_CM_H__
#define __RW_LOCK_CM_H__

#ifdef __cplusplus
//simple cplusplus implement of rwlock

#include "spinlock.h"
#include "interlocked.h"
#include <unistd.h>

class RWLock
{
private:
	int lock;
	int counter;
private:
	inline void Lock() { mutex_spinlock(&lock); }
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
		RWLock & _stub;
		bool _lock_write;
		bool _lock_read;
		Keeper & operator=(Keeper & );
		public:
		Keeper(RWLock & stub):_stub(stub),_lock_write(false),_lock_read(false) {}
		~Keeper() { Unlock();}
		void LockWrite()
		{
			if(_lock_write)  return;
			if(_lock_read)
			{
				_stub.ReadLockToWrite();
				_lock_read = false;
				return;
			}
			else
			{
				_stub.WriteLock();
			}
			_lock_write = true;
		}

		void LockRead()
		{       
			if(_lock_read)  return;
			if(_lock_write)
			{       
				_stub.WriteLockToRead();
				_lock_write = false;
				return;
			}
			else
			{       
				_stub.WriteLock();
			}
			_lock_read = true;
		}

		void Unlock()
		{       
			if(_lock_write)
			{       
				_stub.WriteUnlock();
			}
			else if(_lock_read)
			{       
				_stub.ReadUnlock();
			}
			_lock_read = false;
			_lock_write = false;
		}
	};
};


extern "C" {
#endif

typedef struct rwlock_t rwlock;

rwlock * rwlock_init();
void 	 rwlock_finalize(rwlock *);
int	 rwlock_lockread(rwlock *);
int	 rwlock_lockwrite(rwlock *);
int	 rwlock_unlock(rwlock *);
	
#ifdef __cplusplus
}
#endif

#endif

