#ifndef __GNET_GLINKCLIENT_HPP
#define __GNET_GLINKCLIENT_HPP

#include "protocol.h"
#include "thread.h"
#include "timersender.h"
#include "keepalive.hpp"
namespace GNET
{
class GLinkClient : public Protocol::Manager
{
	typedef std::map<int/*roleid*/,GLinkClient*> ClientArray;
	static Thread::RWLock locker_clientarray;
	static ClientArray clientarray;

	static GLinkClient instance;	//Ã»ÓÃµÄ

	size_t		accumulate_limit;
	Session::ID	sid;
	bool		conn_state;
	bool            enter_world;
	Thread::Mutex	locker_state;
	enum { BACKOFF_INIT = 32, BACKOFF_DEADLINE = 256 };
	size_t		backoff;
	void Reconnect();
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	void OnAbortSession(Session::ID sid);
	void OnCheckAddress(SockAddr &) const;
	
//	TimerSender timer_sender;
public:
	static GLinkClient *GetInstance() { return &instance; }
	std::string Identification() const { return "GLinkClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GLinkClient() : 
		accumulate_limit(0),
		conn_state(false),enter_world(false),
		locker_state("glinkclient::locker_state"),
		backoff(BACKOFF_INIT),
//		timer_sender(this) ,
		locker_rolenamemap("glinkclient::locker_rolenamemap"),
		role(-1),userid(0),roleid(0)
	{ }
	

	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, &protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }

//	void SetTimerSenderSize(size_t size) { timer_sender.SetSizeLimit(size); }
//	void RunTimerSender(size_t ticks=1) { timer_sender.Run(ticks); }
//	void AccumulateSend(Protocol *protocol) { if (conn_state) timer_sender.SendProtocol(sid,protocol); }
	
	Session::ID GetActiveSid() { return sid; }
	
	void SetUser(int _userid){ userid = _userid; }
	void SetRole(int _roleid);
	static bool DispatchProtocol(int roleid,const Protocol &protocol);
	static bool DispatchProtocol(int roleid,const Protocol *protocol);	
	static bool DispatchProtocol(int roleid,	  Protocol &protocol);
	static bool DispatchProtocol(int roleid,	  Protocol *protocol);	

	static void OnEnterWorld(int roleid);	
	static void ReportAlive();

	typedef std::map<int/*roleid*/,Octets> RolenameMap;
	Thread::Mutex locker_rolenamemap;	
	RolenameMap rolenamemap;
	
	Octets identity;
	Octets origin_password;
	Octets password;
	int role;
	int userid;
	int	roleid;
	int tid;  //transaction id
};
class KeepAliveTask : public Thread::Runnable
{
	GLinkClient * manager;
	unsigned int delay;
public:
	KeepAliveTask(GLinkClient * _manager, unsigned int _delay,unsigned int priority=1) : Runnable(priority),manager(_manager),delay(_delay) { } 
	~KeepAliveTask() { }
	void Run()
	{
		manager->SendProtocol(KeepAlive((unsigned int)PROTOCOL_KEEPALIVE));
		Thread::HouseKeeper::AddTimerTask(this,delay);
	}	
};
class ReportAliveTask : public Thread::Runnable
{
	unsigned int delay;
public:	
	ReportAliveTask(unsigned int _delay) : Runnable(1),delay(_delay) { }
	void Run()
	{
		GLinkClient::ReportAlive();		
		Thread::HouseKeeper::AddTimerTask(this,delay);
	}
};


};
#endif
