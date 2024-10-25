#ifndef __GNET_GACD_STATE
#define __GNET_GACD_STATE

#ifdef WIN32
#include "gnproto.h"
#else
#include "protocol.h"
#endif

namespace GNET
{

extern GNET::Protocol::Manager::Session::State state_ACServer;

extern GNET::Protocol::Manager::Session::State state_ACControlServer;

};

#endif
