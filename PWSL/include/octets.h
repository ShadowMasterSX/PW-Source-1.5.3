/*
 *		一个可以动态增长的缓冲区，试用了Copy On Write和引用计数，可以对复制进行管理；
 *		作者： 未知
 *		时间： 200x
 *		公司：完美时空
*/

#ifndef __GNET_OCTETS_H
#define __GNET_OCTETS_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <abaseoctets.h>
namespace GNET
{
class Octets
{

	struct Rep
	{
		size_t cap;
		size_t len;
		size_t ref;
		char   d[];
		void addref();
		inline void release();
		inline void *data();
		inline void *clone();
		void *unique();
		inline 	void *reserve(size_t size);

		static Rep* create(size_t cap);
		static Rep null;
	};
	
	
	void *base;
	Rep *rep () const { return (Rep*)(((char*)base) - sizeof(Rep));}
	inline void unique() { base = rep()->unique(); }
public:
	~Octets();
	Octets (); 
	Octets (size_t size);
	Octets (const void *x, size_t size);
	Octets (const void *x, const void *y); 
	Octets (const Octets &x) : base(x.base) { rep()->addref(); }
	Octets& operator = (const Octets&x);
	bool operator == (const Octets &x) const { return size() == x.size() && !memcmp( base, x.base, size() ); }
	bool operator != (const Octets &x) const { return ! operator == (x); }
	bool operator < (const Octets &x) const { 
		int rst = memcmp( base, x.base, std::min(size(),  x.size()));
		if(rst) return rst < 0; else return size() < x.size();
	}
	Octets& swap(Octets &x) { void *tmp = base; base = x.base; x.base = tmp; return *this; }
	Octets& reserve(size_t size);
	Octets& replace(const void *data, size_t size);
	void *begin() { unique(); return base; }
	void *end()   { unique(); return (char*)base + rep()->len; }
	const void *begin() const { return base; }
	const void *end()   const { return (char*)base + rep()->len; }
	Octets& erase(void *x, void *y);
	Octets& insert(void *pos, const void *x, size_t len);
	inline bool empty() const { return rep()->len == 0;}
	inline size_t size()     const { return rep()->len; }
	inline size_t capacity() const { return rep()->cap; }
	inline Octets& clear() { unique(); rep()->len = 0; return *this;  }
	inline Octets& erase(size_t pos, size_t len) { char * x = (char*)begin(); return erase(x + pos, x + pos + len); }
	inline Octets& insert(void *pos, const void *x, const void *y) { insert(pos, x, (char*)y - (char*)x); return *this; }
	inline Octets& resize(size_t size) { reserve(size); rep()->len = size; return *this; }
	inline void push_byte(const char byte) { char b = byte; insert(end(), &b, sizeof(b));}
	void dump();
	static int GetCurrOctetsCounter();
};

};

#endif
