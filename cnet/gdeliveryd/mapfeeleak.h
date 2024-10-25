#ifndef __GNET_MAPFEELEAK_H
#define __GNET_MAPFEELEAK_H

#include <map>

#include "thread.h"

namespace GNET
{

class FeeLeak
{
	//fee leak map, Èç¹ûÓÃ»§µÄ¼Æ·ÑÏûÏ¢³öÏÖÒÅÂ©£¬Ôò¼ÓÈë´Ë¼¯ºÏ£¬ÏÂ´ÎµÇÂ½Ê±½«ÓÃ»§¸ÄÎªÇ¿ÖÆµÇÂ½£¬±ÜÃâ³öÏÖÖØ¸´µÇÂ½µÄÌáÊ¾
	typedef std::set<int /*userid*/> Set; 
	Set set;
	Thread::Mutex locker;
	static FeeLeak	instance;
public:
	FeeLeak() : locker("FeeLeak::locker") { }
	~FeeLeak() { }
	static FeeLeak & GetInstance() { return instance; }
	size_t Size() { Thread::Mutex::Scoped l(locker);	return set.size();	}

	bool Find( int userid )
	{
		Thread::Mutex::Scoped l(locker);
		return set.end() != set.find(userid);
	}

	void Insert( int userid )
	{
		Thread::Mutex::Scoped l(locker);
		set.insert( userid );
	}

	void Erase( int userid )
	{
		Thread::Mutex::Scoped l(locker);
		set.erase( userid );
	}

};

};

#endif

