#ifndef __GNET_CENTRALDELIVERYSERVER_HPP
#define __GNET_CENTRALDELIVERYSERVER_HPP

#include "protocol.h"
#include "serverload.h"

namespace GNET
{

class CentralDeliveryServer : public Protocol::Manager
{
public:
	enum
	{
		CRS_TTL = 120,//300,
	};
	
	struct delivery_t
	{
		unsigned int sid;
		unsigned int crs_ttl;
		ServerLoad cache_server_load; 
	};	
	typedef std::map<int/*zoneid*/, delivery_t> DSMap;
	typedef std::set<int> AcceptedZoneSet;

	friend class CrsSvrCheckTimer;

private:
	static CentralDeliveryServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);

	DSMap ds_map;
	AcceptedZoneSet accepted_zone_set; //用来记录可接受的zone id列表

public:
	static CentralDeliveryServer *GetInstance() { return &instance; }
	std::string Identification() const { return "CentralDeliveryServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	CentralDeliveryServer() : accumulate_limit(0) { }
	
	bool IsConnect(int zoneid)
	{
		return (ds_map.find(zoneid) != ds_map.end());
	}
	
	void InsertZoneId(int zoneid, unsigned int sid)
	{
		ds_map[zoneid].sid = sid;
	}
	
	void SetLoad(int zoneid, int srv_limit, int srv_count);
	int InitAcceptedZoneList( std::string list_str );
	
	bool IsAcceptedZone(int zoneid)
	{
		return (accepted_zone_set.find(zoneid) != accepted_zone_set.end());
	}
	
	void GetAcceptedZone(std::vector<int>& zone_list)
	{
		for(AcceptedZoneSet::iterator it = accepted_zone_set.begin(); it != accepted_zone_set.end(); ++it) {
			zone_list.push_back(*it);
		}
	}
	
	void BroadcastProtocol(const Protocol* protocol)
	{
		for(DSMap::const_iterator it = ds_map.begin(); it != ds_map.end(); ++it) {
			Send((*it).second.sid, protocol);
		}
	}
	
	void BroadcastProtocol(const Protocol& protocol)
	{
		return BroadcastProtocol(&protocol);
	}
	
	bool DispatchProtocol(int zoneid, const Protocol* protocol)
	{
		DSMap::const_iterator it = ds_map.find(zoneid);
		if(it == ds_map.end()) return false;
		
		return Send((*it).second.sid, protocol);
	}
	
	bool DispatchProtocol(int zoneid, const Protocol& protocol)
	{
		return DispatchProtocol(zoneid, &protocol);
	}
};

class LoadExchangeTask: public Thread::Runnable
{
private:
	int interval;
	
public:
	LoadExchangeTask(int _interval, int _prior=1) : Runnable(_prior), interval(_interval) {}
	void Run();
};

class CrsSvrCheckTimer : public Thread::Runnable
{
	int update_time;
public:
	CrsSvrCheckTimer(int _time,int _proir=1) : Runnable(_proir),update_time(_time) { }
	void Run();
private:
	void CheckConnection();
};

};
#endif
