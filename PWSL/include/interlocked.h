#ifndef __CM_INTERLOKED_OPERATION_H__
#define __CM_INTERLOKED_OPERATION_H__

#include <stdint.h>

int interlocked_increment(int *);
int interlocked_decrement(int *);
int interlocked_add(int *,int);
int interlocked_sub(int *,int);

uint32_t interlocked_increment(uint32_t *);
uint32_t interlocked_decrement(uint32_t *);

bool interlocked_CAS2(uint64_t * value, uint64_t oldvalue, uint64_t  newvalue);	//返回 true表示没有复制成功
bool interlocked_CAS(uint32_t * value, uint32_t oldvalue, uint32_t  newvalue);

inline bool interlocked_CAS2(int64_t * value, int64_t oldvalue, int64_t  newvalue) { return interlocked_CAS2((uint64_t*)value, (uint64_t)oldvalue, (uint64_t)newvalue);}
inline bool interlocked_CAS(int32_t * value, int32_t oldvalue, int32_t  newvalue) { return interlocked_CAS((uint32_t*)value, (uint32_t)oldvalue, (uint32_t)newvalue);}

int64_t interlocked_increment(int64_t *);
int64_t interlocked_decrement(int64_t *);
int64_t interlocked_add(int64_t *,int64_t);
int64_t interlocked_sub(int64_t *,int64_t);

inline uint64_t rdtsc(void)
{       
	uint64_t x;
	//__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	__asm__ volatile ("rdtsc" : "=A" (x));
	return x;
}                                


class InterlockedCounter
{
	int * _counter;
	int _offset;
	int _value;
public:
	InterlockedCounter(int * counter,int offset):_counter(counter),_offset(offset)
	{
		_value = interlocked_add(counter,offset);
	}

	InterlockedCounter(int & counter,int offset):_counter(&counter),_offset(offset)
	{
		_value = interlocked_add(&counter,offset);
	}
	
	~InterlockedCounter()
	{
		if(_counter) interlocked_sub(_counter,_offset);
	}
	
	void Detach()
	{
		_counter = 0;
	}
};

#endif

