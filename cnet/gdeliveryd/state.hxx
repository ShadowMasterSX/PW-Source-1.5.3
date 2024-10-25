#ifndef __GNET_GDELIVERYD_STATE
#define __GNET_GDELIVERYD_STATE

#ifdef WIN32
#include "gnproto.h"
#else
#include "protocol.h"
#endif

namespace GNET
{

extern GNET::Protocol::Manager::Session::State state_GDeliverServer;

extern GNET::Protocol::Manager::Session::State state_GAuthClient;

extern GNET::Protocol::Manager::Session::State state_GProviderDeliveryServer;

extern GNET::Protocol::Manager::Session::State state_GameDBClient;

extern GNET::Protocol::Manager::Session::State state_UniqueNameClient;

extern GNET::Protocol::Manager::Session::State state_GRoleDBClient;

extern GNET::Protocol::Manager::Session::State state_GFactionDeliveryClient;

extern GNET::Protocol::Manager::Session::State state_ACClient;

extern GNET::Protocol::Manager::Session::State state_GWebTradeClient;

extern GNET::Protocol::Manager::Session::State state_GameTalkClient;

extern GNET::Protocol::Manager::Session::State state_SNSClient;

extern GNET::Protocol::Manager::Session::State state_CentralDeliveryClient;

extern GNET::Protocol::Manager::Session::State state_CentralDeliveryServer;

};

#endif
