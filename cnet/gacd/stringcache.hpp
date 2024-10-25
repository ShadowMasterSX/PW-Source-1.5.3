#ifndef __GNET_STRINGCACHE_HPP
#define __GNET_STRINGCACHE_HPP

#include <map>
#include "strinfo.h"

namespace GNET
{

class StringCache
{
	enum { DEFAULT_TIMEOUT = 600 };
    typedef std::map<int, std::pair<int, StrInfo*> > CacheMap;
	CacheMap m_cache {};
	static StringCache s_instance;
public:
	~StringCache();

	static StringCache *GetInstance() { return &s_instance; }

	void SaveString(int iUserID, StrInfo *pStrInfo);
	void LoadString(int iUserID, StrInfo *&pStrInfo);

	void OnTimer();

private:
	StringCache() { }
	StringCache(const StringCache &sc);
	StringCache& operator=(const StringCache &sc);
	bool operator==(const StringCache &sc) const;
};

};

#endif
