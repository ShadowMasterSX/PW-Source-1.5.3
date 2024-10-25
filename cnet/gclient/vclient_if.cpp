#include "gamedatasend.hpp"
#include "glinkclient.hpp"
#include "vclient_if.h"

namespace VCLIENT
{

void SendC2SGameData(int roleid, void * buf, size_t size)
{
	GLinkClient::DispatchProtocol(roleid, GamedataSend(Octets(buf,size)));	
}

void OnEnterWorld(int roleid)
{
	GLinkClient::OnEnterWorld(roleid);
}

}
