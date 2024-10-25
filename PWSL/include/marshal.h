/*
 *	打包类，这是很多类的基础
 *	公司：完美时空
 *
 */

#ifndef __MARSHAL_H
#define __MARSHAL_H


#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <stdint.h>

#include <octets.h>
#include <byteorder.h>

#include <static_assert.h>

namespace GNET
{

template<typename T>
inline T& REMOVE_CONST(const T &t) { return const_cast<T&>(t); }

class Marshal
{
	template <typename T1,typename T2> static inline T1 binary_cast(const T2 x) 
	{
		union X1
		{
			T1 v1;
			T2 v2;
			X1(const T2& value):v2(value){}
		};
		return X1(x).v1;
	};
public:
	class Exception { };
	enum Transaction { Begin, Commit, Rollback };

	class OctetsStream
	{
		enum { MAXSPARE = 16384};
		Octets  data;
		mutable unsigned int pos;
		mutable unsigned int tranpos;
		unsigned int base;
		unsigned int end_offset;
		template<typename T> OctetsStream& push_byte(T t)
		{
			data.insert(data.end(), &t, sizeof(t));
			return *this;
		}

		template<typename T> OctetsStream& replace_byte(size_t pos_rep, T t)
		{
			void * p = (char*)data.begin() + pos_rep;
			memcpy(p, &t, sizeof(t));
			return *this;
		}
		template<typename T> void pop_byte(T &t) const
		{
			if (pos + sizeof(t) > endpos()) throw Marshal::Exception();
			t = *(T *)( (char*)begin() + pos );
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
		uint32_t pop_byte_32() const
		{
			uint32_t l;
			pop_byte(l);
			return byteorder_32(l);
		}
		uint64_t pop_byte_64() const
		{
			uint64_t ll;
			pop_byte(ll);
			return byteorder_64(ll);
		}

		friend class CompactUINT;
		friend class SpecCompactUINT;
		friend class CompactSINT;

		OctetsStream& compact_uint32(unsigned int x)
		{
			if (x < 0x80) return push_byte((unsigned char)x);
			else if (x < 0x4000) return push_byte(byteorder_16(x|0x8000));
			else if (x < 0x20000000) return push_byte(byteorder_32(x|0xc0000000));
			push_byte((unsigned char)0xe0);
			return push_byte(byteorder_32(x));
		}

		OctetsStream& spec_compact_uint32(size_t pos , unsigned int x/*实际值*/, unsigned int y/*确定位数的值*/)
		{
			if (y < 0x80) return replace_byte(pos, (unsigned char)x);
			else if (y < 0x4000) return replace_byte(pos,byteorder_16(x|0x8000));
			else if (y < 0x20000000) return replace_byte(pos, byteorder_32(x|0xc0000000));
			replace_byte(pos, (unsigned char)0xe0);
			return  replace_byte(pos + 1, byteorder_32(x));
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
			if (pos == endpos()) throw Marshal::Exception();
			switch ( *((unsigned char *)data.begin()+pos) & 0xe0)
			{
			case 0xe0:
				pop_byte_8();
				REMOVE_CONST(x) = pop_byte_32();
				return *this;
			case 0xc0:
				REMOVE_CONST(x) = pop_byte_32() & ~0xc0000000;
				return *this;
			case 0xa0:
			case 0x80:
				REMOVE_CONST(x) = pop_byte_16() & ~0x8000;
				return *this;
			}
			REMOVE_CONST(x) = pop_byte_8();
			return *this;
		}
		const OctetsStream& uncompact_sint32(const int &x) const
		{
			if (pos == endpos()) throw Marshal::Exception();
			switch ( *((unsigned char *)data.begin()+pos) & 0xf0)
			{
			case 0xf0:
				pop_byte_8();
				REMOVE_CONST(x) = -pop_byte_32();
				return *this;
			case 0xe0:
				pop_byte_8();
				REMOVE_CONST(x) = pop_byte_32();
				return *this;
			case 0xd0:
				REMOVE_CONST(x) = -(pop_byte_32() & ~0xd0000000);
				return *this;
			case 0xc0:
				REMOVE_CONST(x) = pop_byte_32() & ~0xc0000000;
				return *this;
			case 0xb0:
			case 0xa0:
				REMOVE_CONST(x) = -(pop_byte_16() & ~0xa000);
				return *this;
			case 0x90:
			case 0x80:
				REMOVE_CONST(x) = pop_byte_16() & ~0x8000;
				return *this;
			case 0x70:
			case 0x60:
			case 0x50:
			case 0x40:
				REMOVE_CONST(x) = -(pop_byte_8() & ~0x40);
				return *this;
			}
			REMOVE_CONST(x) = pop_byte_8();
			return *this;
		}
	public:
		OctetsStream() : pos(0),base(0),end_offset(0) {}
		OctetsStream(const Octets &o) : data(o), pos(0),base(0) ,end_offset(0){}
		OctetsStream(const OctetsStream &os) : data(os.data), pos(0), base(os.base) ,end_offset(os.end_offset){}

		OctetsStream& operator = (const OctetsStream &os) 
		{
			if (&os != this)
			{
				pos  = os.pos;
				data = os.data;
				base = os.base;
				end_offset = os.end_offset;
			}
			return *this;
		}

		bool operator == (const OctetsStream &os) const { return data == os.data; }
		bool operator != (const OctetsStream &os) const { return data != os.data; }
		size_t size() const { return data.size() - base - end_offset; }
		unsigned int endpos() const { return data.size() - base  - end_offset;}
		void swap (OctetsStream &os) { data.swap(os.data); }
		operator Octets& () { return data; }
		operator const Octets& () const { return data; }
		size_t position() const { return pos; }
		size_t raw_size() const { return data.size(); }	//当前数据的原始大小，只用在某些特殊的地方

		void *begin() { return (char*)data.begin() + base; }
		void *end()   { return (char*)data.end() - end_offset; }
		void reserve(size_t size) { data.reserve(size); }
		const void *begin() const { return (char*)data.begin() + base; }
		const void *end()   const {return (char*)data.end() - end_offset; }

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
		OctetsStream& operator << (int64_t x)            { return push_byte(byteorder_64(x)); }
		OctetsStream& operator << (uint64_t x)            { return push_byte(byteorder_64(x)); }
		OctetsStream& operator << (float x)              { return push_byte(byteorder_32(binary_cast<unsigned int>(x)));}
		OctetsStream& operator << (double x)             { return push_byte(byteorder_64(binary_cast<uint64_t>(x))); }
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
		template<typename T1, typename T2>
		OctetsStream& operator << (const std::set<T1, T2> &x) 
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
					REMOVE_CONST(*this).data.erase((char*)data.begin(), (char*)data.begin()+pos);	
					pos = 0;
				}
			}
			return *this;
		}

		const OctetsStream& operator << (const Marshal::Transaction trans) 
		{
			switch (trans)
			{
			case Marshal::Begin:
				tranpos = data.size();
				break;
			case Marshal::Rollback:
				REMOVE_CONST(*this).data.erase((char*)data.begin() + tranpos, (char*)data.end());
				break;
			case Marshal::Commit:
				break;
			}
			return *this;
		}
		const OctetsStream& operator >> (const char &x) const
		{
			REMOVE_CONST(x) = pop_byte_8();
			return *this;
		}
		const OctetsStream& operator >> (const unsigned char &x) const
		{
			REMOVE_CONST(x) = pop_byte_8();
			return *this;
		}
		const OctetsStream& operator >> (const bool &x) const
		{
			REMOVE_CONST(x) = pop_byte_8();
			return *this;
		}
		const OctetsStream& operator >> (const short &x) const
		{
			REMOVE_CONST(x) = pop_byte_16();
			return *this;
		}
		const OctetsStream& operator >> (const unsigned short &x) const
		{
			REMOVE_CONST(x) = pop_byte_16();
			return *this;
		}
		const OctetsStream& operator >> (const int &x) const
		{
			REMOVE_CONST(x) = pop_byte_32();
			return *this;
		}
		const OctetsStream& operator >> (const unsigned int &x) const
		{
			REMOVE_CONST(x) = pop_byte_32();
			return *this;
		}

		const OctetsStream& operator >> (const int64_t &x) const
		{
			REMOVE_CONST(x) = pop_byte_64();
			return *this;
		}

		const OctetsStream& operator >> (const uint64_t &x) const
		{
			REMOVE_CONST(x) = pop_byte_64();
			return *this;
		}
		
		const OctetsStream& operator >> (const float &x) const
		{
			uint32_t l = pop_byte_32();
			REMOVE_CONST(x) = binary_cast<float>(l);
			return *this;
		}
		const OctetsStream& operator >> (const double &x) const
		{
			uint64_t ll = pop_byte_64();
			REMOVE_CONST(x) = binary_cast<double>(ll);
			return *this;
		}
		const OctetsStream& operator >> (const Marshal &x) const
		{
			return REMOVE_CONST(x).unmarshal(*this);
		}
		const OctetsStream& operator >> (const Octets &x) const
		{
			unsigned int len;
			uncompact_uint32(len);
			if (len > (unsigned int)endpos() - pos) throw Marshal::Exception();
			REMOVE_CONST(x).replace((char*)data.begin()+pos, len);
			pos += len;
			return *this;
		}

		const OctetsStream & ShiftRight(unsigned int len) const
		{
			if (len > (unsigned int)endpos() - pos) throw Marshal::Exception();
			pos += len;
			return *this;
		}

		void StreamRefData(OctetsStream &x) const
		{
			unsigned int len;
			uncompact_uint32(len);
			if (len > (unsigned int)endpos() - pos) throw Marshal::Exception();
			x.data = data;
			x.pos = 0;
			x.base = pos;
			pos += len;
			x.end_offset = data.size() - pos;
		}
		template<typename T>
		const OctetsStream& operator >> (const std::basic_string<T> &x) const
		{
			STATIC_ASSERT(sizeof(T) == 1); // 需要在服务器处理utf16,utf32时，开放其他sizeof

			unsigned int bytes;
			uncompact_uint32(bytes);
			if (bytes % sizeof(T)) throw Marshal::Exception(); // invalid length
			if (bytes > endpos() - pos) throw Marshal::Exception();
			REMOVE_CONST(x).assign((T*)((char*)data.begin()+pos), bytes/sizeof(T));
			pos += bytes;
			return *this;
		}
		void pop_byte(char *x, size_t len) const
		{
			if (pos + len > endpos()) throw Marshal::Exception();
			memcpy(x, (char*)data.begin()+pos, len);
			pos += len;
		}
		template<typename T1, typename T2>
		const OctetsStream& operator >> (const std::pair<T1, T2> &x) const
		{
			return *this >> REMOVE_CONST(x.first) >> REMOVE_CONST(x.second);
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
		template<typename T1, typename T2>
		const OctetsStream& operator >> (const std::set<T1, T2> &x) const
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
	explicit CompactUINT(const unsigned int &i): pi(&REMOVE_CONST(i)) { }

	OctetsStream& marshal(OctetsStream &os) const
	{
		return os.compact_uint32(*pi);
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		return os.uncompact_uint32(*pi);
	}
};

class SpecCompactUINT : public Marshal
{
	unsigned int  pos;
	unsigned int  value;
	unsigned int  max;
public:
	SpecCompactUINT(unsigned int pos, unsigned int value, unsigned int max):pos(pos),value(value),max(max) {}

	OctetsStream& marshal(OctetsStream &os) const
	{
		return os.spec_compact_uint32(pos, value, max);
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		ASSERT(false);	//不支持
		return os;
	}
};

class CompactSINT : public Marshal
{
	int *pi;
public:
	explicit CompactSINT(const int &i): pi(&REMOVE_CONST(i)) { }

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
	return STLContainer<Container> (REMOVE_CONST(c));
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

template < typename T1, typename T2>
std::ostream & operator << (std::ostream & os, const std::set<T1, T2> &x)
{
	return TraceContainer(os, x);
}

};
#endif
