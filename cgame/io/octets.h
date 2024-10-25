#ifndef __OCTETS_H
#define __OCTETS_H

#include <stdlib.h>
#include <algorithm>

namespace ONET
{

class Octets
{
	void *base, *high;
	size_t capacity;
public:
	void reserve(size_t size)
	{
		if (size > capacity)
		{
			for (capacity = 2; size >>= 1; capacity <<= 1);
			size = (char*)high - (char*)base;
			base = realloc(base, capacity);
			high = (char*)base + size;
		}
	}
	Octets& replace(const void *data, size_t size)
	{
		reserve(size);
		memmove(base, data, size);
		high = (char*)base + size;
		return *this;
	}
	virtual ~Octets () { free(base); }
	Octets () : base(0), high(0), capacity(0) {}
	Octets (const void *x, const void *y) : base(0), high(0), capacity(0) { replace(x, (char*)y - (char*)x); }
	Octets (const Octets &x) : base(0), high(0), capacity(0) { if (x.size()) replace(x.begin(), x.size()); }
	Octets& operator = (const Octets&x) { return replace(x.begin(), x.size()); }
	bool operator == (const Octets &x) const { return x.size() == size() && !memcmp(x.base, base, size()); }
	void swap(Octets &x) { std::swap(base, x.base); std::swap(high, x.high); std::swap(capacity, x.capacity); }
	void* begin() { return base; }
	void* end()   { return high; }
	const void* begin() const { return base; }
	const void* end()   const { return high; }
	size_t size() const { return (char*)high - (char*)base; }
	void clear() { high = base; }
	void *erase(void *x, void *y)
	{
		if (x != y)
		{
			memmove(x, y, (char*)high - (char*)y);
			high = (char*)high - ((char*)y - (char*)x);
		}	
		return x;
	}
	void insert(void *pos, const void *x, size_t size)
	{
		size_t off = (char*)pos - (char*)base;
		reserve(size + (char*)high - (char*)base);
		if (pos)
		{
			pos = (char*)base + off;
			memmove((char*)pos + size, pos, (char*)high - (char*)pos);
			memmove(pos, x, size);
			high = (char*)high + size;
		} else {
			memmove(base, x, size);
			high = (char*)base + size;
		}
	}
	void insert(void *pos, const void *x, const void *y) { insert(pos, x, (char*)y - (char*)x); }
	void truncate(void *pos) { high = pos; }
};

};

#endif
