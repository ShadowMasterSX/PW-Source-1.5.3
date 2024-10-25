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

static GNET::Protocol::Type _state_ACServer[] = 
{
	PROTOCOL_ACREPORT,
	PROTOCOL_ACANSWER,
	PROTOCOL_ACWHOAMI,
	PROTOCOL_ACPROTOSTAT,
	PROTOCOL_GMKICKOUTUSER_RE,
	PROTOCOL_ACSTATUSANNOUNCE,
	PROTOCOL_ACSTATUSANNOUNCE2,
	PROTOCOL_ACREPORTCHEATER,
	PROTOCOL_ACTRIGGERQUESTION,
	PROTOCOL_ACACCUSE,
};

GNET::Protocol::Manager::Session::State state_ACServer(_state_ACServer,
						sizeof(_state_ACServer)/sizeof(GNET::Protocol::Type), 86400);

static GNET::Protocol::Type _state_ACControlServer[] = 
{
	PROTOCOL_ACWHOAMI,
	PROTOCOL_ACRELOADCONFIG,
	PROTOCOL_ACREMOTEEXE,
	PROTOCOL_ACQUERY,
	PROTOCOL_ACSENDCODE,
};

GNET::Protocol::Manager::Session::State state_ACControlServer(_state_ACControlServer,
						sizeof(_state_ACControlServer)/sizeof(GNET::Protocol::Type), 86400);


};

