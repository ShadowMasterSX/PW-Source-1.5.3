#ifndef __GNET_GAMED_STATE
#define __GNET_GAMED_STATE

#ifdef WIN32
#include "gnproto.h"
#else
#include "protocol.h"
#endif

namespace GNET
{

extern GNET::Protocol::Manager::Session::State state_GProviderClient;

};

#endif
