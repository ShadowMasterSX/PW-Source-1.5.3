#ifndef __GNET_GFACTION_STATE
#define __GNET_GFACTION_STATE

#ifdef WIN32
#include "gnproto.h"
#else
#include "protocol.h"
#endif

namespace GNET
{

extern GNET::Protocol::Manager::Session::State state_GFactionServer;

extern GNET::Protocol::Manager::Session::State state_GProviderFactionServer;

extern GNET::Protocol::Manager::Session::State state_GFactionDBClient;

extern GNET::Protocol::Manager::Session::State state_UniqueNameClient;

};

#endif
