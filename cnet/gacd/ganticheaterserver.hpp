#ifndef __GNET_GANTICHEATERSERVER_HPP
#define __GNET_GANTICHEATERSERVER_HPP

#include "protocol.h"
#include "macros.h"

namespace GNET
{

class GAntiCheaterServer : public Protocol::Manager
{
	static GAntiCheaterServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	Thread::Mutex clients_locker;
	Session::ID deliver_sid;
	//enum { DATA_TIMEOUT = 300, FRAG_SIZE = 2048 };
    //size_t GetNextID() { static size_t init = 1; return init++; }
    //typedef std::map<int, std::pair<int, std::vector<Octets> > > DataMap;
    //DataMap data_map;
public:
	//void ParseData(OctetsBuffer &data);
    //void SendData(const Octets &data);
    //void TimerUpdate();
    //void PutData(size_t id, size_t seq, size_t total, const Octets &data);
	static GAntiCheaterServer *GetInstance() { return &instance; }
	std::string Identification() const { return "GAntiCheaterServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GAntiCheaterServer() : accumulate_limit(0), clients_locker("ganticheaterserver locker"), deliver_sid((unsigned int)-1) { }
	void ResetSource();
	void OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer);
	bool SendProtocol(const Protocol* pro);
	bool SendProtocol(const Protocol& pro);
	bool SendProtocol(const Protocol* pro, Session::ID sid);
	bool SendProtocol(const Protocol& pro, Session::ID sid);
	void SetDeliverSID(Session::ID sid); 

};

};
#endif
