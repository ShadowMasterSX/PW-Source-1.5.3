/*
*filename:		spinlock.h 
*description:	copy from share/common/mutex.h 
*/
#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

class SpinLock
{
	volatile int locker;
	SpinLock(const SpinLock& rhs) { }
public:
	~SpinLock() { }
	explicit SpinLock() : locker(1) { }
	void Lock()
	{
		// XXX gcc 优化 BUG。会 coredump
		// 已确认版本 4.1.2 20070925 (Red Hat 4.1.2-33) 
		register int tmp;
		__asm__ __volatile__ (
			"1:		\n"
			"	cmp	$1, %0	\n"
			"	je	2f	\n"
			"	pause		\n"
			"	jmp	1b	\n"
			"2:		\n"
			"	xor	%1, %1	\n"
			"	xchg	%0, %1	\n"
			"	test	%1, %1	\n"
			"	je	1b	\n"
			: "=m"(locker), "=r"(tmp)
		);
	}
	void UNLock()
	{
		__asm__ __volatile__ (
			"	movl $1, %0		\n"
			: "=m"(locker)
		);
	}
	class Scoped
	{
		SpinLock *sl;
	public:
		~Scoped () { sl->UNLock(); }
		explicit Scoped(SpinLock& m) : sl(&m) { sl->Lock(); }
	};
};
#endif
