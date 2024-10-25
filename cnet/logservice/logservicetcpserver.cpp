
#include "logservicetcpserver.hpp"
#include "state.hxx"

#include "log.h"

namespace GNET
{

LogserviceTcpServer LogserviceTcpServer::instance;

const Protocol::Manager::Session::State* LogserviceTcpServer::GetInitState() const
{
	return &state_LogNormal;
}

void LogserviceTcpServer::OnAddSession(Session::ID sid)
{
	//TODO
	Log::log( LOG_INFO, "TcpManager OnAddSession\n" );
}

void LogserviceTcpServer::OnDelSession(Session::ID sid)
{
	//TODO
	Log::log( LOG_INFO, "TcpManager OnDelSession\n" );
}

};
