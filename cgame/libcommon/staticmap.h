#ifndef __ONLINEGAME_GS_STATIC_MAP_TEMPLATE_H__
#define __ONLINEGAME_GS_STATIC_MAP_TEMPLATE_H__

#include "hashtab.h" //for abase::pair
#include <algorithm>

namespace abase
{

/**
	有着和map类似接口的静态map，使用数组完成
	因此有些时候不适合保存类，更适用于结构
*/

template <class _Key, class _Value,size_t map_cap>
class static_map
{
public:
	typedef _Key	key_type;
	typedef _Value	data_type;
	typedef _Value	mapped_type;
	typedef pair<_Key,_Value> value_type;
	typedef size_t size_type;

	typedef value_type * iterator;
	typedef const value_type * const_iterator;
	enum
	{
		CAPACITY = map_cap
	};
private:
	value_type _data[map_cap];
	size_t 	   _count;
	
	inline static bool __KeyCompare(const value_type & lhs, const key_type & __key)
	{
		return lhs.first < __key;
	}
	inline iterator __Find(const key_type & __key) const 
	{
		return (iterator)std::lower_bound(begin(),end(),__key,__KeyCompare);
	}
public:
	static_map():_count(0)
	{} 
public:
	size_type size()  const { return _count;}
	size_type max_size() const { return map_cap;}
	bool empty() const { return _count != 0;}
	iterator begin() { return _data;}
	iterator end() { return _data+_count;}
	const_iterator begin() const { return _data;}
	const_iterator end() const { return _data + _count;}

	iterator find(const key_type& __key) const 
	{
		iterator it = __Find(__key);
		if(it != end() && it->first == __key)
		{
			return it;
		}
		return (iterator)end();
	}

	data_type & operator[](const key_type& __key) {
		iterator it = __Find(__key);
		if(it != end() && it->first == __key)
		{
			return it->second;
		}
		if(_count < map_cap)
		{
			iterator tmp = end();
			for(;tmp != it; --tmp)
			{
				*tmp = *(tmp - 1);
			}
			*tmp = value_type(__key,data_type());
			it = tmp;
			_count ++;
		}
		else
		{
			ASSERT(false);
			it = begin();
		}
		return it->second;
	}

	pair<iterator, bool> insert(const value_type& x)
	{
		if(_count >= map_cap) return pair<iterator, bool>(end(),false);
		
		iterator it = __Find(x.first);
		if(it != end() && it->first == x.first)
		{
			return pair<iterator, bool>(end(),false);
		}
		else
		{
			iterator tmp = end();
			for(;tmp != it; --tmp)
			{
				*tmp = *(tmp - 1);
			}
			*tmp = x;
			_count ++;
			return pair<iterator,bool>(tmp,true);
		}
	}
	size_type erase(const key_type& __key) 
	{
		iterator it = find(__key);
		if(it == end()) return 0;
		erase(it);
		return 1;
	}

	iterator erase(iterator __it)
	{
		if(__it == end()) return __it;
		iterator it = __it;
		while((++it) != end())
		{
			*(it-1) = *it;
		}
		_count --;
		return __it;
	}
	
	void clear()
	{
		_count = 0;
	}

};

template <class _Key, class _Value,typename _Allocator = default_alloc>
class static_multimap
{
public:
	typedef _Key	key_type;
	typedef _Value	data_type;
	typedef _Value	mapped_type;
	typedef pair<_Key,_Value> value_type;
	typedef size_t size_type;

	typedef value_type * iterator;
	typedef const value_type * const_iterator;
private:
	abase::vector<value_type, _Allocator> _data;
	inline static bool __KeyCompare(const value_type & lhs, const key_type & __key)
	{
		return lhs.first < __key;
	}
	inline iterator __Find(const key_type & __key) const 
	{
		return (iterator)std::lower_bound(begin(),end(),__key,__KeyCompare);
	}
public:
	static_multimap()
	{} 

	void swap( static_multimap<_Key,_Value,_Allocator> & rhs)
	{
		_data.swap(rhs._data);
	}

	void reserve(size_t capacity)
	{
		if(capacity > _data.capacity())
		{
			_data.reserve(capacity);
		}
	}
public:
	size_type size()  const { return _data.size();}
	bool empty() const { return _data.empty();}
	iterator begin() { return _data.begin();}
	iterator end() { return _data.end();}
	const_iterator begin() const { return _data.begin();}
	const_iterator end() const { return _data.end();}

	iterator find(const key_type& __key) const 
	{
		iterator it = __Find(__key);
		if(it != end() && it->first == __key)
		{
			return it;
		}
		return (iterator)end();
	}

	data_type & operator[](const key_type & __key)
	{
		iterator it = find(__key);
		if(it != end()) return it->second;
		return insert(value_type(__key,data_type())).first->second;
	}


	pair<iterator, bool> insert(const value_type& x)
	{
		iterator it = __Find(x.first);
		int index = it - _data.begin();
		_data.insert(it,x);
		return pair<iterator,bool>(_data.begin() + index,true);
	}

	size_type erase(const key_type& __key) 
	{
		iterator it = __Find(__key);
		if(it == end()) return 0;
		iterator it2 = it;
		while(it2 != end() && it2->first == __key)
		{
			++it2;
		}
		_data.erase(it,it2);
		return it2 - it;
	}

	iterator erase(iterator __it)
	{
		_data.erase(__it);
		return __it;
	}
	
	void clear()
	{
		_data.clear();
	}

};

template <class _Value, typename _Allocator = default_alloc>
class static_set
{
public:
	typedef _Value	key_type;
	typedef _Value	data_type;
	typedef _Value	value_type;
	typedef _Value	mapped_type;
	typedef size_t size_type;

	typedef value_type * iterator;
	typedef const value_type * const_iterator;
private:
	abase::vector<value_type, _Allocator> _data;
	inline static bool __Compare(const key_type & __lhs, const key_type & __rhs)
	{
		return __lhs < __rhs;
	}
	inline iterator __Find(const key_type & __key) const 
	{
		return (iterator)std::lower_bound(begin(),end(),__key,__Compare);
	}
	static_set<_Value,_Allocator> & operator=(const static_set<_Value,_Allocator> &) ;
public:
	static_set()
	{} 

	void swap( static_set<_Value,_Allocator> & rhs)
	{
		_data.swap(rhs._data);
	}

	void reserve(size_t capacity)
	{
		if(capacity > _data.capacity())
		{
			_data.reserve(capacity);
		}
	}
public:
	size_type size()  const { return _data.size();}
	bool empty() const { return _data.empty;}
	iterator begin() { return _data.begin();}
	iterator end() { return _data.end();}
	const_iterator begin() const { return _data.begin();}
	const_iterator end() const { return _data.end();}

	bool exist(const key_type & __key) const
	{
		iterator it = __Find(__key);
		if(it != end() && *it == __key) return true;
		return false;
	}
	
	iterator find(const key_type& __key) const 
	{
		iterator it = __Find(__key);
		if(it != end() && *it == __key)
		{
			return it;
		}
		return (iterator)end();
	}

	size_type erase(const key_type& __key) 
	{
		iterator it = __Find(__key);
		if(it == end()) return 0;
		iterator it2 = it;
		while(it2 != end() && *it2 == __key)
		{
			++it2;
		}
		_data.erase(it,it2);
		return it2 - it;
	}
	
	size_type count(const key_type& __key) const
	{
		iterator it = __Find(__key);
		if(it == end()) return 0;
		iterator it2 = it;
		while(it2 != end() && *it2 == __key)
		{
			++it2;
		}
		return it2 - it;
	}

	pair<iterator, bool> insert(const value_type& x)
	{
		iterator it = __Find(x);
		int index = it - _data.begin();
		if(it != _data.end() && *it == x) return pair<iterator,bool>(_data.end(),false);
		_data.insert(it,x);
		return pair<iterator,bool>(_data.begin() + index,true);
	}


	iterator erase(iterator __it)
	{
		_data.erase(__it);
		return __it;
	}
	
	void clear()
	{
		_data.clear();
	}

};

template <size_t _MaxBitSize = 65536, typename _Allocator = default_alloc>
class bitmap
{
private:
	abase::vector<unsigned char, _Allocator> _data;
	size_t _bits_size;
public:
	typedef size_t size_type;
	bitmap():_bits_size(0)
	{} 

	void swap( bitmap<_MaxBitSize,_Allocator> & rhs)
	{
		_data.swap(rhs._data);
		abase::swap(_bits_size, rhs._bits_size);
	}

	void reserve(size_t capacity)
	{
		_data.reserve(capacity);
	}
public:
	size_type size()  const { return _bits_size;}
	bool empty() const { return _bits_size == 0;}
	void clear() { _data.clear(); _bits_size = 0;}
	const unsigned char * data(size_t &size) const 
	{
		size = _data.size();
		return _data.begin();
	}

	void set(size_t idx, bool value)
	{
		if(idx >= _MaxBitSize) return;
		size_t base = idx >> 3;
		if(base >= _data.size())
		{
			_data.reserve(base);
			_data.insert(_data.end(), base - _data.size() + 1, 0);
		}

		unsigned char val = _data[base];
		val &= ~(1<< (idx & 0x07));
		if(value) val |= (1 << (idx & 0x07));
		_data[base] = val;
		if(_bits_size < idx) _bits_size = idx;
	}

	bool get(size_t idx)
	{
		if(idx >= _MaxBitSize) return false;
		size_t base = idx >> 3;
		if(base >= _data.size()) return false;
		
		return _data[base] & (1 << (idx & 0x07));
	}

	void init(unsigned char * buf, size_t size)
	{
		abase::vector<unsigned char, _Allocator> data(buf, buf + size);
		_data.swap(data);
		_bits_size = size * 8;
	}
};
}


#endif

