#ifndef __GNET_MAPLINKSERVER_H
#define __GNET_MAPLINKSERVER_H

#include <map>

#include "thread.h"
#include "protocol.h"

namespace GNET
{

class LinkServer
{
	typedef std::set<unsigned int/*sessionid*/> Set;
	Set set;
	Thread::Mutex locker;
	static LinkServer	instance;
public:
	LinkServer() : locker("LinkServer::locker") { }
	~LinkServer() { }
	static LinkServer & GetInstance() { return instance; }
	size_t Size() { Thread::Mutex::Scoped l(locker);	return set.size();	}

	bool Find( int sid )
	{
		Thread::Mutex::Scoped l(locker);
		return set.end() != set.find(sid);
	}

	void Insert( int sid )
	{
		Thread::Mutex::Scoped l(locker);
		set.insert( sid );
	}

	void Erase( int sid )
	{
		Thread::Mutex::Scoped l(locker);
		set.erase( sid );
	}

	void BroadcastProtocol(const Protocol* p);
	void BroadcastProtocol(const Protocol& p) { BroadcastProtocol(&p); }
	void BroadcastProtocol(		 Protocol* p);
	void BroadcastProtocol(		 Protocol& p) { BroadcastProtocol(&p); }

};

};

#endif

