
#include "stringcache.hpp"
#include "gacdutil.h"

namespace GNET
{

StringCache StringCache::s_instance;

void StringCache::OnTimer()
{		
	std::vector<int> dummy;
	for(CacheMap::iterator it = m_cache.begin(), ie = m_cache.end(); it != ie; ++it)
	{
		int &tick = (*it).second.first;
		if ( tick )
		{
			if( --tick <= 0 )
			{
				dummy.push_back((*it).first);
				delete (*it).second.second;
			}
		}
	}
	for(std::vector<int>::const_iterator it = dummy.begin(), ie = dummy.end(); it != ie; ++it)
		m_cache.erase((*it));
}

StringCache::~StringCache()
{
	for(CacheMap::iterator it = m_cache.begin(), ie = m_cache.end(); it != ie; ++it)
		delete (*it).second.second;
	m_cache.clear();
}

void StringCache::SaveString(int iUserID, StrInfo *pStrInfo)
{
	DEBUG_PRINT_DEBUG("user %d save strinfo 0x%08x into cache\n", iUserID, pStrInfo);
	if( pStrInfo == NULL ) 
	{
#ifdef GACD_DEBUG_PRINT_DEBUG
		GACD_STD_DEBUG_PRINT("user %dsave null string pointer\n", iUserID);
		exit(0);
#endif
		return;
	}

	CacheMap::iterator it = m_cache.find(USERID2ACCOUNTID(iUserID));
	if( it != m_cache.end() )
	{
		DEBUG_PRINT_DEBUG("multi cache\n");
		if( (*it).second.second == pStrInfo )
		{
			DEBUG_PRINT_ERROR("save string cache failed\n");
			exit(0);
		}
		delete (*it).second.second;
		(*it).second.second = pStrInfo;
		(*it).second.first = DEFAULT_TIMEOUT;
	}
	else
		m_cache[USERID2ACCOUNTID(iUserID)] = std::make_pair(DEFAULT_TIMEOUT, pStrInfo);
}
void StringCache::LoadString(int iUserID, StrInfo *&pStrInfo)
{
	delete pStrInfo;
	pStrInfo = NULL;
	CacheMap::iterator it = m_cache.find(USERID2ACCOUNTID(iUserID));
	if( it != m_cache.end() )
	{
		pStrInfo = (*it).second.second;
		m_cache.erase(it);
	}
	if( pStrInfo == NULL ) 
	{
		pStrInfo = new StrInfo();
		DEBUG_PRINT_DEBUG("cache miss, make new str info\n");
	}
#ifdef GACD_DEBUG_PRINT_DEBUG
	if( pStrInfo == NULL )
	{
		GACD_STD_DEBUG_PRINT("user %d load null string pointer\n", iUserID);
		exit(0);
	}
#endif
	DEBUG_PRINT_DEBUG("user %d load strinfo 0x%08x from cache\n", iUserID, pStrInfo);
}

};

