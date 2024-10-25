
#include "ganticheaterclient.hpp"
#include "state.hxx"
#include "macros.h"
#include "acwhoami.hpp"
#include "commander.hpp"
#include "timertask.h"

namespace GNET
{

GAntiCheaterClient GAntiCheaterClient::instance[SERVER_COUNT];
std::map<int, size_t> GAntiCheaterClient::area_map;

GAntiCheaterClient::GAntiCheaterClient() : accumulate_limit(0), conn_state(false), backoff(BACKOFF_INIT)
		, locker_state("ganticheaterclient state locker")
     { c_id = GetNo(); a_id = -1; printf("client cid %d construct\n", c_id); }

const Protocol::Manager::Session::State* GAntiCheaterClient::GetInitState() const
{
	return &state_ACControlClient;
}

void GAntiCheaterClient::Reconnect()
{
    Thread::HouseKeeper::AddTimerTask(new ReconnectTask(this, 1), backoff);
    backoff *= 2;
    if (backoff > BACKOFF_DEADLINE) backoff = BACKOFF_DEADLINE;
}

void GAntiCheaterClient::OnAddSession(Session::ID sid)
{
	printf("on add session\n");
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
	SendProtocol( ACWhoAmI(_CONTROL_CLIENT,0/* for control, id is always 0 */) );
}

/*
void GAntiCheaterClient::SendData(const Octets &data)
{
	size_t remain_size = data.size();
	size_t gid = GetNextID();
	if( remain_size <= FRAG_SIZE )
	{
		ACData acdata;
		acdata.id = gid;
		acdata.total = acdata.seq = 1;
		acdata.data = data;
		SendProtocol(acdata);
	}
	else
	{
		size_t seq = 1;
		size_t total = remain_size / FRAG_SIZE;
		if( total == 0 ) total = 1;
		const unsigned char* start = (const unsigned char*)data.begin();
		const unsigned char* end = (const unsigned char*)data.end();
		unsigned char *cur = start;
		while( remain_size > 0 )
		{
			ACData acdata;
			acdata.id = gid;
			acdata.total = total;
			acdata.seq = i + 1;
			if( remain_size >= FRAG_SIZE )
			{
				acdata.data = Octets(cur, cur + FRAG_SIZE);
				cur += FRAG_SIZE;
				remain_size -= FRAG_SIZE;
			}
			else
			{
				acdata.data = Octets(cur,  end);
				remain_size = 0;
				cur = end;
			}
			SendProtocol(acdata); 
		}
	}
}
*/
void GAntiCheaterClient::OnDelSession(Session::ID sid)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	area_map.erase(a_id);
	printf("disconnected\n");
	Reconnect();
	//TODO
}

void GAntiCheaterClient::OnAbortSession(const SockAddr &sa)
{
	Thread::Mutex::Scoped l(locker_state);
	conn_state = false;
	printf("client %s on abort\n", Identification().c_str());
	area_map.erase(a_id);
	Reconnect();
	//TODO
}

void GAntiCheaterClient::OnCheckAddress(SockAddr &sa) const
{
	//TODO
}

void GAntiCheaterClient::SendProtocol(const Protocol &protocol, int _aid)
{
	if( _aid == 0 )	
	{
		if( area_map.empty() ) 
			printf("no connecting server\n");
		else
			for(std::map<int, size_t>::iterator it = area_map.begin();
				it != area_map.end(); ++it)
			{
				instance[ (*it).second ].SendProtocol(protocol);
			}
	}
	else
	{
		if( area_map.find(_aid) != area_map.end() )
			instance[area_map[_aid]].SendProtocol(protocol);
		else
			printf("wrong area id\n");
	}
}

void GAntiCheaterClient::SendProtocol(const Protocol *protocol, int _aid)
{
	if( _aid == 0 )	
	{
		if( area_map.empty() ) 
			printf("no connecting server\n");
		else
			for(std::map<int, size_t>::iterator it = area_map.begin();
				it != area_map.end(); ++it)
			{
				instance[ (*it).second ].SendProtocol(protocol);
			}
	}
	else
	{
		if( area_map.find(_aid) != area_map.end() )
			instance[area_map[_aid]].SendProtocol(protocol);
		else
			printf("wrong area id\n");
	}
}

/*
void GAntiCheaterClient::PutData(size_t id, size_t seq, size_t total, const Octets &data)
{
	OctetsBuffer ob;
	if( total == 1 )
		ob.SetO(data);
	else if( total > 1 && seq > 0 )
	{
		data_map[id].second.push_back(data);
		if( seq == total )
		{
			std::vector<Octets> &vos = data_map[id].second;
			if( vos.size() != total )
				data_map.erase(id);
			else
			{
				OctetsBuffer ob;
				for(size_t i = 0; i < total ; ++i)
					ob.SetO(vos[i]);
			}
		}
	}
	ob.Rewind();
	ParseData(ob);
}

void GAntiCheaterClient::ParseData(OctetsBuffer &data)
{
	int rescode = data.Get32();
	int cmd = data.Get32();
	Octets os = data.GetOE();
	Commander::GetInstance()->ParseResponse(rescode, cmd, os);
}

void GAntiCheaterClient::DoTimer()
{
	std::vector<int> erase_ids;
	for(DataMap::iterator it = data_map.begin(); it != data_map.end(); ++it)
	{
		if( --((*it).second.first) == 0 )
			erase_ids.push_back((*it).first);
	}
	for(std::vector<int>::const_iterator it = erase_ids.begin(); it != erase_ids.end(); ++it)
		data_map.erase((*it));
}
*/
};
