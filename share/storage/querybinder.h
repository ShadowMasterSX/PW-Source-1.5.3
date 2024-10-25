#ifndef __GNET_QUERYBINDER_H
#define __GNET_QUERYBINDER_H

#include <list>

#include "storage.h"

namespace GNET
{

class QueryBinder : public StorageEnv::IQuery
{
	typedef std::list<IQuery*>	QueryList;
	QueryList	m_queries;
	bool		m_stopifany;
public:
	QueryBinder( bool stopifany = true ) : m_stopifany(stopifany) { }
	QueryBinder & Bind( IQuery * q )
	{
		m_queries.push_back(q);
		return *this;
	}
	bool Update( StorageEnv::Transaction& txn, Octets & key, Octets & value )
	{
		size_t oldsize = m_queries.size();

		QueryList::iterator it = m_queries.begin();
		while( it != m_queries.end() )
		{
			if( (*it)->Update( txn, key, value ) )
			{
				++ it;
			}
			else
			{
				QueryList::iterator ite = it;
				++ it;
				m_queries.erase(ite);
			}
		}

		if( m_stopifany && oldsize > m_queries.size() )
			return false;
		return (m_queries.size() > 0);
	}
};

}

#endif

