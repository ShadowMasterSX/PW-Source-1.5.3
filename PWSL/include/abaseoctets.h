#ifndef __ABASE_OCTET_H__
#define __ABASE_OCTET_H__

#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <stdlib.h>
#include "ASSERT.h"
#include "amemory.h"

namespace abase
{

class octets
{
public:
	typedef unsigned char byte;
	typedef byte * pointer;
	typedef const byte * const_pointer;
	typedef byte & reference;
	typedef const byte & const_reference;
	typedef pointer iterator;
	typedef const_pointer const_iterator;
private:
	byte * _data;
	byte * _end;
	size_t _capacity;

public:

	explicit octets(size_t n):_data(0),_end(0),_capacity(0)
	{ 
		reserve(n);
	}
	octets():_data(0),_end(0),_capacity(0)
	{
	}
	octets(iterator first,iterator last):_data(0),_end(0),_capacity(0)
	{
		push_back(first,last - first);
	}
	octets(const void * buf, size_t len):_data(0),_end(0),_capacity(0)
	{
		push_back(buf,len);
	}
	octets(const octets & rhs):_data(0),_end(0),_capacity(0)
	{
		push_back(rhs.begin(),rhs.size());
	}
	~octets()
	{
		if(_data) abase::fast_allocator::free(_data,_capacity);
	}
	
	iterator begin() { return _data;}
	iterator end() { return _end;}
	const_iterator begin() const { return _data;}
	const_iterator end() const { return _end;}
	reference operator[](size_t off){ ASSERT(_data && off < (unsigned int )(_end - _data));return _data[off];}
	const_reference operator[](size_t off) const { ASSERT(_data && off < (unsigned int)(_end - _data));return _data[off];}
	size_t size() const { return _end - _data;}
	bool empty() const { return _end == _data;}
	void clear(){ _end = _data; }
	
	void swap(octets & rhs)
	{
		std::swap(_data,rhs._data);
		std::swap(_end,rhs._end);
		std::swap(_capacity,rhs._capacity);
	}
	
	octets& operator =(const octets & rhs)
	{
		octets tmp(rhs);
		swap(tmp);
		return *this;
	}
	bool operator ==(const octets & rhs)
	{
		return size() == rhs.size() && !memcmp(begin(),rhs.begin(),size());
	}
	
	void __resize(size_t size)
	{
		reserve(size);
		if((size_t)(_end - _data) < size) _end = _data + size;
	}

	void reserve(size_t size)
	{
		if(size > _capacity)
		{
			size_t tmp = _capacity;
			if(_capacity < 16) _capacity = 16;
			while(_capacity < size) _capacity = _capacity * 2;
			size = _end - _data;
			_data = (pointer)abase::fast_allocator::realloc(_data, _capacity, tmp);
			_end = _data + size;
		}
	}

	void push_back(byte ch)
	{
		reserve((_end - _data) + 1);
		*_end++ = ch;
	}

	void push_back(const void * ref, size_t size)
	{
		reserve((_end - _data) + size);
		memcpy(_end,ref,size);
		_end += size;
	}

	void push_back(byte val ,size_t count)
	{
		reserve((_end - _data) + count);
		memset(_end,val,count);
		_end += count;
	}

	void insert(iterator it,const void * ref, size_t size)
	{
		int off = it - begin();
		reserve((_end - _data) + size);
		it = begin() + off;
		memmove(it + size,it,end() - it);
		memcpy(it,ref,size);
		_end += size;
	}

	void insert(iterator pos,iterator first,iterator last)
	{
		insert(pos,first,last - first);
	}

	void insert(iterator pos, byte val , size_t count)
	{
		int off = pos - begin();
		reserve((_end - _data) + count);
		pos = begin() + off;
		memmove(pos + count,pos,end() - pos);
		memset(pos,val,count);
		_end += count;		
	}



	iterator erase(iterator first, iterator last)
	{
		int size = last - first;
		if(size > 0)
		{
			memmove(first,last,end() - last);
			_end -= size;			
		}
		return first;
	}

	iterator erase(iterator pos)
	{
		ASSERT(pos);
		memmove(pos,pos+1,(end() - pos) - 1);
		_end --;
		return pos;
	}
};
}
#endif
