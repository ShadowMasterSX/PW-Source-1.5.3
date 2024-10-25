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

static GNET::Protocol::Type _state_LogNull[] = 
{
};

GNET::Protocol::Manager::Session::State state_LogNull(_state_LogNull,
						sizeof(_state_LogNull)/sizeof(GNET::Protocol::Type), 3600);


};

