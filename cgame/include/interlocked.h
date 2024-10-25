#ifndef __CM_INTERLOKED_OPERATION_H__
#define __CM_INTERLOKED_OPERATION_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __INTERLOCKED_USE_INLINE__
inline int interlocked_increment(int * value)
{
	int ret;
	__asm__ __volatile__( \
		"lock xadd %0,%1" \
		:"=r"(ret),"=m"(*value) \
		:"0"(1),"m"(*value) \
		:"memory");
	return ret+1;
}

inline int interlocked_decrement(int * value)
{
	int ret;
	__asm__ __volatile__( \
		"lock xadd %0,%1" \
		:"=r"(ret),"=m"(*value) \
		:"0"(-1),"m"(*value) \
		:"memory");
	return ret-1;
}

inline int interlocked_add(int * value,int addval)
{
	int ret;
	__asm__ __volatile__( \
		"lock xadd %0,%1" \
		:"=r"(ret),"=m"(*value) \
		:"0"(addval),"m"(*value) \
		:"memory");
	return ret+addval;
}

inline int interlocked_sub(int * value,int subval)
{
	int ret;
	__asm__ __volatile__( \
		"lock xadd %0,%1" \
		:"=r"(ret),"=m"(*value) \
		:"0"(-subval),"m"(*value) \
		:"memory");
	return ret - subval;
}

#else
int interlocked_increment(int *);
int interlocked_decrement(int *);
int interlocked_add(int *,int);
int interlocked_sub(int *,int);
#endif

inline unsigned long long int rdtsc(void)
{       
	unsigned long long int x;
	//__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	__asm__ volatile ("rdtsc" : "=A" (x));
	return x;
}                                

#ifdef __cplusplus

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

}
#endif
#endif

