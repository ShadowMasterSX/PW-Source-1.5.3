
#include "centraldeliveryserver.hpp"
#include "state.hxx"

#include "loadexchange.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"

namespace GNET
{

CentralDeliveryServer CentralDeliveryServer::instance;

const Protocol::Manager::Session::State* CentralDeliveryServer::GetInitState() const
{
	return &state_CentralDeliveryServer;
}

void CentralDeliveryServer::OnAddSession(Session::ID sid)
{
	//TODO
}

void CentralDeliveryServer::OnDelSession(Session::ID sid)
{
	/*DSMap::iterator it = ds_map.begin();
	MapEraser<DSMap> del_keys(ds_map);
	for (; it != ds_map.end(); ++it) {
		if (it->second.sid == sid) {
			int count = UserContainer::GetInstance().DisconnectZoneUsers(it->first);
			//Log::log(LOG_ERR, "CrossRelated CentralDeliveryServer erase deliveryd %d sid=%d drop %d users", it->first, sid, count);
			//将该deliveryd上的玩家踢下线
			del_keys.push(it->first);
		}
	}*/

	std::set<int> zoneid_set;
	DSMap::iterator it = ds_map.begin();
	while(it != ds_map.end()) {
		if (it->second.sid == sid) {
			zoneid_set.insert(it->first);

			ds_map.erase(it++);
			continue;
		}

		++it;
	}

	for(std::set<int>::iterator it = zoneid_set.begin(); it != zoneid_set.end(); ++it) {
		int count = UserContainer::GetInstance().DisconnectZoneUsers(*it);
		Log::log(LOG_ERR, "CrossRelated CentralDeliveryServer erase deliveryd %d sid=%d drop %d users", *it, sid, count);
	}
}

void CentralDeliveryServer::SetLoad(int zoneid, int srv_limit, int srv_count)
{
	DSMap::iterator it = ds_map.find(zoneid);
	if (it == ds_map.end()) {
		Log::log(LOG_ERR, "CrossRelated Central delivery set load, invalid zoneid %d", zoneid);
		return;
	}
	
	it->second.cache_server_load.SetLoad(srv_limit, srv_count);
	it->second.crs_ttl = CRS_TTL;
}

/*int CentralDeliveryServer::InitAcceptedZoneList( std::string list_str )
{
	accepted_zone_set.clear();
	if( list_str.length() <= 0 ) return 0;

	char* delim = ",";
	char* buffer = new char[list_str.length() + 1];
	if( NULL == buffer ) return 0;

	memcpy( buffer, list_str.c_str(), list_str.length() );
	buffer[list_str.length()] = 0;

	char* token = strtok( buffer, delim );
	while( NULL != token ) { 
		accepted_zone_set.insert( atoi(token) );
		token = strtok( NULL, delim );
	}       

	delete [] buffer; 
	return accepted_zone_set.size();
}*/

void LoadExchangeTask::Run()
{
	LOG_TRACE("CrossRelated CentralDeliveryServer start load exchange");
	
	LoadExchange exchg;
	GDeliveryServer* dsm = GDeliveryServer::GetInstance();
	exchg.zoneid = dsm->GetZoneid();
	exchg.version = dsm->GetVersion();
	exchg.edition = dsm->GetEdition();
	exchg.server_limit = UserContainer::GetInstance().GetPlayerLimit();
	exchg.server_count = UserContainer::GetInstance().Size();

	CentralDeliveryServer::GetInstance()->BroadcastProtocol(exchg);
	Thread::HouseKeeper::AddTimerTask(this, interval);
}

void CrsSvrCheckTimer::CheckConnection()
{
	CentralDeliveryServer* cds = CentralDeliveryServer::GetInstance();
	CentralDeliveryServer::DSMap::iterator it = cds->ds_map.begin();

	while(it != cds->ds_map.end() ) {
		CentralDeliveryServer::delivery_t& info = it->second;
		info.crs_ttl -= update_time;

		if(info.crs_ttl <= 0) {
			Log::log(LOG_ERR, "CrossRelated CentralDeliveryServer session %d's TTL is expired(%d). session closed.\n", info.sid, info.crs_ttl);
			cds->Close(info.sid);
		}

		++it;
	}
}

void CrsSvrCheckTimer::Run()
{
	CheckConnection();
	Thread::HouseKeeper::AddTimerTask(this,update_time);
}

};
