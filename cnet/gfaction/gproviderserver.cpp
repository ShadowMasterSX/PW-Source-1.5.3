
#include "gproviderserver.hpp"
#include "state.hxx"

namespace GNET
{

GProviderServer GProviderServer::instance;

const Protocol::Manager::Session::State* GProviderServer::GetInitState() const
{
	return &state_GProviderFactionServer;
}

void GProviderServer::OnAddSession(Session::ID sid)
{
	//TODO
}

void GProviderServer::OnDelSession(Session::ID sid)
{
	//TODO
	UnRegisterSession(sid);
}

};
