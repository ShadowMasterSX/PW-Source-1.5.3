#include "glinkclient.hpp"
#include "state.hxx"
#include "macros.h"
#include "timertask.h"
#include "vclient_if.h"

namespace GNET
{
GLinkClient GLinkClient::instance;
Thread::RWLock 				GLinkClient::locker_clientarray;
GLinkClient::ClientArray 	GLinkClient::clientarray;

void GLinkClient::Reconnect()
{
	Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
	backoff += (backoff >> 1);
	if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

const Protocol::Manager::Session::State* GLinkClient::GetInitState() const
{
	return &state_GLoginClient;
}

void GLinkClient::OnAddSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	if (conn_state)
	{
		Close(sid);
		return;
	}
	conn_state = true;
	this->sid = sid;
	backoff = BACKOFF_INIT;
	//TODO
	DEBUG_PRINT("GLinkClient::OnAddSession %d\n",sid);
}

void GLinkClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	//TODO
	{
		Thread::RWLock::WRScoped l(locker_clientarray);
		clientarray.erase(roleid);
	}
	VCLIENT::RemovePlayer(roleid);
	DEBUG_PRINT("VCLIENT: RemovePlayer %d\n",roleid);
	password = origin_password;	
	userid = 0;
	roleid = 0;
	Reconnect();
	DEBUG_PRINT("GLinkClient::OnDelSession %d\n",sid);
}

void GLinkClient::OnAbortSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	//TODO
	Reconnect();
	DEBUG_PRINT("GLinkClient::OnAbortSession %d\n",sid);
}

void GLinkClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

void GLinkClient::SetRole(int _roleid)
{
	if((role == -1 || role == _roleid%16) && roleid == 0)
	{
		roleid = _roleid;
		Thread::RWLock::WRScoped l(locker_clientarray);
		clientarray[roleid] = this;
	}
}

bool GLinkClient::DispatchProtocol(int roleid,const Protocol &protocol)
{
	return DispatchProtocol(roleid,&protocol);
}
bool GLinkClient::DispatchProtocol(int roleid,const Protocol *protocol)
{
	Thread::RWLock::RDScoped l(locker_clientarray);
	GLinkClient::ClientArray::iterator it=clientarray.find(roleid);
	if (it==clientarray.end()) return false;
	return (*it).second->SendProtocol(protocol);
}
bool GLinkClient::DispatchProtocol(int roleid,		Protocol &protocol)
{
	return DispatchProtocol(roleid,&protocol);
}
bool GLinkClient::DispatchProtocol(int roleid,		Protocol *protocol)
{
	Thread::RWLock::RDScoped l(locker_clientarray);
	GLinkClient::ClientArray::iterator it=clientarray.find(roleid);
	if (it==clientarray.end()) return false;
	return (*it).second->SendProtocol(protocol);
}	

void GLinkClient::OnEnterWorld(int roleid)
{
	GLinkClient::ClientArray::iterator it=clientarray.find(roleid);
	if (it!=clientarray.end()) (*it).second->enter_world = true;
}

void GLinkClient::ReportAlive()
{
	GLinkClient::ClientArray::iterator beg =clientarray.begin();
	GLinkClient::ClientArray::iterator end =clientarray.end();
	
	int total = 0;
	int enter = 0;

	for(;beg != end;++beg)
	{
		if((*beg).second->conn_state) 
		{
			++total;
			if((*beg).second->enter_world) ++enter;
		}
	}

	DEBUG_PRINT("\n");
	DEBUG_PRINT("############################################################\n");
	DEBUG_PRINT("GLinkClient::ALIVE  %d ENTERWORLD %d\n",total,enter);
	DEBUG_PRINT("############################################################\n");
	DEBUG_PRINT("\n");
}

};
