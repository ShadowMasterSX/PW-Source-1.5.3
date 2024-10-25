
#include "gacdutil.h"
#include "ganticheaterserver.hpp"
#include "state.hxx"
#include "userdatamanager.hpp"
#include "acconstants.h"
#include "xmlconfig.h"

namespace GNET
{

GAntiCheaterServer GAntiCheaterServer::instance;

const Protocol::Manager::Session::State* GAntiCheaterServer::GetInitState() const
{
	return &state_ACServer;
}

void GAntiCheaterServer::OnAddSession(Session::ID sid)
{
	//TODO
	DEBUG_PRINT_INIT("session %d on add session\n", sid);
}

void GAntiCheaterServer::OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer)
{
	/*
	const struct sockaddr_in *peer_addr = (const struct sockaddr_in*)peer;
	const struct sockaddr_in *local_addr = (const struct sockaddr_in*)local;
	const char *peer_ip_str = inet_ntoa(peer_addr->sin_addr);
	const char *local_ip_str = inet_ntoa(local_addr->sin_addr);
	DEBUG_PRINT_INIT("session %d on set trasport %s->%s\n", sid, peer_ip_str, local_ip_str);
	if( peer_addr->sin_addr.s_addr == 0x6e0210ac )
	{
		fake_sid = sid;
	}
	*/
}

void GAntiCheaterServer::OnDelSession(Session::ID sid)
{
	//TODO
	DEBUG_PRINT_INIT("session %d on del session\n", sid);
	Thread::Mutex::Scoped l(clients_locker);
	UserDataManager::GetInstance()->Stop();
	deliver_sid = 0xffffffff;
}

bool GAntiCheaterServer::SendProtocol(const Protocol *pro )
{
	Thread::Mutex::Scoped l(clients_locker);
	return Send(deliver_sid, pro);
}

void GAntiCheaterServer::ResetSource()
{
	Close(deliver_sid);
}

bool GAntiCheaterServer::SendProtocol(const Protocol &pro )
{
	Thread::Mutex::Scoped l(clients_locker);
	return Send(deliver_sid, pro);
}

bool GAntiCheaterServer::SendProtocol(const Protocol *pro, Session::ID sid)
{
	return Send(sid, pro);
}

bool GAntiCheaterServer::SendProtocol(const Protocol &pro, Session::ID sid)
{
	return Send(sid, pro);
}

void GAntiCheaterServer::SetDeliverSID(Session::ID sid)
{
	Thread::Mutex::Scoped l(clients_locker);
	if( deliver_sid != (unsigned int)-1 )
	{
		DEBUG_PRINT_INIT("multi delivery?\n");
		Close(sid);
		return;
	}
	deliver_sid = sid;
	DEBUG_PRINT_INIT("delivery connected!\n");
	UserDataManager::GetInstance()->Run();
}

/*
void GAntiCheaterServer::SendData(const Octets &data)
{
    size_t remain_size = data.size();
    size_t gid = GetNextID();
    if( remain_size <= FRAG_SIZE )
    {   
        ACData acdata;
        acdata.id = gid;
        acdata.total = acdata.seq = 1;
        acdata.data = data;
        SendProtocol(acdata, control_sid);
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
            SendProtocol(acdata, control_sid);
        }
    }
}

void GAntiCheaterServer::PutData(size_t id, size_t seq, size_t total, const Octets &data)
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

void GAntiCheaterServer::ParseData(OctetsBuffer &data)
{
    int cmd = data.Get32();
    Octets os = data.GetOE();
    Commander::GetInstance()->Request(cmd, os);
}

void GAntiCheaterServer::TimerUpdate()
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
