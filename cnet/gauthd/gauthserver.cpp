
#include "gauthserver.hpp"
#include "state.hxx"

namespace GNET
{

GAuthServer GAuthServer::instance;

const Protocol::Manager::Session::State* GAuthServer::GetInitState() const
{
	return &state_GAuthServer;
}

void GAuthServer::OnAddSession(Session::ID sid)
{
	//TODO
}

void GAuthServer::OnDelSession(Session::ID sid)
{
	//TODO
	Thread::Mutex::Scoped l(locker_zonemap);
	zonemap.erase(sid);
}

bool GAuthServer::ValidUser(Session::ID sid,int userid)
{
	Thread::RWLock::RDScoped l(locker_map);
	UserMap::iterator it=usermap.find(userid);
	if (it==usermap.end()) return false;
	if ((*it).second.sid != sid) return false;	
	return true;
}

};
