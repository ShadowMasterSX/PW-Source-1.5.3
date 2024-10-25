#ifndef __RW_LOCK_CM_H__
#define __RW_LOCK_CM_H__

#ifdef __cplusplus
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

