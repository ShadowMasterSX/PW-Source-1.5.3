#ifndef __GNET_MAP_H
#define __GNET_MAP_H

#include "common/platform.h"

#include <map>

#undef gnet_map

#ifdef USE_HASH_MAP
#include <ext/hash_map>
#define gnet_map __gnu_cxx::hash_map 
namespace __gnu_cxx
{
template<typename T> struct hash<T*>
{
  size_t operator()(T *__x) const { return (size_t)__x; }
};

template<typename T> struct hash<const T*>
{
  size_t operator()(const T *__x) const { return (size_t)__x; }
};
};
#else
#define gnet_map std::map
#endif

#endif
