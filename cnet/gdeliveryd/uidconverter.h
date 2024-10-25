#ifndef __GNET_UIDCONVERTER_H
#define __GNET_UIDCONVERTER_H

#include <map>
#include "thread.h" 
#include "macros.h"

namespace GNET
{

class UidConverter
{
public:
	int Uid2Luid(int userid)
	{
		Thread::RWLock::RDScoped l(locker);
		UidIterator it = uid2luid.find(userid);
		if(it!=uid2luid.end())
			return it->second;
		return 0;
	}

	int Luid2Uid(int logicuid)
	{
		Thread::RWLock::RDScoped l(locker);
		UidIterator it = luid2uid.find(logicuid);
		if(it!=luid2uid.end())
			return it->second;
		return 0;
	}
	int Roleid2Uid(int roleid)
	{
		Thread::RWLock::RDScoped l(locker);
		UidIterator it = luid2uid.find(LOGICUID(roleid));
		if(it!=luid2uid.end())
			return it->second;
		return 0;
	}

	void Insert(int userid, int logicuid)
	{
		Thread::RWLock::WRScoped l(locker);
		uid2luid[userid] = logicuid;
		luid2uid[logicuid] = userid;
	}
	void Erase(int userid, int logicuid)
	{
		Thread::RWLock::WRScoped l(locker);
		uid2luid.erase(userid);	
		luid2uid.erase(logicuid);	
	}

	UidConverter() { }
	~UidConverter() { }

	typedef std::map<int, int>  UidMap;
	typedef std::map<int, int>::iterator  UidIterator;
	static UidConverter & Instance() { static UidConverter instance; return instance; }
private:
	UidMap uid2luid;
	UidMap luid2uid;
	
	Thread::RWLock locker;	
};

};

#endif

