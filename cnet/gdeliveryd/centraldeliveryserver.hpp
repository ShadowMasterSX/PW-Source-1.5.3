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
	typedef std::map<int/*zoneid*/,int/*groupid*/> AcceptedZoneMap;

	friend class CrsSvrCheckTimer;

private:
	static CentralDeliveryServer instance;
	size_t		accumulate_limit;
	const Session::State *GetInitState() const;
	bool OnCheckAccumulate(size_t size) const { return accumulate_limit == 0 || size < accumulate_limit; }
	void OnAddSession(Session::ID sid);
	void OnDelSession(Session::ID sid);

	DSMap ds_map;
	AcceptedZoneMap accepted_zone_map; //用来记录可接受的zone id列表

public:
	static CentralDeliveryServer *GetInstance() { return &instance; }
	std::string Identification() const { return "CentralDeliveryServer"; }
	void SetAccumulate(size_t size) { accumulate_limit = size; }
	CentralDeliveryServer() : accumulate_limit(0) { }

	const DSMap& GetDsMap() const { return ds_map;}

	bool IsConnect(int zoneid)
	{
		return (ds_map.find(zoneid) != ds_map.end());
	}
	
	void InsertZoneId(int zoneid, unsigned int sid)
	{
		ds_map[zoneid].sid = sid;
	}
	
	void SetLoad(int zoneid, int srv_limit, int srv_count);
//	int InitAcceptedZoneList( std::string list_str );
	
	bool IsAcceptedZone(int zoneid)
	{
		return (accepted_zone_map.find(zoneid) != accepted_zone_map.end());
	}
	int  GetGroupIdByZoneId(int zid)
	{
		AcceptedZoneMap::iterator it = accepted_zone_map.find(zid);
		if(it == accepted_zone_map.end()) return -1;
		return it->second;
	}

	void SetAcceptedZoneGroup(AcceptedZoneMap& azm)
	{
		accepted_zone_map.swap(azm);
	}

	void  GetAcceptedZoneGroup(std::vector<std::pair<int,int> > & list)
	{
		list.clear();
		AcceptedZoneMap::iterator it = accepted_zone_map.begin();
		AcceptedZoneMap::iterator ie = accepted_zone_map.end();
		for(;it != ie; ++it)
		{
			list.push_back(*it);
		}
	}

	void  GetAcceptedZone(int groupid,std::vector<int> &list)
	{
		list.clear();
		AcceptedZoneMap::iterator it = accepted_zone_map.begin();
		AcceptedZoneMap::iterator ie = accepted_zone_map.end();
		for(;it != ie; ++it)
		{
			if(it->second == groupid)
				list.push_back(it->first);
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
