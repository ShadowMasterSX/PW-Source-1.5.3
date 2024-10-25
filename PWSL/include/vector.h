  /*
 * FILE: vector.h
 *
 * DESCRIPTION: vector
 *
 * CREATED BY: Cui Ming 2002-1-21
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
*/

#include "ASSERT.h"
#include <stdlib.h>
#include <new>

#ifndef __ABASE_VECTOR_H__
#define __ABASE_VECTOR_H__

#include "allocator.h"
namespace abase
{

template<class T>
inline void swap(T & lhs,T & rhs)
{
	T tmp;
	tmp = lhs;
	lhs = rhs;
	rhs = tmp;
}

template <class T, class _Allocator=default_alloc>
class vector
{
protected:
	inline T * _M_allocate(size_t __n) { return (T*)_allocator.alloc(__n * sizeof(T));}
	inline void _M_deallocate(T* __p, size_t __n){ if (__p) _allocator.free(__p,__n * sizeof(T));}
	inline T* _M_allocate_new(size_t __n,size_t & rst)
	{
		if(__n < 5)
			__n = 5;
		else
			__n += (size()>>1) + 2; 
		rst = __n;
		return _M_allocate(__n);
	}
	
	inline T *_M_allocate_and_copy(size_t __n, T *start, T * finish){
		T * tmp = _M_allocate(__n);
		T * tmp2;
		tmp2 = tmp;
		while(start != finish && (__n--)>0)
		{
			new ((void *)&*tmp2++) T(*start++);
		}
		return tmp;
	}
protected:
	T * _data;
	T * _finish;
	size_t _max_size;
	_Allocator _allocator;
public:
	typedef T * iterator;
	typedef const T * const_iterator;
	typedef _Allocator allocator_type;
	typedef T value_type;

public:
//	explicit vector(int n=10);
	vector(const _Allocator & = _Allocator());
	vector(int n, const T & val, const _Allocator & = _Allocator());
	vector(const vector<T,_Allocator> & rhs);
	vector(T * _F, T* _L, const _Allocator & = _Allocator());
	~vector();
	const vector<T,_Allocator> &operator=(const vector<T,_Allocator> & rhs);

public:
	inline const T & operator [](size_t pos) const {ASSERT(pos >=0 && pos < size());return _data[pos];}
	inline T & operator [](size_t pos) {ASSERT(pos >=0 && pos < size());return _data[pos];}
	inline const T & at(size_t pos) const {ASSERT(pos >=0 && pos < size()); return _data[pos];}
	inline T & at(size_t pos) {ASSERT(pos >=0 && pos < size()); return _data[pos];}
	inline size_t capacity() const {return _max_size;}
	inline size_t max_size() const{ return size_t(-1) / sizeof(T); }
	inline size_t size() const { return _finish - _data;}
	inline T & front() { return *begin(); }
	inline const T &front() const { return *begin(); }
	inline T & back(){return *(end()-1);}
	inline const T & back() const {return *(end()-1);}
	inline bool empty() const {return _data == _finish;}
	inline const T * begin() const {return _data;}
	inline T * begin() {return _data;}
	inline const T * end() const {return _finish;}
	inline T * end() { return _finish;}
	inline void clear(){ erase(begin(),end());}

	void reserve(size_t __n) {
		if (capacity() < __n) {
			const size_t __old_size = size();
			T * __tmp = _M_allocate_and_copy(__n, _data, _finish);
			for(T * fp = _data;fp < _finish;fp ++)
				fp->~T();
			_M_deallocate(_data, _max_size);
			_data		= __tmp;
			_finish		= __tmp + __old_size;
			_max_size	= __n;
		}
	}
	void push_back(const T & x);
	void pop_back() {_finish--;_finish->~T();}
	T *insert(T * data,const T& x);
	void insert(T *it, size_t n, const T& x);
	void swap(vector<T,_Allocator> & x);
	T * erase(T *obj);		// delete the obj
	void erase_noorder(T *obj);
	void erase(T *first, T * last);	// delete [first, last)

};

template <class T,class _Allocator>
vector<T,_Allocator>::vector(const _Allocator & alloc):_allocator(alloc)
{
	_max_size = 0;
	_data = NULL;
	_finish = _data;
}

template <class T,class _Allocator>
vector<T,_Allocator>::vector(int n, const T & val, const _Allocator & alloc):_allocator(alloc)
{
	_max_size	= n;
	if(n) 
	{
		_data		= _M_allocate(n);
		ASSERT(_data);
	}
	else 
		_data  = NULL;
	_finish		= _data;
	while(n--) new ((void*)&*_finish++) T(val);
}

template<class T,class _Allocator>
vector<T,_Allocator>::vector(const vector<T,_Allocator> & rhs):_allocator(rhs._allocator)
{
	_max_size	= rhs._max_size;
	if(rhs._data)
		_data = _M_allocate(_max_size);
	else 
		_data = NULL;
	_finish		= _data;
	int n = rhs.size();
	const T * it = rhs.begin();
	while(n--) new((void*)&*_finish++) T(*it++);
}

template<class T,class _Allocator>
vector<T,_Allocator>::vector(T * _F, T* _L, const _Allocator & alloc):_allocator(alloc)
{
	_max_size = 0;
	_data = NULL;
	_finish = _data;
	reserve(_L - _F);
	while(_F != _L)
		new ((void*)&*_finish++) T(*_F++);

}

template<class T,class _Allocator>
const vector<T,_Allocator> & vector<T,_Allocator>::operator=(const vector<T,_Allocator> & rhs)
{
	if(this == &rhs) return rhs;
	vector<T,_Allocator> tmp(rhs);
	tmp.swap(*this);
	return *this;
}

template <class T,class _Allocator>
vector<T,_Allocator>::~vector()
{	
	clear();
	_M_deallocate(_data,_max_size);
}

template <class T,class _Allocator>
void vector<T,_Allocator>::push_back(const T & x)
{
	if(_finish == _data + _max_size)
	{
		size_t i;
		T * newdata,*tmp1,*tmp2;
		size_t newsize;
		size_t cur_size = size();
		newdata = _M_allocate_new(cur_size + 1,newsize);
		ASSERT(newdata);
		tmp1 = newdata;
		tmp2 = _data;
		for(i = 0; i < cur_size;i++)
			new ((void *)&*tmp1++) T(*tmp2++);

		tmp2 = _data;
		for(i = 0; i < cur_size; i++)
			tmp2++->~T();

		_M_deallocate(_data,_max_size);
		_max_size = newsize;
		_data = newdata;
		_finish = _data + cur_size;
	}
	
	new ((void *)&*(_finish)) T(x);
	_finish ++;
}

template <class T,class _Allocator>
T *vector<T,_Allocator>::insert(T * data,const T& x)
{
	int idx;
	idx = data - _data;
	if(_finish == _data + _max_size)
	{
		T * newdata,*tmp1,*tmp2;
		size_t newsize;
		size_t cur_size = size();
		newdata = _M_allocate_new(cur_size + 1, newsize);
		ASSERT(newdata);
		tmp1 = newdata;
		tmp2 = _data;

		while(tmp2 < data)
			new ((void *)&*tmp1++) T(*tmp2++);

		new ((void *)&*tmp1++) T(x);

		while(tmp2 < _finish)
			new ((void *)&*tmp1++) T(*tmp2++);

		tmp2 = _data;
		for(size_t i = 0; i < cur_size; i++)
			tmp2++->~T();
			
		_M_deallocate(_data,_max_size);
		_max_size = newsize;
		_data = newdata;
		_finish = _data + cur_size;
	}
	else
	{
		if(_finish != data)
		{
			new ((void*)_finish) T(*(_finish -1));
			for(T * tp = _finish - 1;tp > data;tp --)
				*tp = *(tp-1);
			*data = x;
		}
		else
		{
			new((void*)_finish) T(x);
		}

		
	}
	_finish ++;
	return _data + idx;
}

template <class T,class _Allocator>
void vector<T,_Allocator>::insert(T *it, size_t n, const T& x)
{
	if(!n) return;
	int idx;
	idx = it - _data;
	if(size() + n > _max_size)
	{
		T * newdata,*tmp1,*tmp2;
		size_t newsize;
		size_t cur_size = size();
		newdata = _M_allocate_new(cur_size + n, newsize);
		ASSERT(newdata);
		tmp1 = newdata;
		tmp2 = _data;

		while(tmp2 < it)
			new ((void *)&*tmp1++) T(*tmp2++);

		for(size_t i = 0; i< n; i++) new ((void *)&*tmp1++) T(x);

		while(tmp2 < _finish)
			new ((void *)&*tmp1++) T(*tmp2++);

		tmp2 = _data;
		for(size_t i = 0; i < cur_size; i++)
			tmp2++->~T();

		_M_deallocate(_data,_max_size);
		_max_size = newsize;
		_data = newdata;
		_finish = _data + cur_size;
	}
	else
	{
		ASSERT(it <= _finish);
		if((size_t)(_finish - it) < n)
		{
			T * p = it + n;
			for(T * it2 = it; it2  != _finish;p ++,it2++)
			{
				new ((void*)p) T(*it2);
			}

			p = _finish;
			for(size_t size = n -(_finish - it); size > 0; --size,++p)
			{
				new ((void*)p) T(x);
			}
			for(p = it; p != _finish; ++p)
			{
				*p = x;
			}
		}
		else
		{
			T * p = _finish;
			for(T * it2 = _finish - n; it2  != _finish;p ++,it2++)
			{
				new ((void*)p) T(*it2);
			}

			p = _finish;
			for(T * it2 = _finish - n ; it2 != it ; )
			{
				*--p = *--it2;
			}

			for(p = it; p != it + n ; ++p)
			{
				*p = x;
			}
		}
	}
	_finish += n;
	return;
}

template <class T,class _Allocator>
T* vector<T,_Allocator>::erase(T *obj)
{
	if(size())
	{
		int idx = obj - _data;
		ASSERT(obj >= _data && obj < _finish);

		for(T * fp = obj;fp < _finish - 1;fp++)
		{
			*fp = *(fp+1);
		}
		_finish --;
		_finish->~T();	//destroy the last

		return _data + idx;
	}
	return _finish;

}

template <class T,class _Allocator>
void vector<T,_Allocator>::swap(vector<T,_Allocator> & __x)
{
    abase::swap(_data, __x._data);
    abase::swap(_finish, __x._finish);
    abase::swap(_max_size, __x._max_size);
    abase::swap(_allocator, __x._allocator);
}

template <class T,class _Allocator>
void vector<T,_Allocator>::erase(T *first, T * last)
{
	if(first == last) return;
	T * fp;
	T * fp2;
	for(fp = last,fp2 = first; fp < _finish;fp++,fp2++)
	{
		*fp2 = *fp;
	}

	for(;fp2 < _finish; fp2++)
	{
		fp2->~T();
	}
	_finish   -= (last - first);
}
template <class T,class _Allocator>
void vector<T,_Allocator>::erase_noorder(T *obj)
{
	int idx = obj - _data;
	ASSERT(idx >=0 && idx < (int)size());
	_finish --;
	if(!empty())
	{
		_data[idx] = *_finish;
	}
	_finish->~T();	//destroy the last
}

template<class T,class _Allocator>
inline void clear_ptr_vector(vector<T,_Allocator> & vec)
{
	size_t i;
	for(i = 0; i < vec.size(); i++)
	{
		delete vec[i];
	}
	vec.clear();
}

}
#endif
