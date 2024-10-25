/*
*filename:		ASSERT.h 
*description:	copy from cgame/include/ASSERT.h 
*/
#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <stdio.h>

inline int * ASSERT_FAIL(const char * msg, const char *filename , int line)
{
	fprintf(stderr, "%s %s %d\n", msg, filename, line);
	return NULL;
}

#ifdef  NDEBUG
#define ASSERT(expr)           ( (void)(0))
#else
#define ASSERT(expr)	(void)((expr)?1:(*ASSERT_FAIL(#expr,__FILE__,__LINE__) = 0)) 
#endif

#endif

