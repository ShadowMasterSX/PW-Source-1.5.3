#ifndef __ALLOC_H
#define __ALLOC_H

#include <sys/types.h>
#include <malloc.h>

#include "mutex.h"

namespace GNET
{

class DefaultMemoryAllocator
{
public:
	static void *alloc(size_t size) { return malloc(size); }
	static void free(void *p, size_t size) { ::free(p); }
};

template< int BOUNDARY, int BLOCKS, typename Allocator = DefaultMemoryAllocator >
class AlignedMemoryBlock
{
	void  *list;
	size_t size;
	AlignedMemoryBlock *prev;
	AlignedMemoryBlock *next;
	Thread::SpinLock locker;
	enum { MASK = ~(BOUNDARY - 1) };
public:
	AlignedMemoryBlock() : prev(this), next(this), locker("AlignMemoryBlock") { }
	void *alloc()
	{
		Thread::SpinLock::Scoped l(locker);
		for ( AlignedMemoryBlock *c = next; c != this; c = c->next )
			if ( void *p = c->list )
			{
				c->list = *(void **)p;
				c->size ++;
				return p;
			}
		AlignedMemoryBlock *c = (AlignedMemoryBlock *)Allocator().alloc(sizeof(AlignedMemoryBlock) + BOUNDARY * (BLOCKS + 1));
		c->prev       = this;
		c->next       = next;
		c->size       = 1;
		next->prev    = c;
		next          = c;
		void *q       = (void *)( ((ptrdiff_t)c + sizeof(AlignedMemoryBlock) + BOUNDARY - 1) & MASK );
		void *p       = c->list = (char *)q + BOUNDARY;
		for ( int i = BLOCKS - 1; --i; p = *(void **)p = (char *)p + BOUNDARY );
		*(void **)p = NULL;
		return q;
	}

	void free( void *p )
	{
		Thread::SpinLock::Scoped l(locker);
		for ( AlignedMemoryBlock *c = next; c != this; c = c->next )
			if ( p >= c && p < ((char *)c + sizeof(AlignedMemoryBlock) + BOUNDARY * (BLOCKS + 1) ) )
			{
				*(void **)p = c->list;
				c->list     = p;
				if ( ! --c->size )
				{
					c->next->prev = c->prev;
					c->prev->next = c->next;
					Allocator().free(c, sizeof(AlignedMemoryBlock) + BOUNDARY * (BLOCKS + 1) );
				}
				break;
			}
	}
};

};
#endif

