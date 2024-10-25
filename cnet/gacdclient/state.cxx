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

static GNET::Protocol::Type _state_ACControlClient[] = 
{
	PROTOCOL_ACCONNECTRE,
	PROTOCOL_ACRELOADCONFIGRE,
	PROTOCOL_ACREMOTEEXERE,
	PROTOCOL_ACQLOGINFO,
	PROTOCOL_ACQUERY,
	PROTOCOL_ACQUSERSTRS,
	PROTOCOL_ACQUSERONLINE,
	PROTOCOL_ACQMOUSEINFO,
	PROTOCOL_ACQTHREADTIMES,
	PROTOCOL_ACQPROTOCOLSTATS,
	PROTOCOL_ACQSTROWNER,
	PROTOCOL_ACCHEATERS,
	PROTOCOL_ACQPATTERNS,
	PROTOCOL_ACFORBIDPROCESS,
	PROTOCOL_ACQPLATFORMINFO,
	PROTOCOL_ACBRIEFINFO,
	PROTOCOL_ACPERIODINFO,
	PROTOCOL_ACQCODERES,
	PROTOCOL_ACVERSION,
};

GNET::Protocol::Manager::Session::State state_ACControlClient(_state_ACControlClient,
						sizeof(_state_ACControlClient)/sizeof(GNET::Protocol::Type), 86400);


};

