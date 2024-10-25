#ifndef __GNET_GANTICHEATERCLIENT_HPP
#define __GNET_GANTICHEATERCLIENT_HPP

#include "protocol.h"
#include "thread.h"

namespace GNET
{

class GAntiCheaterClient : public Protocol::Manager
{
	enum { SERVER_COUNT = 2 };
	static GAntiCheaterClient instance[SERVER_COUNT];
	static size_t GetNo() { static size_t cc_id = 0; return cc_id++; }
	size_t		accumulate_limit;
	Session::ID	sid;
	bool		conn_state;
	enum { BACKOFF_INIT = 2, BACKOFF_DEADLINE = 8 };
    size_t      backoff;
    void Reconnect();
	Thread::Mutex	locker_state;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);
	void OnAbortSession(const SockAddr &sa);
	void OnCheckAddress(SockAddr &) const;
	size_t c_id;
	int a_id;
	std::string ident;
	//enum { DATA_TIMEOUT = 300, FRAG_SIZE = 2048 };
	//size_t GetNextID() { static size_t init = 1; return init++; }
	//typedef std::map<int, std::pair<int, std::vector<Octets> > > DataMap;
	//DataMap data_map;
	static std::map<int, size_t> area_map;
	std::map<int,std::pair<int,std::pair<size_t,int> > > brief_map;
public:
	void SetBrief(int b, const std::pair<int,std::pair<size_t,int> > p) { brief_map[b] = p; }
	std::pair<int,std::pair<size_t,int> > GetBrief(int b) { return brief_map[b]; }
	//void ParseData(OctetsBuffer &data);
	//void SendData(const Octets &data);
	//void DoTimer();
	//void PutData(size_t id, size_t seq, size_t total, const Octets &data);
	static size_t GetServerCount() { return SERVER_COUNT; }
	size_t GetServerNo() const { return c_id; }
	bool IsConnecting() const { return conn_state == true && a_id != -1; }
	void SetAreaID(int _aid) { a_id = _aid; area_map[a_id] = c_id; }
	int GetAreaID() const { return a_id; }
	static GAntiCheaterClient *GetInstance(size_t id) { return &(instance[id]); }
	static GAntiCheaterClient *GetInstance() { return &(instance[0]); }
	static GAntiCheaterClient *GetClientByArea(int _aid) { return GetInstance(area_map[_aid]); }
	static void SendProtocol(const Protocol &protocol, int _aid);
	static void SendProtocol(const Protocol *protocol, int _aid);
	std::string Identification() const { 
		char buffer[16];
		sprintf(buffer, "%d", c_id);
		return "GAntiCheaterClient" + std::string(buffer); 
	}	
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	GAntiCheaterClient();

	bool SendProtocol(const Protocol &protocol) { return conn_state && Send(sid, protocol); }
	bool SendProtocol(const Protocol *protocol) { return conn_state && Send(sid, protocol); }
};

};
#endif
