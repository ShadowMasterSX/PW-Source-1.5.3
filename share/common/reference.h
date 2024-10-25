#ifndef __GNET_REFERENCE_H
#define __GNET_REFERENCE_H

#include "mutex.h"

namespace GNET
{

template<typename T> struct DefaultDeleter
{
	void operator() (T *t) const { delete t; }
};

struct NullDeleter
{
	void operator() (const void *) const { }
};

namespace reference_helper
{

template<typename T> struct traits            { typedef T&   reference; };
template<> struct traits<void>                { typedef void reference; };
template<> struct traits<void const>          { typedef void reference; };
template<> struct traits<void volatile>       { typedef void reference; };
template<> struct traits<void const volatile> { typedef void reference; };

class Reference
{
	unsigned long hard_count;
	unsigned long weak_count;
	Thread::SpinLock locker;
	virtual void destroy() = 0;
protected:
	virtual ~Reference() { }
	Reference() : hard_count(1), weak_count(1), locker("Reference::locker") { }
public:
	void hard_release()
	{
		{
		Thread::SpinLock::Scoped l(locker);
		if ( --hard_count )
			return;
		}
		destroy();
		weak_release();
	}
	void weak_release()
	{
		{
		Thread::SpinLock::Scoped l(locker);
		if ( --weak_count )
			return;
		}
		delete this;
	}
	void hard_ref()
	{
		Thread::SpinLock::Scoped l(locker);
		hard_count++;
	}
	bool hard_ref_check()
	{
		Thread::SpinLock::Scoped l(locker);
		if ( hard_count == 0 )
			return true;
		hard_count++;
		return false;
	}
	void weak_ref()
	{
		Thread::SpinLock::Scoped l(locker);
		weak_count++;
	}
};

template<typename T, typename D=DefaultDeleter<T> >
class ReferenceObject : public Reference
{
	T* obj;
	D deleter;
	void destroy() { deleter(obj); }
public:
	ReferenceObject(T* obj) : obj(obj), deleter(D()) { }
	ReferenceObject(T* obj, const D& del) : obj(obj), deleter(del) { }
};

};

template<typename T> class WeakReference;
template<typename T> class HardReference;

template<typename T>
class HardReference
{
	typedef typename reference_helper::traits<T>::reference reference;
	template<typename Y> friend class HardReference;
	template<typename Y> friend class WeakReference;
	reference_helper::Reference *ref;
	T *obj;
public:
	~HardReference() { if ( ref ) ref->hard_release(); }
	HardReference() : ref(NULL), obj(NULL) { }

	template<typename Y>
	explicit HardReference(Y *o) : ref(o ? new reference_helper::ReferenceObject<Y>(o) : NULL), obj(o) { }

	template<typename Y, typename D>
	HardReference(Y *o, const D& d) : ref(o ? new reference_helper::ReferenceObject<Y, D>(o, d) : NULL), obj(o) { }

	template<typename Y>
	explicit HardReference(const WeakReference<Y>& rhs);

	template<typename Y>
	HardReference(const HardReference<Y>& rhs) : ref(rhs.ref), obj(rhs.obj) { if ( ref ) ref->hard_ref(); }
	HardReference(const HardReference&    rhs) : ref(rhs.ref), obj(rhs.obj) { if ( ref ) ref->hard_ref(); }

	template<typename Y>
	HardReference& operator = (const HardReference<Y>& rhs)
	{
		if (ref) ref->hard_release();
		ref = rhs.ref;
		obj = rhs.obj;
		if (ref) ref->hard_ref();
		return *this;
	}

	HardReference& operator = (const HardReference& rhs)
	{
		if ( &rhs != this )
		{
			if (ref) ref->hard_release();
			ref = rhs.ref;
			obj = rhs.obj;
			if (ref) ref->hard_ref();
		}
		return *this;
	}

	template<typename Y>
	HardReference& operator = (const WeakReference<Y>& rhs);

	reference operator*  () const { return *obj; }
	T* GetObject   () const { return  obj; }
	T* operator->  () const { return  obj; }
	operator bool  () const { return ref ? true : false; }
	bool operator! () const { return ref ? false : true; }

	template<typename Y>
	void swap(HardReference<Y>& rhs)
	{
		std::swap(ref, rhs.ref);
		std::swap(obj, rhs.obj);
	}

	void clear()
	{
		HardReference<T>().swap(*this);
	}
};

template<typename T>
class WeakReference
{
	template<typename Y> friend class HardReference;
	template<typename Y> friend class WeakReference;
	reference_helper::Reference *ref;
	T         *obj;
public:
	~WeakReference() { if ( ref ) ref->weak_release(); }
	WeakReference() : ref(NULL), obj(NULL) { }

	template<typename Y>
	WeakReference(const HardReference<Y>& rhs) : ref(rhs.ref), obj(rhs.obj) { if( ref ) ref->weak_ref(); }

	template<typename Y>
	WeakReference(const WeakReference<Y>& rhs) : ref(rhs.ref), obj(rhs.obj) { if( ref ) ref->weak_ref(); }
	WeakReference(const WeakReference&    rhs) : ref(rhs.ref), obj(rhs.obj) { if( ref ) ref->weak_ref(); }

	template<typename Y>
	WeakReference& operator = (const WeakReference<Y>& rhs)
	{
		if (ref) ref->weak_release();
		ref = rhs.ref;
		obj = rhs.obj;
		if (ref) ref->weak_ref();
		return *this;
	}

	WeakReference& operator = (const WeakReference& rhs)
	{
		if ( &rhs != this )
		{
			if (ref) ref->weak_release();
			ref = rhs.ref;
			obj = rhs.obj;
			if (ref) ref->weak_ref();
		}
		return *this;
	}

	template<typename Y>
	WeakReference& operator = (const HardReference<Y>& rhs)
	{
		if (ref) ref->weak_release();
		ref = rhs.ref;
		obj = rhs.obj;
		if (ref) ref->weak_ref();
		return *this;
	}

	template<typename Y>
	void swap(WeakReference<Y>& rhs)
	{
		std::swap(ref, rhs.ref);
		std::swap(obj, rhs.obj);
	}

	void clear()
	{
		WeakReference<T>().swap(*this);
	}
};

template<typename T> template<typename Y> inline HardReference<T>::HardReference(const WeakReference<Y>& rhs) : ref(rhs.ref), obj(rhs.obj)
{
	if( ref && ref->hard_ref_check() )
	{
		ref = NULL;
		obj = NULL;
	}
}

template<typename T> template<typename Y> inline HardReference<T>& HardReference<T>::operator = (const WeakReference<Y>& rhs)
{
	if (ref) ref->hard_release();
	ref = rhs.ref;
	obj = rhs.obj;
	if (ref && ref->hard_ref_check() )
	{
		ref = NULL;
		obj = NULL;
	}
	return *this;
}

};
#endif
