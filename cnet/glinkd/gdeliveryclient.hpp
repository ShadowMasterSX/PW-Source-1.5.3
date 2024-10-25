#ifndef __GNET_GDELIVERYCLIENT_HPP
#define __GNET_GDELIVERYCLIENT_HPP

#include "protocol.h"
#include "thread.h"
#include "localmacro.h"

namespace GNET
{

class GDeliveryClient : public Protocol::Manager
{
	static GDeliveryClient instance;
	size_t		accumulate_limit;
	Session::ID	sid;
	bool		conn_state;
	Thread::Mutex	locker_state;
	enum { BACKOFF_INIT = 2, BACKOFF_DEADLINE = 256 };
	size_t		backoff;
	char link_type;
	void Reconnect();
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	void OnAbortSession(Session::ID sid);
	void OnCheckAddress(SockAddr &) const;
public:
	static GDeliveryClient *GetInstance() { return &instance; }
	std::string Identification() const { return "GDeliveryClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GDeliveryClient() : accumulate_limit(0), conn_state(false),locker_state("GDeliveryClient::conn_state"), backoff(BACKOFF_INIT),link_type(-1) { }

	inline void SetLinkType(char type) {link_type = type;}
	inline bool IsPhoneLink() const { return LINK_TYPE_PHONE == link_type; }

	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(		Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(		Protocol *protocol) { return conn_state && Send(sid, protocol); }

};

};
#endif
