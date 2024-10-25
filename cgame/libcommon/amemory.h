#include "ASSERT.h"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

#include "spinlock.h"
#include "interlocked.h"
#include "amemobj.h"

#ifndef __CMLIB_MEMORY_H__
#define __CMLIB_MEMORY_H__

#include "vector.h"
namespace abase
{
class chunk
{
	unsigned char * _buf;
	unsigned short _element_size;
	unsigned char _header;
	unsigned char _free_count;
	friend class fix_allocator;
public:
	static void * raw_alloc(size_t size)
	{
		return malloc(size);
	}
	static void raw_free(void * ptr)
	{
		::free(ptr);
	}
public:
	enum {
		MAX_COUNT = 255
	};

	chunk()
	{
		_buf = NULL;
		_header = 0xFF;
		_free_count = 0;
		_element_size = 0;
	}
	
	void init(unsigned short element_size)
	{
		_element_size = element_size;
		size_t count = element_size * MAX_COUNT;
		_buf = (unsigned char*)raw_alloc(count);
		if(!_buf) 
		{
			ASSERT(false);
			throw 0;
		}
		size_t offset = 0;
		for(size_t i = 0; i < MAX_COUNT; i ++,offset += element_size)
		{
			*(_buf + offset) = (i + 1) & 0xFF;
		}
		_header = 0;
		_free_count = MAX_COUNT;
	}
	
	void final()
	{
		raw_free(_buf);
		_buf = NULL;
		_header = 0xFF;
		_free_count = 0;
	}

	bool operator<(const chunk & rhs) const
	{
		ASSERT(_element_size == rhs._element_size);
		return _buf < rhs._buf;
	}

	bool operator==(const chunk & rhs) const
	{
		ASSERT(_element_size == rhs._element_size);
		return _buf == rhs._buf;
	}

	bool operator<(const void * tmp) const
	{
		return _buf + MAX_COUNT * _element_size < (unsigned char*)tmp;
	}

	void * alloc()
	{
		if(_header != 0xFF) 
		{
			void * tmp = _buf + _header * _element_size;
			_header = *(unsigned char *)tmp;
			--_free_count;
			return tmp;
		}
		ASSERT(false);
		return NULL;
	}
	bool is_empty()
	{
		return !_free_count;
	}
	bool is_full()
	{
		return _free_count == MAX_COUNT;
	}

	void  free(void * tmp)
	{
		ASSERT((unsigned char *)tmp >= _buf && (unsigned char *)tmp < (_buf + _element_size* MAX_COUNT) );
		int offset = (unsigned char *)tmp - _buf;
		ASSERT(offset % _element_size == 0);
		*(unsigned char*)tmp = _header;
		_header = offset / _element_size;
		++_free_count;
	}
	bool is_inside(void * buf)
	{
		return (unsigned char *)buf>= _buf && (unsigned char *)buf < (_buf + _element_size* MAX_COUNT);
	}
};

class fix_allocator
{
	typedef abase::vector<chunk>  chunk_list;
	chunk_list _block_list;
	chunk_list _empty_list;
	int _spinlock;
	int _element_size;
	int _count;
	
	friend class fast_allocator;
	static bool PT_COMP(const chunk &lhs, void * buf)
	{
		return lhs < buf;
	}
	
	void push_to_empty_list(chunk & ch)
	{
		if(_empty_list.empty())
		{
			_empty_list.push_back(ch);
		}
		else
		{
			insert(_empty_list,_empty_list.begin(),_empty_list.end(),ch);
		}
	}
	void alloc_new_chunk()
	{
		ASSERT(_block_list.empty());
		chunk ch;
		ch.init(_element_size);
		_block_list.push_back(ch);
		_count += chunk::MAX_COUNT;
		
	}
	inline chunk_list::iterator insert(	chunk_list & list, 
						chunk_list::iterator begin,chunk_list::iterator end,chunk & ch)
	{
		chunk_list::iterator it = std::lower_bound(begin,end,ch);
		ASSERT(it == end || ch < *it  );
		list.insert(it,ch);
		return it;
	}

	chunk * find_chunk(void * buf)
	{
		if(_block_list.back().is_inside(buf)) 
		{
			return _block_list.end() -1;
		}
		chunk_list::iterator end = _block_list.end() - 1;	
		chunk_list::iterator it = std::lower_bound(_block_list.begin(),end,buf,PT_COMP);
		if(it == end) return NULL;
		if(it->is_inside(buf)) 
		{
			return it;
		}
		return NULL;
	}
	
	bool get_from_empty_list(void * buf,chunk & ch)
	{
		chunk_list::iterator end = _empty_list.end();
		chunk_list::iterator it = std::lower_bound(_empty_list.begin(),end,buf,PT_COMP);
		if(it == end) 
		{
			ASSERT(false);
			return false;
		}
		if(it->is_inside(buf)) {
			ch = *it;
			_empty_list.erase(it);
			return true;
		}
		ASSERT(false);
		return false;
	}
	void remove_chunk(chunk & ch)
	{
		if(ch == _block_list.back())
		{
			ch.final();
			_block_list.pop_back();
			return;
		}
		chunk_list::iterator end = _block_list.end() - 1;
		chunk_list::iterator it = std::lower_bound(_block_list.begin(),end,ch);
		if(it != end && *it == ch)
		{
			ch.final();
			_block_list.erase(it);
			return;
		}
		else
		{
			ASSERT(false);
			return;
		}
	}

	int obj_count()
	{
		return total_count() - _count;
	}
	
	int total_count()
	{
		return 	(_block_list.size() + _empty_list.size()) * chunk::MAX_COUNT;
	}
	
public:
	void * operator new(size_t size)
	{
		return chunk::raw_alloc(size);
	}
	void operator delete(void * p)
	{
		chunk::raw_free(p);
	}
	
	fix_allocator(int element_size):_spinlock(0),_element_size(element_size),_count(0)
	{
		alloc_new_chunk();
	}

	void * alloc()
	{
		chunk & ch = _block_list.back();	
		void * tmp = ch.alloc();
		if(ch.is_empty())
		{
			push_to_empty_list(ch);
			_block_list.pop_back();
			if(_block_list.empty())
			{
				alloc_new_chunk();
			}
		}
		_count --;
		return tmp;
	}

	void free(void * buf)
	{
		chunk  *pChunk;
		if(!(pChunk = find_chunk(buf)))
		{
			chunk ch;
			if(!get_from_empty_list(buf,ch))
			{
				ASSERT(false);
				throw 0;
				return;
			}
			ch.free(buf);
			insert(_block_list,_block_list.begin(),_block_list.end() -1,ch);
			_count ++;
			return ;
		}
		pChunk->free(buf);
		_count ++;
		if(pChunk->is_full() && _count > 2 * chunk::MAX_COUNT)
		{
			remove_chunk(*pChunk);
			_count -= chunk::MAX_COUNT;
		}
	}
};

class fast_allocator
{
	fast_allocator(){}
	fast_allocator(const fast_allocator&);
	
	struct node_t
	{
		int 		_Slock;
		fix_allocator * _Ap;
	};
	enum 
	{
		MAX_SIZE = 1024,
		MAX_RECORD_SIZE = 10240,
	};
	static node_t _a_table[MAX_SIZE];
	static int _other_counter;
	static int _inside_counter;
	static int _large_size_counter[MAX_RECORD_SIZE - MAX_SIZE];

	static void record_large_size(size_t size, int offset)
	{
		if(size < MAX_RECORD_SIZE)
		{
			size_t off = size - MAX_SIZE;
			interlocked_add(&_large_size_counter[off],offset);
		}
		else
		{
			interlocked_add(&_inside_counter,offset);
		}
	}
public:
/*
	static fast_allocator & get_instance()
	{
		static fast_allocator __FA_Singleton;
		return __FA_Singleton;
	}*/
public:
	static void * raw_alloc(size_t size)
	{
		interlocked_increment(&_other_counter);
		return chunk::raw_alloc(size);
	}
	static void * alloc(size_t size)
	{
		if(size >= MAX_SIZE) 
		{
			record_large_size(size,1);
			return raw_alloc(size);
		}
		node_t & node = _a_table[size];
		spin_autolock keeper(node._Slock);
		if(node._Ap == NULL)
		{
			node._Ap = new fix_allocator(size);
		}
		void * pRst = node._Ap->alloc();
		keeper.detach();
#ifdef _CHECK_MEM_ALLOC		
		abase::RecordMemAlloc(pRst,size);
#endif
		return pRst;
	}
	static void * realloc(void * p,size_t size,size_t old_size)
	{
		if(p == NULL)
		{
			return alloc(size);
		}
		void * tmp = alloc(size);
		memcpy(tmp,p,size < old_size?size:old_size);
		free(p,old_size);
		return tmp;
	}

	static void * align_alloc(size_t size)
	{
		size_t newsize = ((-(int)size)&0x03) + size;
		ASSERT( newsize >= size);
		return alloc(newsize);
	}
	static void align_free(void * p, size_t size)
	{
		size_t newsize = ((-(int)size)&0x03) + size;
		ASSERT( newsize >= size);
		return free(p,newsize);
	}

	static void * align_alloc(size_t size,size_t align)
	{
		ASSERT((align &(align-1)) == 0);
		size_t newsize = ((-(int)size)&(align-1)) + size;
		ASSERT( newsize >= size);
		return alloc(newsize);
	}
	static void align_free(void * p, size_t size, size_t align)
	{
		ASSERT((align &(align-1)) == 0);
		size_t newsize = ((-(int)size)&(align-1)) + size;
		ASSERT( newsize >= size);
		return free(p,newsize);
	}
	
	static void raw_free(void * buf)
	{
		interlocked_decrement(&_other_counter);
		chunk::raw_free(buf);
	}
	
	static void free(void * buf, size_t size)
	{
		if(size >= MAX_SIZE) {
			record_large_size(size,-1);
			raw_free(buf);
			return ;
		}
#ifdef _CHECK_MEM_ALLOC		
		abase::RecordMemFree(buf,size);
#endif
		node_t & node = _a_table[size];
		spin_autolock keeper(node._Slock);
		if(node._Ap == NULL)
		{
			ASSERT(false);
			raw_free(buf);
			return ;
		}
		return node._Ap->free(buf);
	}

	static void dump(FILE * file)
	{
		int i;
		int count = 0;
		size_t size = 0;
		size_t size2 = 0;
		for(i = 0; i < MAX_SIZE; i++)
		{
			node_t & node = _a_table[i];
			spin_autolock keeper(node._Slock);
			if(node._Ap)
			{
				count ++;
				printf("%3d:%5d(alloced:%5d/%5d)\n",count,i,node._Ap->obj_count(),node._Ap->total_count());
				size += node._Ap->obj_count() * i;
				size2 += node._Ap->total_count() * i;
			}
		}
		printf("TOL:%u/%u\n",size,size2);
		printf("INSIDE:1024+(allocated:%5d)\n",_inside_counter);
		printf("OTR:1024+(allocated:%5d)\n",_other_counter);
	}
	static void dump_large(FILE * file)
	{
		size_t total = 0;
		int index = 0;
		int i;
		for(i = 0; i < MAX_RECORD_SIZE - MAX_SIZE; i++)
		{
			int counter = _large_size_counter[i];
			if(!counter)  continue;
			index ++;
			printf("%3d:%6d(alloced:%5d)\n",index, i + MAX_SIZE,counter);
			total += (i+MAX_SIZE)*counter;
		}
		printf("TOL:%u\n",total);
	}
};

template <int align = 4 ,int threshold= 128>
class fast_alloc
{
public:
	inline static void * allocate(size_t size) { 
		if(size <= threshold)
			return fast_allocator::align_alloc(size,align);
		else
			return fast_allocator::raw_alloc(size);
	}
	inline static void  deallocate(void * ptr,size_t size){ 
		if(size <= threshold)
			return fast_allocator::align_free(ptr,size,align);
		else
			return fast_allocator::raw_free(ptr);
	}
};

template <int threshold>
class fast_alloc<1,threshold>
{
public:
	inline static void * allocate(size_t size) { 
		if(size <= threshold)
			return fast_allocator::alloc(size);
		else
			return fast_allocator::raw_alloc(size);
	}
	inline static void * deallocate(void * ptr,size_t size){ 
		if(size <= threshold)
			fast_allocator::free(ptr,size);
		else
			fast_allocator::raw_free(ptr);

		return NULL;
	}
};

typedef fast_alloc<> dft_fast_alloc;

inline void * fastalloc(size_t size)
{
	return fast_allocator::align_alloc(size,sizeof(int));
}

inline void fastfree(void * p, size_t size)
{
	return fast_allocator::align_free(p,size,sizeof(int));
}

}
#endif

