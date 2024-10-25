
#ifndef __GNET_GMLISTONLINEUSER_HPP
#define __GNET_GMLISTONLINEUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gmlistonlineuser_re.hpp"
#include "mapuser.h"
namespace GNET
{

class GMListOnlineUserQueryUser : public UserContainer::IQueryUser
{   
public:
	GMListOnlineUser_Re gmlou_re;
	bool Update( int userid, UserInfo & info )
	{
                PlayerInfo* role = info.role;
		if(role)
			gmlou_re.userlist.add(GMPlayerInfo( userid, info.roleid, info.linkid, info.localsid, info.gameid,
					info.status, role->name));
		return true;
	}
};

class GMListOnlineUser : public GNET::Protocol
{
	#include "gmlistonlineuser"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int beginid = -1;
		if( _HANDLE_BEGIN != handler )
			beginid = handler;

		GMListOnlineUserQueryUser	q;
		q.gmlou_re.retcode = ERR_SUCCESS;
		q.gmlou_re.gmroleid = gmroleid;
		q.gmlou_re.localsid = localsid;
		UserContainer::GetInstance().PartlyWalkUser( beginid, _HANDLE_PAGESIZE, q );

		q.gmlou_re.handler = ( -1 != beginid ? beginid : _HANDLE_END );
		GDeliveryServer::GetInstance()->Send(sid,q.gmlou_re);
	}
};

};

#endif
