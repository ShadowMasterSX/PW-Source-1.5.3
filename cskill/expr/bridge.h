#ifndef _GNET_BRIDGE_H
#define _GNET_BRIDGE_H
#include "hashstring.h"

#include "memfunc.h"
#include <ext/hash_map>
using namespace __gnu_cxx;
namespace GNET
{
class CommonObject;

class Visitor
{
public:
	virtual void visit(CommonObject* _co)=0;
	virtual ~Visitor() { }
};

class CommonObject
{
protected:
	static inline size_t hash_string(const char* __s)
	{
		unsigned long __h = 0;
		for ( ; *__s; ++__s)
		{
			__h = 5*__h + *__s;
		}
		return size_t(__h);
	}		
	typedef hash_map<std::string,memfunc_pointer_t> Method_Map;
public:
	virtual void Accept(Visitor* _v) { _v->visit(this); }
	virtual ~CommonObject() { }
};
template<typename _type>
struct pair_t
{
	int		index;
	_type	value;
	
	pair_t(int _index,_type _value) : index(_index),value(_value) { }
};

} //end namespace
#endif
