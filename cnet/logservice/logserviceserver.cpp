
#include "logserviceserver.hpp"
#include "state.hxx"

#include "log.h"

namespace GNET
{

LogserviceServer LogserviceServer::instance;

const Protocol::Manager::Session::State* LogserviceServer::GetInitState() const
{
	return &state_LogNormal;
}

void LogserviceServer::OnAddSession(Session::ID sid)
{
	//TODO
	Log::log( LOG_INFO, "OnAddSession\n" );
}

void LogserviceServer::OnDelSession(Session::ID sid)
{
	//TODO
	Log::log( LOG_INFO, "OnDelSession\n" );
}

};
