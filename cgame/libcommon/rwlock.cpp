#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "ASSERT.h"

#include "spinlock.h"
#include "rwlock.h"

struct rwlock_t 
{
	//two lock
	int spinlock;
	int wr_lock;

	//member
	int wait_rd;
	int wait_wr;
	int count_rd;
	sem_t sem_rd;
	sem_t sem_wr;
};

rwlock * rwlock_init()
{
	rwlock * rwl = (rwlock *)calloc(1,sizeof(rwlock));
	sem_init(&rwl->sem_rd,0,0);
	sem_init(&rwl->sem_wr,0,0);
	return rwl;
}

void	rwlock_finalize(rwlock * rwl)
{
	sem_destroy(&rwl->sem_rd);
	sem_destroy(&rwl->sem_wr);
	free(rwl);
}

int	 rwlock_lockread(rwlock *rwl)
{
	int iswait = 0;
mutex_spinlock(&rwl->spinlock);
	iswait = (rwl->wait_wr || rwl->count_rd < 0);
	if(iswait){
		rwl->wait_rd ++;
	}
	else
	{
		rwl->count_rd ++;
	}
mutex_spinunlock(&rwl->spinlock);
	if(iswait) {
		sem_wait(&rwl->sem_rd);
	}
	return 0;
}

int	 rwlock_lockwrite(rwlock *rwl)
{
	int iswait = 0;
mutex_spinlock(&rwl->spinlock);
	iswait = (rwl->count_rd != 0);
	if(iswait){
		rwl->wait_wr ++;
	}
	else
	{
		rwl->count_rd = -1; 
	}
mutex_spinunlock(&rwl->spinlock);
	if(iswait) { 
		sem_wait(&rwl->sem_wr);
	}
	return 0;
}

int	 rwlock_unlock(rwlock *rwl)
{
	sem_t *sem= NULL;
	int count = 1; 
	ASSERT(rwl->count_rd);
mutex_spinlock(&rwl->spinlock);
	if(rwl->count_rd > 0){
		rwl->count_rd --;
	}
	else
	{
		rwl->count_rd ++;
	}

	if(rwl->count_rd == 0){
		if(rwl->wait_wr > 0){
			rwl->count_rd = -1;
			rwl->wait_wr --;
			sem = &rwl->sem_wr;
		} 
		else if (rwl->wait_rd > 0){
			rwl->count_rd = rwl->wait_rd;
			rwl->wait_rd  = 0;
			sem = &rwl->sem_rd;
			count = rwl->count_rd;
		}
	}
mutex_spinunlock(&rwl->spinlock);
	if(sem){
		while(count--)
		{
			sem_post(sem);
		}
	}
	return 0;
}


