/*
	用于字节序转换的几个函数，作用是将本地字节序转换为网络序，便于在网络上传递数据
	公司：完美时空
	时间：2009-07
*/
#ifndef __BYTEORDER_H
#define __BYTEORDER_H

#include <static_assert.h>
#include <stdint.h>

namespace GNET
{

#if defined BYTE_ORDER_BIG_ENDIAN
	#define byteorder_16(x)	(x)
	#define byteorder_32(x)	(x)
	#define byteorder_64(x)	(x)
#elif defined __GNUC__
	/*
	#define byteorder_16 htons
	#define byteorder_32 htonl
	*/
	inline unsigned short byteorder_16(unsigned short x)
	{
		register unsigned short v;
		 __asm__ __volatile__("xchgb %b0, %h0" : "=a"(v) : "0"(x));
		return v;
	}
	inline unsigned int byteorder_32(unsigned int x)
	{
		register unsigned int v;
		__asm__  __volatile__("bswap %0" : "=r"(v) : "0"(x));
		return v;
	}
	inline uint64_t byteorder_64(uint64_t x)
	{

#if defined(__x86_64__)
		register uint64_t v;
		__asm__ __volatile__("bswapq %0":"=r"(v):"0"(x));
		return v;
#else
		union
		{
			uint64_t __ll;
			uint32_t __l[2];
		} i, o;
		i.__ll = x;
		o.__l[0] = byteorder_32(i.__l[1]);
		o.__l[1] = byteorder_32(i.__l[0]);
		return o.__ll;
#endif
	}

#elif defined WIN32
	inline unsigned __int16 byteorder_16(unsigned __int16 x)
	{
		__asm ror x, 8
		return x;
	}
	inline unsigned __int32 byteorder_32(unsigned __int32 x)
	{
		__asm mov eax, x
		__asm bswap eax
		__asm mov x, eax
		return x;
	}
	inline unsigned __int64 byteorder_64(unsigned __int64 x)
	{
		union
		{
			unsigned __int64 __ll;
			unsigned __int32 __l[2];
		} i, o;
		i.__ll = x;
		o.__l[0] = byteorder_32(i.__l[1]);
		o.__l[1] = byteorder_32(i.__l[0]);
		return o.__ll;
	}
#endif
};

#endif
