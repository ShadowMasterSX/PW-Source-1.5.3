#include "callid.hxx"

#ifdef WIN32
#include <winsock2.h>
#include "gnproto.h"
#include "gncompress.h"
#else
#include "protocol.h"
#include "binder.h"
#endif

namespace GNET
{

static GNET::Protocol::Type _state_UniqueNameServer[] = 
{
	RPC_PRECREATEROLE,
	RPC_POSTCREATEROLE,
	RPC_POSTDELETEROLE,
	RPC_PRECREATEFACTION,
	RPC_POSTCREATEFACTION,
	RPC_POSTDELETEFACTION,
	RPC_ROLENAMEEXISTS,
	RPC_USERROLECOUNT,
	RPC_MOVEROLECREATE,
	RPC_PRECREATEFAMILY,
	RPC_POSTCREATEFAMILY,
	RPC_POSTDELETEFAMILY,
	RPC_DBRAWREAD,
	PROTOCOL_KEEPALIVE,
	RPC_PREPLAYERRENAME,
	PROTOCOL_POSTPLAYERRENAME,
	RPC_PREFACTIONRENAME,
	PROTOCOL_POSTFACTIONRENAME,
};

GNET::Protocol::Manager::Session::State state_UniqueNameServer(_state_UniqueNameServer,
						sizeof(_state_UniqueNameServer)/sizeof(GNET::Protocol::Type), 86400);


};

