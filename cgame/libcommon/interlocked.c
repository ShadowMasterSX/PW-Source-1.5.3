#include "interlocked.h"

#if defined(interlocked_increment)
#undef interlocked_increment
#undef interlocked_decrement
#endif

#if  defined( WIN32 ) || defined(WIN64)
int interlocked_increment(int *)
{
	__asm { 
		
	}
}

int interlocked_decrement(int *)
{
	
}
#else

int interlocked_increment(int *value)
{
	int ret;
	__asm__ __volatile__(
		"lock xadd %0,%1"
		:"=r"(ret),"=m"(*value)
		:"0"(1),"m"(*value)
		:"memory");
	return ret + 1;
}

int interlocked_decrement(int * value)
{
	int ret;
	// a just test asm __asm__ __volatile__("incl %0" : "=rm"(ret):"0"(10):"memory");

	__asm__ __volatile__(
		"lock xadd %0,%1"
		:"=r"(ret),"=m"(*value)
		:"0"(-1),"m"(*value)
		:"memory");
	return ret -1 ;
}

int interlocked_add(int * value,int addval)
{
	int ret;
	__asm__ __volatile__( \
		"lock xadd %0,%1" \
		:"=r"(ret),"=m"(*value) \
		:"0"(addval),"m"(*value) \
		:"memory");
	return ret-1;
}

int interlocked_sub(int * value,int subval)
{
	int ret;
	__asm__ __volatile__( \
		"lock xadd %0,%1" \
		:"=r"(ret),"=m"(*value) \
		:"0"(-subval),"m"(*value) \
		:"memory");
	return ret-1;
}
/*
#define xchg(m, in, out) \
asm("xchg %2,%0" : "=g" (*(m)), "=r" (out) : "1" (in))
int
bar(void *m, int x,int p)
{
	xchg((char *)m, (char)x, p);
	xchg((short *)m, (short)x, p);
	xchg((int *)m, (int)x, p);
	return x;
}

*/

#endif

