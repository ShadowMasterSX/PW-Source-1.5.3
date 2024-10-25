#ifndef __GNET_GCLIENT_STATE
#define __GNET_GCLIENT_STATE

#ifdef WIN32
#include "gnproto.h"
#else
#include "protocol.h"
#endif

namespace GNET
{

extern GNET::Protocol::Manager::Session::State state_GLoginClient;

extern GNET::Protocol::Manager::Session::State state_GSelectRoleClient;

extern GNET::Protocol::Manager::Session::State state_GDataExchgClient;

};

#endif
