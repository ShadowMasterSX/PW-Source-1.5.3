#ifndef __ONLINE_GAME_SHARE_MEM_ALLOC_H__
#define __ONLINE_GAME_SHARE_MEM_ALLOC_H__


namespace abase
{
	void * shm_init(size_t max_size, int initial_size = 0);
	void * shm_alloc(void * ptr, size_t size);
	void shm_free(void * ptr, void * buf);
	void shm_dump(void * ptr,  FILE *);

}
#endif

