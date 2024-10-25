#ifndef __BASE64_H__
#define __BASE64_H__
#include "octets.h"
#include "perf.h"


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
		out.resize(encodeBound(in.size()));
		base64_encode(out.begin(), in.begin(), in.size());
		return in.swap(out);
	}
	static void Convert(Octets& out, const Octets& in) 
	{ 
		out.resize(encodeBound(in.size()));
		base64_encode(out.begin(), in.begin(), in.size());
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
		size_t len = base64_decode(out.begin(), in.begin(), in.size());
		out.resize(len);
		return in.swap(out);
	}
	static void Convert(Octets& out, const Octets& in) 
	{ 
		out.resize(decodeBound(in.size()));
		base64_decode(out.begin(), in.begin(), in.size());
	}
};
}

#endif	
