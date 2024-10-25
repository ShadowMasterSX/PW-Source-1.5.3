/*
 * FILE: allocator.h
 *
 * DESCRIPTION: Buffered allocator
 *
 * CREATED BY: Cui Ming 2002-1-24
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include "spinlock.h"

#ifndef __ABASE_ALLOCATOR_H__
#define __ABASE_ALLOCATOR_H__

namespace abase
{

class default_alloc
{
public: 
	inline static void * allocate(size_t size) { 
		void * rst = ::operator new(size);
		//printf("++> %p\n",rst);
		return rst;
	}
	inline static void  deallocate(void * ptr,size_t size) 
	{ 
	//	printf("--> %p\n",ptr);
		return ::operator delete(ptr);
	}
	inline static void  deallocate(void * ptr) 
	{ 
	//	printf("--> %p\n",ptr);
		return ::operator delete(ptr);
	}
};

//#include "amemory.h"

}
#endif

