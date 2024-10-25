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

static GNET::Protocol::Type _state_LogNormal[] = 
{
	PROTOCOL_STATINFOVITAL,
	PROTOCOL_STATINFO,
	PROTOCOL_REMOTELOGVITAL,
	PROTOCOL_REMOTELOG,
};

GNET::Protocol::Manager::Session::State state_LogNormal(_state_LogNormal,
						sizeof(_state_LogNormal)/sizeof(GNET::Protocol::Type), 3600);


};

