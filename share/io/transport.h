#ifndef __GNET_TRANSPORT_H
#define __GNET_TRANSPORT_H

#include "protocol.h"
#include "taskgraph.h"

namespace GNET
{

class Transport
{
	Protocol::Manager *manager;
	Protocol::Manager::Session::ID sid;
public:
	Transport() : manager(NULL), sid(0) { }
	Transport(Protocol::Manager *m, Protocol::Manager::Session::ID s) : manager(m), sid(s) { }
	bool Send(const Protocol &protocol) const { return manager ? manager->Send(sid, protocol) : false; }
	bool Send(const Protocol *protocol) const { return manager ? manager->Send(sid, protocol) : false; }
	bool operator() (const Protocol &protocol) const { return manager ? manager->Send(sid, protocol) : false; }
	bool operator() (const Protocol *protocol) const { return manager ? manager->Send(sid, protocol) : false; }
	bool operator < (const Transport &rhs) const { return sid < rhs.sid; }
	const Protocol::Manager* GetManager() const  { return manager; }
	Protocol::Manager::Session::ID GetID() const { return sid; }
};

class TransportContext : virtual public Thread::TaskContext
{
	Transport transport;
public:
	const Transport& GetTransport() const { return transport; }
	Transport& GetTransport() { return transport; }
	void SetTransport(const Transport& tp) { transport = tp; }
};

};
#endif
