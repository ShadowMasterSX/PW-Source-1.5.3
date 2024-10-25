#ifndef __POOL_H
#define __POOL_H

#include "reference.h"
#include "mutex.h"
#include <vector>

namespace GNET
{
	class CounterPolicy
	{
	protected:
		unsigned int cur_size;
		unsigned int max_size;
	public:
		enum { COUNT_UP, COUNT_DOWN, COUNT_BALANCE };
		CounterPolicy() : cur_size(0), max_size(0) { }
		void SetMaxSize(unsigned int size) { max_size = size; }
		unsigned int GetSize()    const { return cur_size; }
		unsigned int GetMaxSize() const { return max_size; }
		virtual bool Increment() = 0;
		virtual bool Decrement() = 0;
		virtual int  Balance() { return COUNT_BALANCE; }
	};

	class DefaultCounterPolicy : public CounterPolicy
	{
	public:
		DefaultCounterPolicy() { }
		bool Increment()
		{
			unsigned int size = cur_size + 1;
			if ( size > max_size )
				return false;
			cur_size = size;
			return true;
		}

		bool Decrement()
		{
			if ( cur_size > 0 )
			{
				cur_size--;
				return true;
			}
			return false;
		}
	};

	class FixedCounterPolicy : public DefaultCounterPolicy
	{
	public:
		FixedCounterPolicy() { }
		FixedCounterPolicy(unsigned int size) { SetMaxSize(size); }
		int Balance()
		{
			if ( cur_size < max_size )
				return COUNT_UP;
			else if ( cur_size == max_size )
				return COUNT_BALANCE;
			else
				return COUNT_DOWN;
		}
	};

	template<typename Object>
	class ReuseableObject : public Object
	{
		bool active;
	public:
		ReuseableObject() : active(false) { }
		bool SetActive()   { bool r = !active; active = true; return r; }
		bool SetInactive() { bool r = active; active = false; return r; }
	};

	template<typename Object, typename K = void>
	class ReuseableObjectRef : public HardReference<Object>
	{
		K key;
	public:
		typedef Object ref_type;
		ReuseableObjectRef(K k) : key(k) { }
		ReuseableObjectRef(Object *obj) : HardReference<Object>(obj), key(obj->Key()) { } 
		bool operator == (const ReuseableObjectRef& rhs) const { return key == rhs.key; }
		bool operator != (const ReuseableObjectRef& rhs) const { return key != rhs.key; }
		bool operator <  (const ReuseableObjectRef& rhs) const { return key <  rhs.key; }
		bool operator >  (const ReuseableObjectRef& rhs) const { return key >  rhs.key; }
		bool operator <= (const ReuseableObjectRef& rhs) const { return key <= rhs.key; }
		bool operator >= (const ReuseableObjectRef& rhs) const { return key >= rhs.key; }
		K Key() const { return key; }
	};

	template<typename Object>
	class ReuseableObjectRef<Object, void> : public HardReference<Object>
	{
	public:
		typedef Object ref_type;
		ReuseableObjectRef(Object *obj) : HardReference<Object>(obj) { } 
	};

	template<typename ObjectRef>
	class ReuseableObjectRefFactory
	{
	public:
		ObjectRef CreateInstance() { return ObjectRef( new typename ObjectRef::ref_type() ); }
	};

	template <typename ObjectContainer>
	class ObjectChooser
	{
	public:
		typedef typename ObjectContainer::iterator iterator;
		virtual void AppendObject(iterator it) = 0;
		virtual bool Choose(ObjectContainer& oc)
		{
			iterator it = oc.begin(), ie = oc.end();
			for(; it != ie && ! (*it)->SetActive(); ++it );
			bool r = (it != ie);
			if ( r ) AppendObject(it);
			return r;
		}
	};

	template <
		typename ObjectContainer,
		typename ObjectFactory = ReuseableObjectRefFactory<typename ObjectContainer::value_type>
	>
	class Pool
	{
		HardReference<CounterPolicy> counter_policy;
		ObjectContainer container;
		Thread::Mutex locker("Pool::locker");	

		typedef typename ObjectContainer::iterator iterator;
	public:
		Pool(HardReference<CounterPolicy> cp) : counter_policy(cp), locker(true) { while ( ! Balance() ); }

		bool RequestObject( ObjectChooser<ObjectContainer> &oc )
		{
			while (true)
			{
				Thread::Mutex::Scoped l(locker);
				if ( oc.Choose(container) )
					return true;
				if ( ! counter_policy->Increment() )
					break;
				container.insert( container.end(), ObjectFactory().CreateInstance() );
			}
			return false;
		}

		void ReturnObject(iterator it)
		{
			Thread::Mutex::Scoped l(locker);
			(*it)->SetInactive();
		}

		void Shrink()
		{
			Thread::Mutex::Scoped l(locker);
			std::vector<iterator> vec;
			for (iterator it = container.begin(), ie = container.end(); it != ie; ++it )
				if ( (*it)->SetActive() )
					vec.push_back( it );
			for (typename std::vector<iterator>::iterator it = vec.begin(), ie = vec.end(); it != ie; ++it )
				container.erase( *it );
		}

		bool Balance()
		{
			Thread::Mutex::Scoped l(locker);
			switch ( counter_policy->Balance() )
			{
			case CounterPolicy::COUNT_UP:
				if ( counter_policy->Increment() )
					container.insert( container.end(), ObjectFactory().CreateInstance() );
				return false;
			case CounterPolicy::COUNT_DOWN:
				if ( counter_policy->Decrement() )
				{
					for (iterator it = container.begin(), ie = container.end(); it != ie; ++it )
						if ( (*it)->SetActive() )
						{
							container.erase( it );
							break;
						}
				}
				return false;
			}
			return true;
		}
	};
};

#endif
