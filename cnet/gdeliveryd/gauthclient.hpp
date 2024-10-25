#ifndef __GNET_GAUTHCLIENT_HPP
#define __GNET_GAUTHCLIENT_HPP

#include "protocol.h"
#include "thread.h"
#include "timersender.h"
namespace GNET
{

class GAuthClient : public Protocol::Manager
{
	static GAuthClient instance;
	size_t		accumulate_limit;
	Session::ID	sid;
	bool		conn_state;
	Thread::Mutex	locker_state;
	enum { BACKOFF_INIT = 2, BACKOFF_DEADLINE = 256 };
	size_t		backoff;
	int version;
	void Reconnect();
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	void OnAbortSession(Session::ID sid);
	void OnCheckAddress(SockAddr &) const;
	TimerSender timersender;

	bool blreset; //whether clear all onlineuser of this zoneid on Auth server
public:
	static GAuthClient *GetInstance() { return &instance; }
	std::string Identification() const { return "GAuthClient"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	void SetVersion(int ver) { version = ver; }
	int GetVersion() { return version; }
	void IdentifyFailed();
	GAuthClient() : 
		accumulate_limit(0),
	   	conn_state(false), 
		locker_state("GAuthClient::locker_state"),
		backoff(BACKOFF_INIT),
		version(0),
		timersender(this),
		au_cert(true),
		bl_keepalive(false),
		identify_failed(false)
	{ blreset=true; } 
	
	Octets shared_key;
	bool au_cert;				//是否进行AU证书认证
	bool bl_keepalive;			//是否AU证书认证通过
	Octets osec_key;
	Octets authd_cert;	
	bool identify_failed;
	
	bool GetBlreset(){ return blreset; }
	void SetBlreset(bool b){ blreset = b; }
	size_t GetActiveSid() { return sid; }
//	void RunTimerSender(size_t ticks=1) { timersender.Run(ticks); }
//	void AccumulateSend(Protocol *protocol) { if (conn_state) timersender.SendProtocol(sid,protocol); }

	bool SendProtocol(const Protocol &protocol){ return SendProtocol(&protocol); }
	bool SendProtocol(const Protocol *protocol);
	bool SendProtocol(		Protocol &protocol){ return SendProtocol(&protocol); }
	bool SendProtocol(		Protocol *protocol);
	void OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer);

};
};
#endif
