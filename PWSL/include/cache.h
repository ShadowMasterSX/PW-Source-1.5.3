#ifndef __CACHE_H
#define __CACHE_H

#include <stdio.h>

#include <map>
#include <set>

#include "timer.h"
#include "thread.h"

namespace GNET
{
	typedef unsigned int								cache_stamp;

	/* default function, do nothing */
	template <typename _Key, typename _Tp>
	struct cache_get_func
	{
		bool operator() (const _Key &__key, _Tp &__value) const { return false; }
	};

	template <typename _Key, typename _Tp>
	struct cache_put_func
	{
		bool operator() (const _Key &__key, const _Tp &__value) const { return true; }
	};

	template <typename _Key>
	struct cache_remove_func
	{
		bool operator() (const _Key &__key) const { return true; }
	};

	template <typename _KeyTpPair>
	struct cache_onobtained_func
	{
		int operator() (const _KeyTpPair& __ktp, const cache_stamp& __stamp) const { return true; }
	};

	/* Cache Abstract Interface */
	template <typename _Key>
	class ICache
	{
	public:
		virtual ~ICache( ) { }
		virtual size_t Size() = 0;
		virtual void RDLock() = 0;
		virtual void WRLock() = 0;
		virtual void UnLock() = 0;
		virtual void RDLock( const _Key & __key ) = 0;
		virtual void WRLock( const _Key & __key ) = 0;
		virtual void UnLock( const _Key & __key ) = 0;
		virtual void DoSave( const _Key & __key ) = 0;
		virtual void OnDiscard( const _Key & __key ) = 0;
	};

	/* GlobalLock, Implementation of the LockPolicy used by Cache */
	template<typename _Key, typename _Compare>
	class GlobalLock
	{
		Thread::Mutex	_mutex;
	public:
		GlobalLock() : _mutex("Cache::GlobalLock::_mutex") { }

		void RDLock()					{	_mutex.Lock();		}
		void WRLock()					{	_mutex.Lock();		}
		void UNLock()					{	_mutex.UNLock();	}

		void RDLock(const _Key & __key)	{	_mutex.Lock();		}
		void WRLock(const _Key & __key)	{	_mutex.Lock();		}
		void UNLock(const _Key & __key)	{	_mutex.UNLock();	}
	};

	/* GlobalRWLock, Implementation of the LockPolicy used by Cache */
	template<typename _Key, typename _Compare>
	class GlobalRWLock
	{
		Thread::RWLock	_locker;
	public:
		GlobalRWLock() : _locker("Cache::GlobalRWLock::_locker") { }

		void RDLock()					{	_locker.ReadLock();	}
		void WRLock()					{	_locker.WriteLock();	}
		void UNLock()					{	_locker.Unlock();	}

		void RDLock(const _Key & __key)	{	_locker.ReadLock();	}
		void WRLock(const _Key & __key)	{	_locker.WriteLock();	}
		void UNLock(const _Key & __key)	{	_locker.Unlock();	}
	};

	/* GeneralSave, Implementation of the SavePolicy used by Cache */
	template<typename _Key, typename _Tp, typename _Compare, typename _Put>
	class GeneralSave
	{
		typedef std::pair<const _Key, _Tp>				value_type;

		ICache<_Key> *	_cache;
		_Put			_put_func;
	public:
		GeneralSave(ICache<_Key> *__c) : _cache(__c) { }
		~GeneralSave() { }

		size_t size() const					{	return 0;		}
		void set_put_func( _Put __f )		{	_put_func = __f;}
		int save_timer( ) const				{	return -1;		}
		void set_save_timer( int __timer )	{	/*do nothing*/	}
		bool ismodified(const _Key & __key)	{	return false;	}

		void OnClear()
		{
			/*do nothing*/
		}
		bool OnPut(const value_type & __x)
		{
			return _put_func(__x.first,__x.second);
		}
		bool OnRemove(const _Key & __key)
		{
			return true;
		}
	};

	/* BackSave, Implementation of the SavePolicy used by Cache */
	template<typename _Key, typename _Tp, typename _Compare, typename _Put>
	class BackSave
	{
		typedef std::pair<const _Key, _Tp>				value_type;
		typedef std::set<_Key,_Compare>					ModifiedSet;

		ICache<_Key> *	_cache;
		_Put			_put_func;
		int				_save_timer;
		ModifiedSet		_modified;

		class SaveModifiedTask : public Thread::Runnable
		{
			bool m_destroy;
			BackSave * m_save;
		public:
			SaveModifiedTask(BackSave * __s) : m_destroy(false), m_save(__s) { }
			void Destroy( ) { m_destroy = true; }

			void Run( )
			{
				if( m_destroy )
				{
					delete this;
					return;
				}

				if( m_save->_save_timer > 0 )
					m_save->save_modified();

				if( m_save->_save_timer > 0 )
					Thread::HouseKeeper::AddTimerTask( this, m_save->_save_timer );
				else
					Thread::HouseKeeper::AddTimerTask( this, 300 );
			}
		};

		SaveModifiedTask *	_savetask;

		void save_modified( )
		{
			_cache->WRLock();

			typename ModifiedSet::iterator mit;
			for( mit = _modified.begin(); mit != _modified.end(); ++mit )
			{
				_cache->DoSave( *mit );
			}
			_modified.clear();

			_cache->UnLock();
		}


	public:
		BackSave(ICache<_Key> *__c) : _cache(__c),
				_save_timer(300), _savetask(new SaveModifiedTask(this))
		{
			Thread::HouseKeeper::AddTimerTask( _savetask, 5 );
		}

		~BackSave()
		{
			if(_savetask) { _savetask->Destroy(); _savetask->Run();}
			_savetask = NULL;

			save_modified( );
		}

		size_t size() const					{	return _modified.size();}
		void set_put_func( _Put __f )		{	_put_func = __f;		}
		int save_timer( ) const				{	return _save_timer;		}
		void set_save_timer( int __timer )	{	_save_timer = __timer;	}
		bool ismodified(const _Key & __key)	{	return _modified.end() != _modified.find(__key);	}

		void OnClear( )
		{
			save_modified();
		}
		bool OnPut(const value_type & __x)
		{
			if( _save_timer <= 0 )
				return _put_func( __x.first, __x.second );
			_modified.insert(__x.first);
			return true;
		}
		bool OnRemove(const _Key & __key)
		{
			_modified.erase(__key);
			return true;
		}
	};

	/* LRUDiscard, Implementation of the DiscardPolicy used by Cache */
	template<typename _Key, typename _Compare>
	class LRUDiscard
	{
		typedef unsigned int 					access_type;
		typedef std::map<_Key, access_type, _Compare>		AccessMap;

		ICache<_Key> *	_cache;

		AccessMap		_access;
		Timer			_time;

		size_t			_max_size;
		size_t			_discard_time;
		size_t			_update_timer;

		class DiscardTask : public Thread::Runnable
		{
			bool m_destroy;
			LRUDiscard * m_discard;
		public:
			DiscardTask(LRUDiscard * __d) : m_destroy(false), m_discard(__d) { }
			void Destroy( ) { m_destroy = true; }

			void Run()
			{
				if( m_destroy )
				{
					delete this;
					return;
				}

				m_discard->discard();

				Thread::HouseKeeper::AddTimerTask( this, m_discard->_update_timer );
			}
		};

		DiscardTask *		_discardtask;

		void discard( int __elapse )
		{
			_cache->WRLock();

			typename AccessMap::iterator ait = _access.begin();
			while( ait != _access.end() )
			{
				if( (int)((*ait).second) <= __elapse )
				{
					_cache->OnDiscard( (*ait).first );

					typename AccessMap::iterator ait_temp = ait;
					++ ait;
					_access.erase(ait_temp);
				}
				else
				{
					++ ait;
				}
			}

			_cache->UnLock();
		}

		void discard( )
		{
			int now_elapse = _time.Elapse();
			int discard_elapse = now_elapse - _discard_time;
			discard( discard_elapse );

			if( size() > _max_size )
			{
				discard_elapse = now_elapse - (_discard_time/2);
				discard( discard_elapse );
			}
		}

	public:
		LRUDiscard(ICache<_Key> *__c) : _cache(__c),
			_max_size((size_t)-1), _discard_time(300), _update_timer(10),
			_discardtask(new DiscardTask(this))
		{
			Thread::HouseKeeper::AddTimerTask( _discardtask, _update_timer );
		}

		~LRUDiscard()
		{
			if(_discardtask) { _discardtask->Destroy(); _discardtask->Run();}
			_discardtask = NULL;
		}

		size_t size() const					{	return _access.size();	}
		size_t max_size() const				{	return _max_size;		}
		void set_max_size(size_t __size)	{	_max_size = __size;		}
		size_t discard_time() const			{	return _discard_time;	}
		void set_discard_time(size_t __time)
		{
			_discard_time = __time;
			_update_timer = std::max(__time/30, (size_t)10);
			if( _update_timer > __time )	_update_timer = __time;
		}

		void OnClear()
		{
			_access.clear();
		}
		bool OnPut(const _Key & __key)
		{
			_access[__key] = _time.Elapse();
			return true;
		}
		bool OnGet(const _Key & __key)
		{
			_access[__key] = _time.Elapse();
			return true;
		}
		bool OnRemove(const _Key & __key)
		{
			_access.erase(__key);
			return true;
		}
	};

	/* NoSync, Implementation of the SyncPolicy used by Cache */
	template<typename _Key, typename _Compare>
	class NoSync
	{
	public:
		NoSync() { }

		size_t size() const		{	return 0;	}

		void OnClear()
		{
		}
		bool OnPutStamp(const _Key & __key, const cache_stamp &__stamp, bool __autoinc )
		{
			return true;
		}
		bool OnGetStamp(const _Key & __key, cache_stamp &__stamp )
		{
			__stamp = 0;
			return true;
		}
		bool OnRemove(const _Key & __key)
		{
			return true;
		}
	};

	/* StampSync, Implementation of the SyncPolicy used by Cache */
	template<typename _Key, typename _Compare>
	class StampSync
	{
		typedef std::map<_Key, cache_stamp, _Compare>		StampMap;

		StampMap		_stamp;
	public:
		StampSync() { }

		size_t size() const		{	return _stamp.size();	}

		void OnClear()
		{
			_stamp.clear();
		}
		bool OnPutStamp(const _Key & __key, const cache_stamp &__stamp, bool __autoinc )
		{
			typename StampMap::iterator sit;
			sit = _stamp.find(__key);
			if( sit != _stamp.end() )
			{
				if( (*sit).second > __stamp )
					return false;
				if( __autoinc )
					(*sit).second = __stamp+1;
				return true;
			}
			return false;
		}
		bool OnGetStamp(const _Key & __key, cache_stamp &__stamp )
		{
			typename StampMap::iterator sit;
			sit = _stamp.find(__key);
			if( sit != _stamp.end() )
			{
				__stamp = (*sit).second;
			}
			else
			{
				__stamp = 1;
				_stamp.insert(std::make_pair(__key,__stamp));
			}
			return true;
		}
		bool OnRemove(const _Key & __key)
		{
			_stamp.erase(__key);
			return true;
		}
	};

	template
	<
		typename _Key,
		typename _Tp,
		typename _Get = cache_get_func<_Key,_Tp>,
		typename _Put = cache_put_func<_Key,_Tp>,
		typename _Remove = cache_remove_func<_Key>,
		typename _OnObtained = cache_onobtained_func<std::pair<const _Key,_Tp> >,
		typename _Compare = std::less<_Key>,
		typename _Alloc = std::allocator<std::pair<const _Key, _Tp> >,
		typename StoragePolicy = std::map<_Key,_Tp,_Compare,_Alloc>,
		typename LockPolicy = GlobalLock<_Key,_Compare>,
		typename SavePolicy = GeneralSave<_Key,_Tp,_Compare,_Put>,
		typename DiscardPolicy = LRUDiscard<_Key,_Compare>,
		typename SyncPolicy = NoSync<_Key,_Compare>
	>
	class Cache : public ICache<_Key>
	{
	public:
		typedef _Key									key_type;
		typedef _Tp										cached_type;
		typedef std::pair<const _Key, _Tp>				value_type;

	private:
		typedef _Compare								key_compare;
		typedef _Get									get_func;
		typedef _Put									put_func;
		typedef _Remove									remove_func;
		typedef _OnObtained								onobtained_func;

		//__glibcpp_class_requires(_Tp, _SGIAssignableConcept)
		//__glibcpp_class_requires4(_Compare, bool, _Key, _Key, _BinaryFunctionConcept)
		//__glibcpp_class_requires4(_Get, int, _Key, _Tp, _BinaryFunctionConcept)
		//__glibcpp_class_requires4(_Put, int, _Key, _Tp, _BinaryFunctionConcept)
		//__glibcpp_class_requires4(_OnObtained, int, value_type, cache_stamp, _BinaryFunctionConcept)

		StoragePolicy		_storage;
		LockPolicy			_lock;
		SavePolicy			_save;
		DiscardPolicy		_discard;
		SyncPolicy			_sync;

		get_func			_get_func;
		put_func			_put_func;
		remove_func			_remove_func;
		onobtained_func		_onobtained_func;

	public:

		Cache() : _save(this), _discard(this)
		{
			_save.set_put_func(_put_func);
			_save.set_save_timer(-1);
			_discard.set_max_size(-1);
			_discard.set_discard_time(300);
		}

		~Cache()
		{
		}

		void init( size_t __max_size = (size_t)-1, int __save_timer = -1, size_t __discard_time = 300 )
		{
			_discard.set_max_size(__max_size);
			_save.set_save_timer(__save_timer);
			_discard.set_discard_time(__discard_time);
		}

		void initfunc( get_func __get = get_func(),
				put_func __put = put_func(),
				remove_func __remove = remove_func(),
				onobtained_func __onob = onobtained_func() )
		{
			_get_func = __get;
			_put_func = __put;
			_remove_func = __remove;
			_onobtained_func = __onob;

			_save.set_put_func(_put_func);
		}

		void clear()
		{
			_save.OnClear( );	// will acquire lock

			_lock.WRLock();

			_storage.clear();
			_discard.OnClear();
			_sync.OnClear();

			_lock.UNLock();
		}

		size_t max_size() const					{	return _discard.max_size();		}
		void set_max_size(size_t __size)		{	_discard.set_max_size(__size);	}

		int save_timer() const					{	return _save.save_timer();		}
		void set_save_timer(int __timer = -1)	{	_save.set_save_timer(__timer);	}

		size_t discard_time() const				{	return _discard.discard_time();	}
		void set_discard_time(size_t __time)	{	_discard.set_discard_time(__time);	}

		bool put(const value_type& __x,const cache_stamp& __stamp = 0)
		{
			_lock.WRLock(__x.first);

			bool ret = false;
			if( _sync.OnPutStamp( __x.first, __stamp, true ) )
			{
				if( _storage.size() < _discard.max_size() )
				{
					ret = _save.OnPut(__x);
					if( ret )
					{
						_storage[__x.first] = __x.second;
						_discard.OnPut(__x.first);
					}
				}
				else
				{
					ret = _put_func(__x.first,__x.second);
				}
			}

			_lock.UNLock(__x.first);
			return ret;
		}

		bool get(const key_type& __key)
		{
			_lock.RDLock(__key);

			cache_stamp s;
			_sync.OnGetStamp(__key,s);

			typename StoragePolicy::const_iterator it = _storage.find(__key);
			if( it != _storage.end() )
			{
				_onobtained_func( *it, s );
			}
			else
			{
				cached_type v;
				if( ! _get_func(__key,v) )
				{
					_lock.UNLock(__key);
					return false;
				}

				if( _storage.size() < _discard.max_size() )
				{
					it = _storage.insert( std::make_pair(__key,v) ).first;
					_onobtained_func( *it, s );
				}
				else
				{
					_onobtained_func( std::make_pair(__key,v), s );
				}
			}

			_discard.OnGet(__key);
			_lock.UNLock(__key);
			return true;
		}

		bool get(const key_type& __key, cached_type& __value, cache_stamp& __stamp)
		{
			_lock.RDLock(__key);

			typename StoragePolicy::const_iterator it = _storage.find(__key);
			if( it != _storage.end() )
			{
				__value = (*it).second;
			}
			else
			{
				if( ! _get_func(__key,__value) )
				{
					_lock.UNLock(__key);
					return false;
				}

				if( _storage.size() < _discard.max_size() )
					it = _storage.insert( std::make_pair(__key,__value) ).first;
			}

			_sync.OnGetStamp(__key,__stamp);
			_discard.OnGet(__key);
			_lock.UNLock(__key);
			return true;
		}

		bool remove(const key_type& __key)
		{
			_lock.WRLock(__key);

			_storage.erase( __key );
			_save.OnRemove( __key );
			_sync.OnRemove( __key );
			_discard.OnRemove( __key );

			bool b = _remove_func(__key);

			_lock.UNLock(__key);
			return b;
		}

		void dump( )
		{
			_lock.RDLock();

			// printf( "_time.Elapse() = %d\n", _discard._time.Elapse() );
			printf( "_max_size = %zu\n", _discard.max_size() );
			printf( "_save_timer = %d\n", _save.save_timer() );
			printf( "_discard_time = %zu\n", _discard.discard_time() );

			printf( "_storage.size() = %zu\n", _storage.size() );
			printf( "_sync.size() = %zu\n", _sync.size() );
			printf( "_save.size() = %zu\n", _save.size() );
			printf( "_discard.size() = %zu\n", _discard.size() );

			_lock.UNLock();
		}

	public:

		// virtual functions, implement ICache<_Key>
		void Lock()		{	_lock.WRLock();	}
		void RDLock()	{	_lock.RDLock();	}
		void WRLock()	{	_lock.WRLock();	}
		void UnLock()	{	_lock.UNLock();	}
		void Lock( const key_type & __key )		{	_lock.WRLock(__key);	}
		void RDLock( const key_type & __key )	{	_lock.RDLock(__key);	}
		void WRLock( const key_type & __key )	{	_lock.WRLock(__key);	}
		void UnLock( const key_type & __key )	{	_lock.UNLock(__key);	}

		cached_type * GetDirectly(const key_type& __key)
		{
			typename StoragePolicy::const_iterator it = _storage.find(__key);
			if( it != _storage.end() )
			{
				_discard.OnGet(__key);
				return const_cast<cached_type *>(&((*it).second)); 
			}
			return NULL;
		}

		cached_type * GetDirectly(const key_type& __key, cached_type & __rv )
		{
			typename StoragePolicy::const_iterator it = _storage.find(__key);
			if( it != _storage.end() )
			{
				_discard.OnGet(__key);
				return const_cast<cached_type *>(&((*it).second)); 
			}
			else
			{
				if( ! _get_func(__key,__rv) )
				{
					return NULL;
				}

				if( _storage.size() < _discard.max_size() )
				{
					it = _storage.insert( std::make_pair(__key,__rv) ).first;
					_discard.OnGet(__key);
					return const_cast<cached_type *>(&((*it).second)); 
				}
				else
				{
					return &__rv;
				}
			}

			return NULL;
		}
		size_t GetSize()	
		{	
			_lock.RDLock();
			size_t size = _storage.size();	
			_lock.UNLock();
			return size;
		}

	private:
		size_t Size()	{	return _storage.size();	}

		void DoSave( const key_type & __key )	// call from SavePolicy
		{
			typename StoragePolicy::const_iterator it = _storage.find( __key );
			if( it != _storage.end() )
				_put_func( (*it).first, (*it).second );
		}

		void OnDiscard( const key_type& __key ) // call from DiscardPolicy
		{
			if( _save.ismodified(__key) )
				DoSave( __key );
			_storage.erase( __key );
			_save.OnRemove( __key );
			_sync.OnRemove( __key );
		}


	};

}

#endif
