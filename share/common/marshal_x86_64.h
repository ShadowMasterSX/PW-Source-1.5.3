#ifndef __MARSHAL_H
#define __MARSHAL_H

#include "common/platform.h"
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <climits>
#include "byteorder.h"
#include "octets.h"
#include "aliasing.h"
#include <iostream>
#include "boo.h"

namespace GNET
{

template<typename T>
inline T& remove_const(const T &t) { return const_cast<T&>(t); }

class Marshal
{
public:
	class Exception
	{
	public: 
		std::string what;
		Exception (){}
		Exception(char *t):what(t){}
	};
	enum Transaction { Begin, Commit, Rollback };

	class OctetsStream
	{
		enum { MAXSPARE = 16384};
		Octets  data;
		mutable unsigned int pos;
		mutable unsigned int tranpos;
		template<typename T> OctetsStream& push_byte(T t)
		{
			data.insert(data.end(), &t, sizeof(t));
			return *this;
		}
		template<typename T> void pop_byte(T &t) const
		{
			if (pos + sizeof(t) > data.size()) throw Marshal::Exception("marshal::pop_byte\n");
			t = *(T *)( (char*)data.begin() + pos );
			pos += sizeof(t);
		}
		unsigned char pop_byte_8() const
		{
			unsigned char c;
			pop_byte(c);
			return c;
		}
		unsigned short pop_byte_16() const
		{
			unsigned short s;
			pop_byte(s);
			return byteorder_16(s);
		}
		unsigned int pop_byte_32() const
		{
			unsigned int l;
			pop_byte(l);
			return byteorder_32(l);
		}
		unsigned long pop_byte_64() const
		{
			unsigned long ll;
			pop_byte(ll);
			return byteorder_64(ll);
		}

		friend class CompactUINT;
		friend class CompactSINT;

		OctetsStream& compact_uint32(size_t x)//change unsigned int to size_t to forbid convert from size_t to unsigned int 
		{
			if (x>UINT_MAX) throw Marshal::Exception("size of x is bigger than UINT_MAX in Marshal::compact_uint32\n");
			if (x < 0x80) return push_byte((unsigned char)x);
			else if (x < 0x4000) return push_byte(byteorder_16(x|0x8000));
			else if (x < 0x20000000) return push_byte(byteorder_32(x|0xc0000000));
			push_byte((unsigned char)0xe0);
			return push_byte(byteorder_32(x));
		}
		OctetsStream& compact_sint32(int x)
		{
			if (x >= 0)
			{
				if (x < 0x40) return push_byte((unsigned char)x);
				else if (x < 0x2000) return push_byte(byteorder_16(x|0x8000));
				else if (x < 0x10000000) return push_byte(byteorder_32(x|0xc0000000));
				push_byte((unsigned char)0xe0);
				return push_byte(byteorder_32(x));
			}
			if (-x > 0)
			{
				x = -x;
				if (x < 0x40) return push_byte((unsigned char)x|0x40);
				else if (x < 0x2000) return push_byte(byteorder_16(x|0xa000));
				else if (x < 0x10000000) return push_byte(byteorder_32(x|0xd0000000));
				push_byte((unsigned char)0xf0);
				return push_byte(byteorder_32(x));
			}
			push_byte((unsigned char)0xf0);
			return push_byte(byteorder_32(x));
		}
		const OctetsStream& uncompact_uint32(const unsigned int &x) const
		{
			switch ( *((unsigned char *)data.begin()+pos) & 0xe0)
			{
			case 0xe0:
				pop_byte_8();
				remove_const(x) = pop_byte_32();
				return *this;
			case 0xc0:
				remove_const(x) = pop_byte_32() & ~0xc0000000;
				return *this;
			case 0xa0:
			case 0x80:
				remove_const(x) = pop_byte_16() & ~0x8000;
				return *this;
			}
			remove_const(x) = pop_byte_8();
			return *this;
		}
		const OctetsStream& uncompact_sint32(const int &x) const
		{
			if (pos == data.size()) throw Marshal::Exception("uncompact_sint32\n");
			switch ( *((unsigned char *)data.begin()+pos) & 0xf0)
			{
			case 0xf0:
				pop_byte_8();
				remove_const(x) = -pop_byte_32();
				return *this;
			case 0xe0:
				pop_byte_8();
				remove_const(x) = pop_byte_32();
				return *this;
			case 0xd0:
				remove_const(x) = -(pop_byte_32() & ~0xd0000000);
				return *this;
			case 0xc0:
				remove_const(x) = pop_byte_32() & ~0xc0000000;
				return *this;
			case 0xb0:
			case 0xa0:
				remove_const(x) = -(pop_byte_16() & ~0xa000);
				return *this;
			case 0x90:
			case 0x80:
				remove_const(x) = pop_byte_16() & ~0x8000;
				return *this;
			case 0x70:
			case 0x60:
			case 0x50:
			case 0x40:
				remove_const(x) = -(pop_byte_8() & ~0x40);
				return *this;
			}
			remove_const(x) = pop_byte_8();
			return *this;
		}
	public:
		OctetsStream() : pos(0) {}
		OctetsStream(const Octets &o) : data(o), pos(0) {}
		OctetsStream(const OctetsStream &os) : data(os.data), pos(0) {}

		OctetsStream& operator = (const OctetsStream &os) 
		{
			if (&os != this)
			{
				pos  = os.pos;
				data = os.data;
			}
			return *this;
		}

		bool operator == (const OctetsStream &os) const { return data == os.data; }
		bool operator != (const OctetsStream &os) const { return data != os.data; }
		size_t size() const { return data.size(); }
		void swap (OctetsStream &os) { data.swap(os.data); }
		operator Octets& () { return data; }
		operator const Octets& () const { return data; }

		void *begin() { return data.begin(); }
		void *end()   { return data.end(); }
		void reserve(size_t size) { data.reserve(size); }
		const void *begin() const { return data.begin(); }
		const void *end()   const { return data.end();   }
		void insert(void *pos, const void *x, size_t len) { data.insert(pos, x, len); }
		void insert(void *pos, const void *x, const void *y) { data.insert(pos, x, y); }
		void erase(void *x, void *y) { data.erase(x, y); }
		void clear() { data.clear(); pos = 0; }

		OctetsStream& operator << (char x)               { return push_byte(x); }
		OctetsStream& operator << (unsigned char x)      { return push_byte(x); }
		OctetsStream& operator << (bool x)               { return push_byte(x); }
		OctetsStream& operator << (short x)              { return push_byte(byteorder_16(x)); }
		OctetsStream& operator << (unsigned short x)     { return push_byte(byteorder_16(x)); }
		OctetsStream& operator << (int x)                { return push_byte(byteorder_32(x)); }
		OctetsStream& operator << (unsigned int x)       { return push_byte(byteorder_32(x)); }
		//OctetsStream& operator << (long x)               { return push_byte(byteorder_32(x)); }
		//OctetsStream& operator << (unsigned long x)      { printf("dangerous action \n");return push_byte(byteorder_64(x)); }
		//OctetsStream& operator << (long long x)          { return push_byte(byteorder_64(x)); }
		//OctetsStream& operator << (unsigned long long x) { return push_byte(byteorder_64(x)); }
		OctetsStream& operator << (int64_t x)            { return push_byte(byteorder_64(x)); }
		OctetsStream& operator << (float x)              { return push_byte(byteorder_32(aliasing_cast<int>(x))); }
		OctetsStream& operator << (double x)             { return push_byte(byteorder_64(aliasing_cast<unsigned long long>(x))); }
		OctetsStream& operator << (const Marshal &x)     { return x.marshal(*this); }
		OctetsStream& operator << (const Octets &x)  
		{
			compact_uint32(x.size());
			data.insert(data.end(), x.begin(), x.end());
			return *this;
		}
		template<typename T>
		OctetsStream& operator << (const std::basic_string<T> &x)
		{
			STATIC_ASSERT(sizeof(T) == 1); // 需要在服务器处理utf16,utf32时，开放其他sizeof

			unsigned int bytes = x.length()*sizeof(T); // count of bytes
			compact_uint32(bytes);
			insert(end(), (void*)x.c_str(), bytes );

			return *this;
		}
		OctetsStream& push_byte(const char *x, size_t len)
		{
			data.insert(data.end(), x, len);
			return *this;
		}
		template<typename T1, typename T2>
		OctetsStream& operator << (const std::pair<T1, T2> &x)
		{
			return *this << x.first << x.second;
		}
		template<typename T>
		OctetsStream& operator << (const std::vector<T> &x) 
		{
		    return *this <<( MarshalContainer(x));
		}
		template<typename T>
		OctetsStream& operator << (const std::list<T> &x) 
		{
		    return *this <<( MarshalContainer(x));
		}
		template<typename T>
		OctetsStream& operator << (const std::deque<T> &x) 
		{
		    return *this <<( MarshalContainer(x));
		}
		template<typename T1, typename T2>
		OctetsStream& operator << (const std::map<T1, T2> &x) 
		{
		    return *this <<( MarshalContainer(x));
		}

		bool eos() const { return pos == data.size(); }
		const OctetsStream& operator >> (Marshal::Transaction trans) const
		{
			switch (trans)
			{
			case Marshal::Begin:
				tranpos = pos;
				break;
			case Marshal::Rollback:
				pos = tranpos;
				break;
			case Marshal::Commit:
				if (pos >= MAXSPARE)
				{
					remove_const(*this).data.erase((char*)data.begin(), (char*)data.begin()+pos);	
					pos = 0;
				}
			}
			return *this;
		}
		const OctetsStream& operator >> (const char &x) const
		{
			remove_const(x) = pop_byte_8();
			return *this;
		}
		const OctetsStream& operator >> (const unsigned char &x) const
		{
			remove_const(x) = pop_byte_8();
			return *this;
		}
		const OctetsStream& operator >> (const bool &x) const
		{
			remove_const(x) = pop_byte_8();
			return *this;
		}
		const OctetsStream& operator >> (const short &x) const
		{
			remove_const(x) = pop_byte_16();
			return *this;
		}
		const OctetsStream& operator >> (const unsigned short &x) const
		{
			remove_const(x) = pop_byte_16();
			return *this;
		}
		const OctetsStream& operator >> (const int &x) const
		{
			remove_const(x) = pop_byte_32();
			return *this;
		}
		const OctetsStream& operator >> (const unsigned int &x) const
		{
			remove_const(x) = pop_byte_32();
			return *this;
		}
		/*
		const OctetsStream& operator >> (const long &x) const
		{
			remove_const(x) = pop_byte_32();
			return *this;
		}
		const OctetsStream& operator >> (const unsigned long &x) const
		{
			remove_const(x) = pop_byte_64();
			return *this;
		}
		*/
		const OctetsStream& operator >> (const int64_t &x) const
		{
			remove_const(x) = pop_byte_64();
			return *this;
		}
		/*
		const OctetsStream& operator >> (const unsigned long long &x) const
		{
			remove_const(x) = pop_byte_64();
			return *this;
		}
		*/
		const OctetsStream& operator >> (const float &x) const
		{
			unsigned int l = pop_byte_32();
			remove_const(x) = aliasing_cast<float>(l);
			return *this;
		}
		const OctetsStream& operator >> (const double &x) const
		{
			unsigned long ll = pop_byte_64();
			remove_const(x) = aliasing_cast<double>(ll);
			return *this;
		}
		const OctetsStream& operator >> (const Marshal &x) const
		{
			return remove_const(x).unmarshal(*this);
		}
		const OctetsStream& operator >> (const Octets &x) const
		{
			unsigned int len;
			uncompact_uint32(len);
			if (len > data.size() - pos) throw Marshal::Exception("operator >> octets\n");
			remove_const(x).replace((char*)data.begin()+pos, len);
			pos += len;
			return *this;
		}
		template<typename T>
		const OctetsStream& operator >> (const std::basic_string<T> &x) const
		{
			STATIC_ASSERT(sizeof(T) == 1); // 需要在服务器处理utf16,utf32时，开放其他sizeof

			unsigned int bytes;
			uncompact_uint32(bytes);
			if (bytes % sizeof(T)) throw Marshal::Exception(); // invalid length
			if (bytes > data.size() - pos) throw Marshal::Exception();
			remove_const(x).assign((T*)((char*)data.begin()+pos), bytes/sizeof(T));
			pos += bytes;
			return *this;
		}
		void pop_byte(char *x, size_t len) const
		{
			if (pos + len > data.size()) throw Marshal::Exception("pop_byte x len \n");
			memcpy(x, (char*)data.begin()+pos, len);
			pos += len;
		}
		template<typename T1, typename T2>
		const OctetsStream& operator >> (const std::pair<T1, T2> &x) const
		{
			return *this >> remove_const(x.first) >> remove_const(x.second);
		}
		template<typename T>
		const OctetsStream& operator >> (const std::vector<T> &x) const
		{
			return *this >>( MarshalContainer(x));			
		}
		template<typename T>
		const OctetsStream& operator >> (const std::deque<T> &x) const
		{
			return *this >>( MarshalContainer(x));			
		}
		template<typename T>
		const OctetsStream& operator >> (const std::list<T> &x) const
		{
			return *this >>( MarshalContainer(x));			
		}
		template<typename T1, typename T2>
		const OctetsStream& operator >> (const std::map<T1, T2> &x) const
		{
			return *this >>( MarshalContainer(x));			
		}
	};

	virtual OctetsStream& marshal(OctetsStream &) const = 0;
	virtual const OctetsStream& unmarshal(const OctetsStream &) = 0;
	virtual std::ostream & trace(std::ostream & os) const { return os; }
	virtual ~Marshal() { }
};

class CompactUINT : public Marshal  
{
	unsigned int  *pi;
public:
	explicit CompactUINT(const unsigned int &i): pi(&remove_const(i)) { }//be careful ! don't pass an size_t for the argument i

	OctetsStream& marshal(OctetsStream &os) const
	{
		return os.compact_uint32(*pi);
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		return os.uncompact_uint32(*pi);
	}
};

class CompactSINT : public Marshal
{
	int *pi;
public:
	explicit CompactSINT(const int &i): pi(&remove_const(i)) { }

	OctetsStream& marshal(OctetsStream &os) const
	{
		return os.compact_sint32(*pi);
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		return os.uncompact_sint32(*pi);
	}
};



template<typename Container>
class STLContainer : public Marshal
{
	Container *pc;
public:
	explicit STLContainer(Container &c) : pc(&c) { }
	OctetsStream& marshal(OctetsStream &os) const
	{
		os << CompactUINT(pc->size());
		for (typename Container::const_iterator i = pc->begin(), e = pc->end(); i != e; ++i)
			os << *i;
		return os;
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		pc->clear();
		unsigned int size;
		for (os >> CompactUINT(size); size > 0; --size)
		{
			typename Container::value_type tmp;
			os >> tmp;
			pc->insert(pc->end(), tmp);
		}
		return os;
	}
};

template<typename Container>
inline STLContainer<Container> MarshalContainer(const Container &c)
{
	return STLContainer<Container> (remove_const(c));
}

/////////////////////////////////////////////////////////////////////////////
// trace
inline std::ostream & operator << (std::ostream & os, const Marshal & s) { return s.trace(os); }

template <class _T1, class _T2>
inline std::ostream & operator<<(std::ostream & os, const std::pair<_T1, _T2>& __x)
{
	return os << __x.first << '=' << __x.second;
}

template < typename _CType >
std::ostream & TraceContainer(std::ostream & os, const _CType & c)
{
	os << '[';
	typename _CType::const_iterator i = c.begin(), e = c.end();
	if ( i != e ) { os << *i; for ( ++i; i != e; ++i) os << ',' << *i; }
	os << ']';
	return os;
}

template < typename T >
std::ostream & operator << (std::ostream & os, const std::vector<T> &x)
{
	return TraceContainer(os, x);
}

template < typename T >
std::ostream & operator << (std::ostream & os, const std::list<T> &x)
{
	return TraceContainer(os, x);
}

template < typename T >
std::ostream & operator << (std::ostream & os, const std::deque<T> &x)
{
	return TraceContainer(os, x);
}

template < typename T1, typename T2>
std::ostream & operator << (std::ostream & os, const std::map<T1, T2> &x)
{
	return TraceContainer(os, x);
}

};

#endif
