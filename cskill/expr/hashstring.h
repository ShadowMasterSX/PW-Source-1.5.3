#ifndef __HASHSTRING_H
#define __HASHSTRING_H

#include <string>
#include <ext/hash_map>

namespace __gnu_cxx
{
//template<> struct hash<std::string> { size_t operator() (const std::string &__s) const { return __stl_hash_string(__s.c_str()); } };

template<> struct hash<std::string>
{
	size_t operator() (const std::string &__s) const
	{
		const char *__p = __s.data();
		unsigned long __h = 0;
		for (int len = __s.size(); len-- > 0; )  __h = 5*__h + *__p++;
		return size_t(__h);
	}
};

}
#endif
