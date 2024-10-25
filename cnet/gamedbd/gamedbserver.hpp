#ifndef __GNET_GAMEDBSERVER_HPP
#define __GNET_GAMEDBSERVER_HPP

#include "protocol.h"

namespace GNET
{

class CrsLogicuidManager
{
public:
	CrsLogicuidManager() : startid_(0), busy_(false) { }

	void FindFreeLogicuid();
	int AllocLogicuid();

	static CrsLogicuidManager* GetInstance() { return &_instance; }
private:
	static CrsLogicuidManager _instance; 
	Thread::Mutex mutex_;
	std::deque<int> idset_;
	int startid_;
	bool busy_;
};

class CrsLogicuidSeeker : public Thread::Runnable
{
public:
	CrsLogicuidSeeker(int prior=1) : Runnable(prior) { }
	void Run();
};

class GameDBServer : public Protocol::Manager
{
	static GameDBServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	Thread::Mutex	lockerip;
	std::map<Session::ID, int> ipmap;
	void OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer);
public:
	static GameDBServer *GetInstance() { return &instance; }
	std::string Identification() const { return "GameDBServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GameDBServer() : accumulate_limit(0) { delivery_sid=0; }
	bool Send2Delivery(const Protocol* p) {
		if ( !delivery_sid ) return false;
		return Send(delivery_sid,p);
	}
	bool Send2Delivery(      Protocol* p) {
		if ( !delivery_sid ) return false;
		return Send(delivery_sid,p);
	}
	bool Send2Delivery(const Protocol& p) {
		return Send2Delivery(&p);
	}
	bool Send2Delivery(      Protocol& p) {
		return Send2Delivery(&p);
	}
	int GetSessionIP(Session::ID sid)
	{
		Thread::Mutex::Scoped l(lockerip);
		return ipmap[sid];
	}
        int SetSessionIP(Session::ID sid, int ip)
	{       
		Thread::Mutex::Scoped l(lockerip);
		ipmap[sid] = ip;
		return ipmap[sid];
	}
	unsigned int delivery_sid;
};

};
#endif
