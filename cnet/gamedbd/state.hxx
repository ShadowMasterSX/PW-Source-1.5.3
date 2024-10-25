#ifndef __GNET_GAMEDBD_STATE
#define __GNET_GAMEDBD_STATE

#ifdef WIN32
#include "gnproto.h"
#else
#include "protocol.h"
#endif

namespace GNET
{

extern GNET::Protocol::Manager::Session::State state_GameDBServer;

};

#endif
