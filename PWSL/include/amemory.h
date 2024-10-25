#include "ASSERT.h"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <string.h>

#include "spinlock.h"
#include "interlocked.h"
#include "vector.h"

#ifndef __CMLIB_MEMORY_CHUNK2_H__
#define __CMLIB_MEMORY_CHUNK2_H__

namespace abase
{
template<typename _Allocator>
class chunk
{
	unsigned short _element_size;
	unsigned char _header;
	unsigned char _free_count;
	unsigned char _buf[];

	typedef _Allocator allocator_type;
public:
	static void * raw_alloc(size_t size)
	{
		return allocator_type::allocate(size);
	}
	static void raw_free(void * ptr)
	{
		allocator_type::deallocate(ptr);
	}
private:
	void init(unsigned char MAX_COUNT)
	{
		size_t offset = 0;
		for(size_t i = 0; i < MAX_COUNT; i ++,offset += _element_size)
		{
			*(_buf + offset) = (i + 1) & 0xFF;
		}
		_header = 0;
		_free_count = MAX_COUNT;
	}

	void final()
	{
		_header = 0xFF;
		_free_count = 0;
	}
public:
	chunk(unsigned short element_size):_element_size(element_size)
	{
		_header = 0xFF;
		_free_count = 0;
	}
	
	static chunk * make_chunk(unsigned short element_size, unsigned char MAX_COUNT)
	{
		size_t count = element_size * MAX_COUNT;
		void * buffer = raw_alloc(count + sizeof(chunk));
		if(!buffer)
		{
			ASSERT(false);
			throw 0;
		}

		chunk * pChunk = new (buffer) chunk(element_size);
		pChunk->init(MAX_COUNT);
		return pChunk;
	}
	
	static void release_chunk(chunk * pChunk)
	{
		raw_free(pChunk);
	}

	void * alloc()
	{
		if(_free_count) 
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
	bool is_full(unsigned char MAX_COUNT)
	{
		return _free_count == MAX_COUNT;
	}

	void  free(void * tmp)
	{
		int offset = (unsigned char *)tmp - _buf;
		ASSERT(offset % _element_size == 0);
		*(unsigned char*)tmp = _header;
		_header = offset / _element_size;
		++_free_count;
	}
	bool is_inside(void * buf, size_t MAX_COUNT)
	{
		return (unsigned char *)buf>= _buf && (unsigned char *)buf < (_buf + _element_size* MAX_COUNT);
	}
};

template<typename _Allocator>
class fix_allocator
{
	typedef chunk<_Allocator> chunk_type;
	typedef abase::vector<chunk_type *, _Allocator>  chunk_list;
	typedef typename chunk_list::iterator chunk_iterator;
	chunk_list _block_list;
	chunk_list _empty_list;
	int _spinlock;
	int _element_size;
	int _count;
	size_t _count_per_chunk;
	size_t _size_per_chunk;
	
	class PT_COMP
	{
		size_t _size;
	public:
		PT_COMP(size_t size):_size(size) {}
		bool operator () (const chunk_type *lhs, const void * buf) const
		{ 
			return  (void *)(((char*)lhs) + _size) < buf;
		}
	};

	
	
	void push_to_empty_list(chunk_type * ch)
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
		chunk_type *ch = chunk_type::make_chunk(_element_size, _count_per_chunk); 
		_block_list.push_back(ch);
		_count += _count_per_chunk;
		
	}
	inline chunk_iterator insert(	chunk_list & list, 
						chunk_iterator begin, chunk_iterator end,chunk_type * ch)
	{
		chunk_iterator it = std::lower_bound(begin,end,ch);
		ASSERT(it == end || ch < *it  );
		list.insert(it,ch);
		return it;
	}

	chunk_type * find_chunk(void * buf)
	{
		if(_block_list.back()->is_inside(buf, _count_per_chunk)) 
		{
			return *(_block_list.end() -1);
		}
		chunk_iterator end = _block_list.end() - 1;	
		chunk_iterator it = std::lower_bound(_block_list.begin(),end, buf ,PT_COMP(_size_per_chunk));
		if(it == end) return NULL;
		if((*it)->is_inside(buf, _count_per_chunk)) 
			return *it;
		else
			return NULL;
	}
	
	chunk_type *  get_from_empty_list(void * buf)
	{
		chunk_iterator end = _empty_list.end();
		chunk_iterator it = std::lower_bound(_empty_list.begin(),end,buf,PT_COMP(_size_per_chunk));
		if(it == end) 
		{
			ASSERT(false);
			return NULL;
		}
		if((*it)->is_inside(buf, _count_per_chunk)) {
			chunk_type * ch = *it;
			_empty_list.erase(it);
			return ch;
		}
		ASSERT(false);
		return NULL;
	}
	void remove_chunk(chunk_type * ch)
	{
		if(ch == _block_list.back())
		{
			chunk_type::release_chunk(ch);
			_block_list.pop_back();
			return;
		}
		chunk_iterator end = _block_list.end() - 1;
		chunk_iterator it = std::lower_bound(_block_list.begin(),end,ch);
		if(it != end && *it == ch)
		{
			chunk_type::release_chunk(ch);
			_block_list.erase(it);
			return;
		}
		else
		{
			ASSERT(false);
			return;
		}
	}

	
	fix_allocator(const fix_allocator &);


public:
	inline int obj_count()
	{
		return total_count() - _count;
	}
	
	inline int total_count()
	{
		return 	(_block_list.size() + _empty_list.size()) * _count_per_chunk;
	}
	void * operator new(size_t size)
	{
		return chunk_type::raw_alloc(size);
	}
	void operator delete(void * p)
	{
		chunk_type::raw_free(p);
	}


	template <typename CHUNK_FUNC>
	fix_allocator(int element_size, CHUNK_FUNC func):_spinlock(0),_element_size(element_size),_count(0)
	{
		_count_per_chunk = func(element_size,sizeof(chunk_type));
		_size_per_chunk = sizeof(chunk_type) + element_size * _count_per_chunk;
		alloc_new_chunk();
	}
	~fix_allocator()
	{
		for(size_t i = 0;i < _block_list.size();++i)
		{
			chunk_type *ch = _block_list[i];
			chunk_type::release_chunk(ch);
		}
		for(size_t i = 0;i < _empty_list.size();++i)
		{
			chunk_type *ch = _empty_list[i];
			chunk_type::release_chunk(ch);
		}
	}

	void * alloc()
	{
		chunk_type * ch = _block_list.back();	
		void * tmp = ch->alloc();
		if(ch->is_empty())
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
		chunk_type  *pChunk;
		if(!(pChunk = find_chunk(buf)))
		{
			chunk_type * ch = get_from_empty_list(buf);
			if(!ch)
			{
				ASSERT(false);
				throw 0;
				return;
			}
			ch->free(buf);
			insert(_block_list,_block_list.begin(),_block_list.end() -1,ch);
			_count ++;
			return ;
		}
		pChunk->free(buf);
		_count ++;
		if(pChunk->is_full(_count_per_chunk) && _count >= 2 * (int)_count_per_chunk)
		{
			remove_chunk(pChunk);
			_count -= _count_per_chunk;
		}
	}
};

template <typename _Allocator, const int SM_SIZE = 32, const int LM_SIZE = 1024, const int MR_SIZE = 10240>
class fast_allocator_t
{
	fast_allocator_t(const fast_allocator_t&);
	const fast_allocator_t& operator = (const fast_allocator_t&);
	

	typedef fix_allocator<_Allocator> F_ALLOC;
	struct node_t
	{
		int 	_Slock;
		F_ALLOC	* _Ap;

		void 	* _s_ch_start;	//对于大型对象，可以考虑先分配几个小的buffer作为chunk
		void	* _s_ch_end;	//这个小型chunk的终结的指针位置
		void	* _free_header;	//小chunk的free_List
	};
	enum 
	{
		SMALL_MEM_SIZE 	= SM_SIZE,		//32字节一下是按照指定的字节来分配，以上则一律按照4字节对齐分配，这样最大会多耗费10%的内存
		LARGE_MEM_SIZE 	= LM_SIZE, 
		S_NODE_COUNT	= SMALL_MEM_SIZE,
		L_NODE_COUNT	= (LARGE_MEM_SIZE - SM_SIZE)/sizeof(int) + 1,
		MAX_RECORD_SIZE = MR_SIZE,
	};
	node_t _s_table[S_NODE_COUNT+1];
	node_t _l_table[L_NODE_COUNT];
	int _other_counter;
	int _inside_counter;
	int _large_size_counter[MAX_RECORD_SIZE - LARGE_MEM_SIZE];
	unsigned char (*_count_func)(int,int);					//用于计算每个chunk里的element_count的函数指针

	void record_large_size(size_t size, int offset)
	{
		if(size < MAX_RECORD_SIZE)
		{
			size_t off = size - LARGE_MEM_SIZE;
			interlocked_add(&_large_size_counter[off],offset);
		}
		else
		{
			interlocked_add(&_inside_counter,offset);
		}
	}

	static unsigned char Deafult_Count_func(int element_size, int overhead) { return 255;}
	static unsigned char PageAlign_Count_func(int element_size, int overhead)
	{
		const int pagesize = 4096;
		if(overhead < 2) overhead = 2;
		int size = element_size * 255 + overhead;
		if(size > pagesize)
		{
			float minsize = 1e8;
			size_t index = 255;
			for(size_t i = 255; i > 150; i --)
			{
				size_t tsize = element_size * i + overhead;
				int m = tsize & (pagesize - 1);
				if(m != 0) m = pagesize - m;
				float x = (m + overhead) / (float)i;
				if(minsize > x)
				{
					index =i;
					minsize = x;
				}
			}
			return index;
		}
		else
		{
			//不到 或者正好 4096
			//最小512字节，在4096 2048 1024 512 256 里寻找一个最不费的
			size_t max_x = 256;
			while(max_x < (size_t)size) max_x *=2;          //注意这样算出来的最小也要512 因为 overhead 不小于1
			size_t m1 = max_x - size;
			size_t count2 = ((max_x/2 - overhead)/element_size);
			size_t size2 = count2 *element_size + overhead;
			size_t m2 =(max_x/2) - size2;
			if(m2 < m1) return count2; else return 255;
		}
		return 255;
	}
public:

	fast_allocator_t(int chunk_size_policy = 0, unsigned char (*spec_func)(int, int) = NULL):_other_counter(0),_inside_counter(0)
	{
		memset(_s_table, 0 , sizeof(_s_table));
		memset(_l_table, 0 , sizeof(_l_table));
		memset(_large_size_counter, 0, sizeof(_large_size_counter));
		if(spec_func) 
			_count_func = spec_func;
		else if(chunk_size_policy ==0) 
			_count_func = Deafult_Count_func; 
		else 
			_count_func = PageAlign_Count_func;
	}
	void release()
	{
		for(int i = 0;i < S_NODE_COUNT+1;++i)
		{
			node_t& node = _s_table[i];
			spin_autolock keeper(node._Slock);
			if(node._s_ch_start) { _Allocator::deallocate(node._s_ch_start); node._s_ch_start = 0;}
			if(node._Ap) { delete node._Ap; node._Ap = 0;}
		}
		for(int i = 0;i < L_NODE_COUNT;++i)
		{
			node_t& node = _l_table[i];
			spin_autolock keeper(node._Slock);
			if(node._s_ch_start) { _Allocator::deallocate(node._s_ch_start); node._s_ch_start = 0;}
			if(node._Ap) { delete node._Ap; node._Ap = 0;}
		}	
	}

	void * raw_alloc(size_t size)
	{
		interlocked_increment(&_other_counter);
		return _Allocator::allocate(size);
	}

	void raw_free(void * buf)
	{
		interlocked_decrement(&_other_counter);
		return _Allocator::deallocate(buf);
	}

	//-----------------------------------------------------
	void * align_alloc(size_t size,size_t align = sizeof(int))
	{
		ASSERT((align &(align-1)) == 0);
		size_t newsize = ((-(int)size)&(align-1)) + size;
		ASSERT( newsize >= size);
		return alloc(newsize);
	}
	void align_free(void * p, size_t size, size_t align = sizeof(int))
	{
		ASSERT((align &(align-1)) == 0);
		size_t newsize = ((-(int)size)&(align-1)) + size;
		ASSERT( newsize >= size);
		return free(p,newsize);
	}
	
	//-----------------------------------------------------
private:
	node_t & GetNode(size_t & size)
	{
		if(size <= SMALL_MEM_SIZE)
		{
			return _s_table[size];
		}
		else
		{
			size =  ((-(int)size)&(4-1)) + size;
			size_t index = (size - SMALL_MEM_SIZE )/sizeof(int);
			ASSERT(index <L_NODE_COUNT);
			return _l_table[index];
		}
	}
public:
	void * alloc(size_t size)
	{
		if(size >= LARGE_MEM_SIZE) 
		{
			record_large_size(size,1);
			return raw_alloc(size);
		}
		if(size == 0) size = 1;

		node_t & node = GetNode(size);
		spin_autolock keeper(node._Slock);
		if(size > SMALL_MEM_SIZE)
		{
			//对于稍大一些的对象，会首先使用数目较少的chunk来完成分配
			if(node._s_ch_start == NULL)
			{
				//找出一个合适数量的用于小chunk, 这个chunk至少是3个，会尽量按照页对齐
				size_t nsize = size * 3;
				size_t allign_to_page = ((-(int)nsize)&(4096-1)) + nsize;
				size_t count = allign_to_page / size;

				node._s_ch_start = _Allocator::allocate(allign_to_page);
				node._s_ch_end = ((char*)node._s_ch_start) + allign_to_page;
				node._free_header = node._s_ch_start;
				char * header = (char*)node._free_header;
				for(size_t i =0; i < count - 1; i ++)
				{
					*((void**)header) = header + size;
					header += size;
				}
				*((void**)header) = NULL;
			}


			//首先试着在小chunk中进行分配
			if(node._free_header)
			{
				void * rst = node._free_header;
				node._free_header = *((void**)node._free_header);
				return rst;
			}

			//小chunk中已经满了，开始在正式的分配器中分配
		}
		
		if(node._Ap == NULL)
		{
			node._Ap = new F_ALLOC(size, _count_func);
		}
		void * pRst = node._Ap->alloc();
		keeper.detach();
		return pRst;
	}
	
	void free(void * buf, size_t size)
	{
		if(size >= LARGE_MEM_SIZE) {
			record_large_size(size,-1);
			raw_free(buf);
			return ;
		}
		if(size == 0) size = 1;
		node_t & node = GetNode(size);
		spin_autolock keeper(node._Slock);
		if(size > SMALL_MEM_SIZE)
		{
			ASSERT(node._s_ch_start);
			if(node._s_ch_start <= buf && buf < node._s_ch_end)
			{
				//在小chunk里面，直接放回去即可
				*((void**)buf) = node._free_header;
				node._free_header = buf;
				return ;
			}
		}

		if(node._Ap == NULL)
		{
			ASSERT(false);
			raw_free(buf);
			return ;
		}
		return node._Ap->free(buf);
	}

	void * realloc(void * p,size_t size,size_t old_size)
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

	void dump(FILE * file)
	{
		int i;
		int count = 0;
		size_t size = 0;
		size_t size2 = 0;

		for(i = 0; i < S_NODE_COUNT + 1; i++)
		{
			node_t & node = _s_table[i];
			spin_autolock keeper(node._Slock);
			if(node._Ap)
			{
				count ++;
				fprintf(file,"%3d:%5d(alloced:%5d/%5d)\n",count,i,node._Ap->obj_count(),node._Ap->total_count());
				size += node._Ap->obj_count() * i;
				size2 += node._Ap->total_count() * i;
			}
		}

		for(i = 0; i < L_NODE_COUNT;i ++)
		{
			node_t & node = _l_table[i];
			spin_autolock keeper(node._Slock);
			if(node._Ap)
			{
				count ++;
				fprintf(file,"%3d:%5lld(alloced:%5lld/%5lld)\n",count, (long long int)(i*sizeof(int) + SMALL_MEM_SIZE),(long long int)(node._Ap->obj_count()),(long long int)(node._Ap->total_count()));
				size += node._Ap->obj_count() * i;
				size2 += node._Ap->total_count() * i;
			}
			
		}
		
		fprintf(file,"TOL:%llu/%llu\n",(unsigned long long int)size,(unsigned long long int)size2);
		fprintf(file,"INSIDE:1024+(allocated:%5lld)\n",(long long int)_inside_counter);
		fprintf(file,"OTR:1024+(allocated:%5lld)\n",(long long int)_other_counter);
	}
	void dump_large(FILE * file)
	{
		size_t total = 0;
		int index = 0;
		int i;
		for(i = 0; i < MAX_RECORD_SIZE - LARGE_MEM_SIZE; i++)
		{
			int counter = _large_size_counter[i];
			if(!counter)  continue;
			index ++;
			fprintf(file,"%3d:%6d(alloced:%5d)\n",index, i + LARGE_MEM_SIZE,counter);
			total += (i+LARGE_MEM_SIZE)*counter;
		}
		fprintf(file,"TOL:%llu\n",(unsigned long long int) total);
	}
};



template <typename _Allocator = abase::default_alloc, int threshold = 1024> 
class fast_alloc
{
	abase::alloc_implement * _alloc;
	static fast_allocator_t<_Allocator> _f_allocator;
public:
	fast_alloc(abase::alloc_implement * alloc = NULL):_alloc(alloc) {}
	inline static void * allocate(size_t size) { 
		if(size <= threshold)
			return _f_allocator.alloc(size);
		else
			return _Allocator::allocate(size);
	}
	inline static void  deallocate(void * ptr,size_t size){ 
		if(size <= threshold)
			return _f_allocator.free(ptr,size);
		else
			return _Allocator::deallocate(ptr);
	}

	static fast_allocator_t<_Allocator> & GetAllocator() { return _f_allocator;}
	inline abase::alloc_implement * GetAllocatorImp() const {return _alloc;}
	inline void * alloc(size_t size)
	{
		if(_alloc)
			return _alloc->alloc(size);
		else
			return allocate(size);
	}

	inline void free(void * ptr, size_t size)
	{
		if(_alloc)
			return _alloc->free(ptr, size);
		else
			return deallocate(ptr,size);
	}

	inline void free(void * ptr)
	{
		//not support
		ASSERT(false);
	}
};


template <typename _Allocator , int threshold> 
fast_allocator_t<_Allocator> fast_alloc<_Allocator,threshold>::_f_allocator;

class fast_allocator
{
	public:
		static void * alloc(size_t size)
		{
			return fast_alloc<>::GetAllocator().alloc(size);
		}

		static void free(void *p ,size_t size)
		{
			return fast_alloc<>::GetAllocator().free(p, size);
		}
		
		static void * realloc(void * p, size_t size, size_t old_size)
		{

			return fast_alloc<>::GetAllocator().realloc(p, size, old_size);
		}

		static void * align_alloc(size_t size)
		{
			return fast_alloc<>::GetAllocator().align_alloc(size);
		}

		static void align_free(void * p ,size_t size)
		{
			return fast_alloc<>::GetAllocator().align_free(p, size);
		}

		static void * align_alloc(size_t size, size_t align)
		{
			return fast_alloc<>::GetAllocator().align_alloc(size);
		}

		static void  align_free(void * p, size_t size, size_t align)
		{
			return fast_alloc<>::GetAllocator().align_free(p, size, align);
		}

		static void * raw_alloc(size_t size)
		{
			return fast_alloc<>::GetAllocator().raw_alloc(size);
		}

		static void raw_free(void * buf)
		{
			 return fast_alloc<>::GetAllocator().raw_free(buf);
		}
		
		static void dump(FILE * file)
		{
			return fast_alloc<>::GetAllocator().dump(file);
		}

		static void dump_large(FILE * file)
		{
			return fast_alloc<>::GetAllocator().dump_large(file);
		}

};

inline void * fastalloc(size_t size)
{
	return fast_alloc<>::allocate(size);
}

inline void fastfree(void * p, size_t size)
{
	return fast_alloc<>::deallocate(p,size);
}

}
#endif

