#ifndef __VARIANT_H
#define __VARIANT_H

#include <string>
#include <vector>
#include <map>
#include <ctype.h>
#include <stdarg.h>
#include "octets.h"
#include "reference.h"

namespace GNET
{

namespace variant_helper
{

inline static void TABPRINT(int tab,const char* format,...)
{
	for (int i=0;i<tab;i++) printf("\t");
	va_list ap;
	va_start(ap,format);
	vprintf( format, ap );
	va_end(ap);
}

struct Node
{
	struct Exception { };
	struct reference : public HardReference<Node>
	{
		typedef HardReference<Node> super;
		template<typename T>
		T* GetNode()
		{
			T *node = dynamic_cast<T *>( GetObject() );
			if ( !node )
			{
				*this = T::Create();
				node = dynamic_cast<T *>( GetObject() );
			}
			return node;
		}
		reference();
		reference(const Octets &os) { Decode(os); }
		reference(Node *node) : super(node) { }
		reference(std::string s);
		reference(char* s);
		reference(long long ll);
		reference& operator() ( std::string s );
		reference& operator[] ( size_t i );
		reference& erase( std::string s );
		reference& erase( size_t i );
		bool exists( std::string s );
		size_t size()           const { return GetObject()->size(); }
		operator long long ()   const { return GetObject()->operator long long(); }
		operator std::string()  const { return GetObject()->operator std::string(); }
		reference Clone()       const { return GetObject()->Clone(); }
		void Decode(const Octets &os)
		{
			char *begin = (char *)os.begin(), *end = (char *)os.end();
			try
			{
				*this = Node::DecodeAtom(begin, end);
			}
			catch (Exception e)
			{
				*this = reference();
			}
		}
		void Encode(Octets &os) const { GetObject()->Encode(os); }
		void Dump(size_t level = 0) const { GetObject()->Dump(level); }
	};
	struct compare
	{
		bool operator() (const reference& x, const reference& y) const
		{
			return x->toString().compare(y->toString()) < 0;
		}
	};
	static reference DecodeAtom(char *&begin, char *end);
	virtual ~Node() { }
	virtual void Decode(char *&begin, char *end) = 0;
	virtual void Encode(Octets &os) const = 0;
	virtual std::string toString() const
	{
		Octets os;
		Encode(os);
		char *data = (char *)os.begin();
		return std::string( data, os.size() );
	}
	virtual size_t size() const = 0;
	virtual operator long long()   const { return 0; }
	virtual operator std::string() const { return toString(); }
	virtual void Dump(size_t level) const = 0;
	virtual reference Clone() const
	{
		Octets os;
		Encode(os);
		char *begin = (char *)os.begin(), *end = (char *)os.end();
		return DecodeAtom(begin, end);
	}
};

struct UndefNode : public Node
{
	void Decode(char *&begin, char *end) { }
	void Encode(Octets &os) const
	{
		char c = 'e';
		os.insert( os.end(), &c, &c + 1 );
	}
	static reference Create() { return reference(new UndefNode()); }
	std::string toString() const { return std::string(); }
	size_t size() const { return 0; }
	void Dump(size_t level) const { TABPRINT(level, "(undef)\n" ); }
};

struct IntNode : public Node
{
	typedef long long value_type;
	long long value;
	void Decode(char *&begin, char *end)
	{
		std::string v;
		while ( begin < end )
		{
			char c = *begin++;
			if ( isdigit(c) ) v.push_back( c );
			else if ( c == 'e' ) break;
			else throw Exception();
		}
		if ( sscanf( v.c_str(), "%lld", &value ) != 1 ) throw Exception();
	}
	void Encode(Octets &os) const
	{
		char buf[32];
		sprintf(buf, "i%llde", value);
		os.insert( os.end(), buf, buf + strlen(buf) );
	}
	IntNode() { }
	IntNode(long long ll) : value(ll) { }
	static reference Create(char *&begin, char *end)
	{
		reference node(new IntNode());
		node->Decode(begin, end);
		return node;
	}
	static reference Create(long long ll) { return reference(new IntNode(ll)); }
	std::string toString() const
	{
		char buf[32];
		sprintf(buf, "%lld", value);
		return std::string(buf);
	}
	size_t size() const { return sizeof(value); }
	operator long long () const { return value; }
	void Dump(size_t level) const { TABPRINT(level, "(i) %lld\n", value); }
};

struct StringNode : public Node
{
	typedef std::string value_type;
	std::string value;
	void Decode(char *&begin, char *end)
	{
		char *endptr = end;
		size_t len = strtoul(begin, &endptr, 10);
		if ( *endptr++ != ':' || end - endptr < (int)len) throw Exception();
		begin = endptr + len;
		value = std::string( endptr, begin );
	}
	void Encode(Octets &os) const
	{
		char buf[sizeof(size_t)];
		size_t size = value.size();
		sprintf(buf, "%zd:", size);
		os.insert( os.end(), buf, buf + strlen(buf) );
		const char *data = value.data();
		os.insert( os.end(), data, data + size );
	}
	StringNode() { }
	StringNode( std::string s ) : value(s) { }
	static reference Create(char *&begin, char *end)
	{
		reference node(new StringNode());
		node->Decode(begin, end);
		return node;
	}
	static reference Create(std::string s) { return reference(new StringNode(s)); }
	std::string toString() const { return value; }
	size_t size() const { return value.size(); }
	void Dump(size_t level) const { TABPRINT(level, "(s) %s\n", value.c_str()); }
};

struct ListNode : public Node
{
	typedef std::vector< reference > list_type;
	typedef list_type value_type;
	list_type value;
	void Decode(char *&begin, char *end)
	{
		while ( begin < end && *begin != 'e' )
			value.push_back(DecodeAtom(begin, end));
		begin++;
	}
	void Encode(Octets &os) const
	{
		char l = 'l', e = 'e';
		os.insert( os.end(), &l, &l + 1 );
		for ( list_type::const_iterator it = value.begin(), ie = value.end(); it != ie; ++it )
			(*it)->Encode(os);
		os.insert( os.end(), &e, &e + 1 );
	}
	static reference Create(char *&begin, char *end)
	{
		reference node(new ListNode());
		node->Decode(begin, end);
		return node;
	}
	static reference Create() { return reference(new ListNode()); }
	size_t size() const { return value.size(); }
	void Dump(size_t level) const
	{
		TABPRINT(level,"node type:   LIST(%d)\n",value.size() );
		for ( list_type::const_iterator it = value.begin(), ie = value.end(); it != ie; ++it )
			(*it)->Dump(level + 1);
	}
	Node::reference& operator [] ( size_t i )
	{
		if ( value.size() < i + 1 ) value.resize(i + 1);
		return value[i];
	}
	void erase( size_t i ) { if ( i < value.size() ) value.erase( value.begin() + i ); }
};

struct DictNode : public Node
{
	typedef std::map< reference, reference, Node::compare > dict_type;
	typedef dict_type value_type;
	dict_type value;
	void Decode(char *&begin, char *end)
	{
		while ( begin < end && *begin != 'e' )
		{
			reference key(DecodeAtom(begin, end));
			if ( begin < end )
				value.insert( std::make_pair( key, DecodeAtom(begin, end)));
			else
				throw Exception();
		}
		begin ++;
	}
	void Encode(Octets &os) const
	{
		char d = 'd', e = 'e';
		os.insert( os.end(), &d, &d + 1 );
		for ( dict_type::const_iterator it = value.begin(), ie = value.end(); it != ie; ++it )
		{
			(*it).first ->Encode(os);
			(*it).second->Encode(os);
		}
		os.insert( os.end(), &e, &e + 1 );
	}
	static reference Create(char *&begin, char *end)
	{
		reference node(new DictNode());
		node->Decode(begin, end);
		return node;
	}
	static reference Create() { return reference(new DictNode()); }
	size_t size() const { return value.size(); }
	void Dump(size_t level) const
	{
		TABPRINT(level,"node type:   DICT(%d)\n",value.size() );
		for ( dict_type::const_iterator it = value.begin(), ie = value.end(); it != ie; ++it )
		{
			TABPRINT(level+1,"=================\n");
			(*it).first->Dump(level + 1);
			(*it).second->Dump(level + 1);
		}
		TABPRINT(level+1,"=================\n");
	}
	Node::reference& operator () ( std::string key ) { return value[key]; }
	void erase( std::string key ) { value.erase(key); }
	bool exists ( std::string key ) { return value.find(key) != value.end(); }
};
inline Node::reference::reference() : super( UndefNode::Create() ) { }
inline Node::reference::reference(std::string s) : super( StringNode::Create(s) ) { }
inline Node::reference::reference(char* s) : super( StringNode::Create(s) ) { }
inline Node::reference::reference(long long ll) : super( IntNode::Create(ll) ) { }
inline Node::reference& Node::reference::operator() ( std::string s ) { return (*GetNode<DictNode>())(s); }
inline Node::reference& Node::reference::operator[] ( size_t i ) { return (*GetNode<ListNode>())[i]; }
inline Node::reference& Node::reference::erase ( std::string s ) { GetNode<DictNode>()->erase(s); return *this; }
inline Node::reference& Node::reference::erase ( size_t i ) { GetNode<ListNode>()->erase(i); return *this; }
inline bool Node::reference::exists ( std::string s ) { return GetNode<DictNode>()->exists(s); }

inline Node::reference Node::DecodeAtom(char *&begin, char *end)
{
	reference atom;
	switch ( *begin )
	{
	case 'i':
		atom = IntNode::Create(++begin, end);
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		atom = StringNode::Create(begin, end);
		break;
	case 'l':
		atom = ListNode::Create(++begin, end);
		break;
	case 'd':
		atom = DictNode::Create(++begin, end);
		break;
	case 'e':
		++begin;
		atom = UndefNode::Create();
		break;
	default:
		throw Exception();
	}
	return atom;
}

}; // namespace variant_helper

typedef variant_helper::Node::reference variant;

inline size_t length(const variant& var) { return var.size(); }
inline void dump(const variant& var) { var.Dump(); }

}; // namespace GNET

#endif
