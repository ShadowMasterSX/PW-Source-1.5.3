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


class alloc_implement
{
public:
	virtual ~alloc_implement() {}
	virtual void * alloc(size_t size) = 0;
	virtual void  free(void * ptr, size_t size) = 0;
};

class default_alloc
{
public: 
	default_alloc(){}
	default_alloc(alloc_implement *) {}
	inline static void * allocate(size_t size) { 
		return malloc(size);
	}
	inline static void  deallocate(void * ptr,size_t size) 
	{
		::free(ptr);
	}
	inline static void  deallocate(void * ptr) 
	{ 
		::free(ptr);
	}

	inline void * alloc(size_t size)
	{
		return malloc(size);
	}

	inline void free(void * ptr, size_t size)
	{
		::free(ptr);
	}

	inline void free(void * ptr)
	{
		::free(ptr);
	}
	
};


}
#endif

