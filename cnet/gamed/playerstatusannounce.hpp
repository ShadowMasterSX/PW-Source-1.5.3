
#ifndef __GNET_PLAYERSTATUSANNOUNCE_HPP
#define __GNET_PLAYERSTATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "onlineplayerstatus"

void psvr_ongame_notify(int * start , size_t size,size_t step);
void psvr_offline_notify(int * start , size_t size,size_t step);
namespace GNET
{


class PlayerStatusAnnounce : public GNET::Protocol
{
	#include "playerstatusannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
#ifdef _TESTCODE
		// TODO
		printf("\nReceive PlayerStatusAnnounce from deliveryserver.\n");
		OnlinePlayerStatus ops;
		for (size_t i=0;i<playerstatus_list.size();i++)
		{
			ops=playerstatus_list[i];
			printf("OnlineUser: roleid=%d,status=%d,gid=%d\n",ops.roleid,status,ops.gid);
		}
		printf("\n");
#endif
		if(status == _STATUS_OFFLINE)
		{
			psvr_offline_notify(&(playerstatus_list[0].roleid),playerstatus_list.size(),sizeof(playerstatus_list[0]));
		}
		else
		if(status == _STATUS_ONGAME)
		{
			psvr_ongame_notify(&(playerstatus_list[0].roleid),playerstatus_list.size(),sizeof(playerstatus_list[0]));
		}
	}
};

};

#endif
