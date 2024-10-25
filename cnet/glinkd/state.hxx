#ifndef __GNET_GLINKD_STATE
#define __GNET_GLINKD_STATE

#ifdef WIN32
#include "gnproto.h"
#else
#include "protocol.h"
#endif

namespace GNET
{

extern GNET::Protocol::Manager::Session::State state_GLoginServer;

extern GNET::Protocol::Manager::Session::State state_GResponseReceive;

extern GNET::Protocol::Manager::Session::State state_GKeyReestablished;

extern GNET::Protocol::Manager::Session::State state_GVerifyMatrix;

extern GNET::Protocol::Manager::Session::State state_GKeyExchgSend;

extern GNET::Protocol::Manager::Session::State state_GKeyExchgSucc;

extern GNET::Protocol::Manager::Session::State state_GRoleList;

extern GNET::Protocol::Manager::Session::State state_GRoleList_OnGame;

extern GNET::Protocol::Manager::Session::State state_GRoleListReceive;

extern GNET::Protocol::Manager::Session::State state_GSelectRoleServer;

extern GNET::Protocol::Manager::Session::State state_GSelectRoleReceive;

extern GNET::Protocol::Manager::Session::State state_GResetPlayerPosReceive;

extern GNET::Protocol::Manager::Session::State state_GCreateRoleReceive;

extern GNET::Protocol::Manager::Session::State state_GDeleteRoleReceive;

extern GNET::Protocol::Manager::Session::State state_GUndoDeleteRoleReceive;

extern GNET::Protocol::Manager::Session::State state_GReadyGame;

extern GNET::Protocol::Manager::Session::State state_GDataExchgServer;

extern GNET::Protocol::Manager::Session::State state_GDomainServer;

extern GNET::Protocol::Manager::Session::State state_GWebTradeOpReceive;

extern GNET::Protocol::Manager::Session::State state_GPhoneReceive;

extern GNET::Protocol::Manager::Session::State state_GDeliverClient;

extern GNET::Protocol::Manager::Session::State state_GProviderLinkServer;

extern GNET::Protocol::Manager::Session::State state_GFactionLinkClient;

extern GNET::Protocol::Manager::Session::State state___only_for_stub;

extern GNET::Protocol::Manager::Session::State state_Null;

};

#endif
