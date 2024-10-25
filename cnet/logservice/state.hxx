#ifndef __GNET_LOGSERVICE_STATE
#define __GNET_LOGSERVICE_STATE

#ifdef WIN32
#include "gnproto.h"
#else
#include "protocol.h"
#endif

namespace GNET
{

extern GNET::Protocol::Manager::Session::State state_LogNormal;

};

#endif
