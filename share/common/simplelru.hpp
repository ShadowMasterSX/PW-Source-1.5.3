
#ifndef __GNET_SIMPLELRU_HPP__
#define __GNET_SIMPLELRU_HPP__

#include <iostream>
#include <list>
#include <map>

namespace GNET
{

template <typename Key, typename Data, typename Compare = std::less<Key> >
class simplelru
{
	typedef std::list<std::pair<Key, Data> > List;
	typedef std::map<Key, typename List::iterator, Compare> Map;

	List list;
	Map map;
	size_t maxsize;

	void purge()
	{
		if ( list.size() >= maxsize )
		{
			map.erase( list.front().first );
			list.pop_front();
		}
	}
public:	
	explicit simplelru( size_t _maxsize ) : maxsize(_maxsize) { }

	void put( Key key, const Data & data )
	{
		typename List::iterator it = list.insert(list.end(), std::make_pair( key, data ));
		std::pair<typename Map::iterator, bool> r = map.insert(std::make_pair(key, it) );
		if ( !r.second )
		{
			list.erase(r.first->second);
			r.first->second = it;
		}
		else
			purge();
	}

	bool find( Key key, Data &data )
	{
		typename Map::iterator it = map.find(key);
		if ( it == map.end() )
			return false;
		data = it->second->second;
		list.splice( list.end(), list, it->second );
		return true;
	}

	bool exist( Key key )
	{
		typename Map::iterator it = map.find(key);
		if ( it == map.end() )
			return false;
		list.splice( list.end(), list, it->second );
		return true;
	}

	void del( Key key )
	{
		typename Map::iterator it = map.find(key);
		if ( it == map.end() )
			return;
		list.erase(it->second);
		map.erase(it);
	}

	void dump()
	{
		std::cout << "lru[";
		for ( typename List::iterator it = list.begin(), ie = list.end(); it != ie; ++it)
			std::cout << it->first << "-" << it->second << ",";
		std::cout << "]" << std::endl;
	}
};

} // namespace GNET

#endif //__GNET_SIMPLELRU_HPP__
