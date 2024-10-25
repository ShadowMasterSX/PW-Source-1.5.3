#ifndef __GNET_MAPERASER_H
#define __GNET_MAPERASER_H

#include <vector>
#include <map>

namespace GNET
{

template <typename Map>
class MapEraser
{
	typedef typename Map::iterator __iterator_type;
	typedef typename Map::key_type __key_type;
	typedef typename std::vector<__iterator_type> IteratorVector;
	typedef typename std::vector<__key_type> KeyVector;
	IteratorVector iv;
	KeyVector      kv;
	Map *map;
public:
	MapEraser(Map& _map) : map(&_map) { }
	~MapEraser()
	{
		for (typename IteratorVector::const_iterator it = iv.begin(), ie = iv.end(); it != ie; ++it) map->erase(*it);
		for (typename KeyVector::const_iterator      it = kv.begin(), ie = kv.end(); it != ie; ++it) map->erase(*it);
	}
	void push(const __iterator_type& i) { iv.push_back(i); }
	void push(const __key_type& k)      { kv.push_back(k); }
};

};
#endif

