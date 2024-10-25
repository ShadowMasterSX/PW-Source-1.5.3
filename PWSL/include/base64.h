/*
	base64.h
	作者： 魏华 
	修改：崔铭
	功能： base64编/解码函数
	日期： 2001-10
	修改日期：2002-1-17  除了表没有变之外,其他的都变了
*/

#ifndef __base64_h__
#define __base64_h__

/////////////////////////////////////////////////////////////////////////////
// codec
/////////////////////////////////////////////////////////////////////////////
// 解码：__in中的字符数一定是4的整倍数
// 返回解码长度
int	base64_decode(char *__in, int __inlen, unsigned char *__out);

// 编码：__out的长度一定是4的整倍数, 会自动0结尾
// 返回编码长度
int	base64_encode(unsigned char *__in, int __inlen, char *__out);

#endif	// EOF __base64_h__
#ifndef __BASE64_H__
#define __BASE64_H__
#include "octets.h"

namespace GNET
{

class Base64Encoder
{
public:
	static size_t encodeBound( size_t sourcelen ) 
	{ 
		return (sourcelen+2)/3*4; 
	}
	Base64Encoder(){ }
	Base64Encoder(const Base64Encoder &rhs){ }
	Octets& Update(Octets& in) 
	{ 
		Octets out;
		out.resize(encodeBound(in.size())+1); //因为下边的函数是0结尾的，所以要加1
		size_t len = base64_encode((unsigned char*)in.begin(), in.size(), (char*)out.begin());
		out.resize(len);
		return in.swap(out);
	}
	static void Convert(Octets& out, const Octets& in) 
	{ 
		out.resize(encodeBound(in.size())+1);
		size_t len =base64_encode((unsigned char*)in.begin(), in.size(), (char*)out.begin());
		out.resize(len);
	}
};

class Base64Decoder
{
public:
	static size_t decodeBound( size_t sourcelen ) { return sourcelen*3/4+4; }
	Base64Decoder(){ }
	Base64Decoder(const Base64Decoder &rhs){ }
	Octets& Update(Octets &in) 
	{ 
		Octets out;
		out.reserve(decodeBound(in.size()));
		size_t len = base64_decode((char *)in.begin(), in.size(), (unsigned char*)out.begin());
		out.resize(len);
		return in.swap(out);
	}
	static void Convert(Octets& out, const Octets& in) 
	{ 
		out.resize(decodeBound(in.size()));
		size_t len=base64_decode((char *)in.begin(), in.size(), (unsigned char*)out.begin());
		out.resize(len);
	}
};
}

#endif	
