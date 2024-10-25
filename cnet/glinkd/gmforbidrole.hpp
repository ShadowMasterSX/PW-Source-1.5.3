
#ifndef __GNET_GMFORBIDROLE_HPP
#define __GNET_GMFORBIDROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "privilege.hxx"
#include "forbid.hxx"
#include "gdeliveryclient.hpp"
#include "glinkserver.hpp"
#include "gmforbidrole_re.hpp"

namespace GNET
{

class GMForbidRole : public GNET::Protocol
{
	#include "gmforbidrole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		//valid gmroleid
		switch (fbd_type)
		{
		case Forbid::FBD_FORBID_LOGIN:	
			if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORCE_OFFLINE) )
			{
				Log::log(LOG_ERR,"WARNING: user %d try to use PRV_FORCE_OFFLINE privilege that he doesn't have.\n",gmroleid);
				return;
			}
			break;
		case Forbid::FBD_FORBID_TALK:
			if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORBID_TALK) )
			{
				Log::log(LOG_ERR,"WARNING: user %d try to use PRV_FORBID_TALK privilege that he doesn't have.\n",gmroleid);
				return;
			}
			break;
		case Forbid::FBD_FORBID_TRADE:
			if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORBID_TRADE) )
			{
				Log::log(LOG_ERR,"WARNING: user %d try to use PRV_FORBID_TRADE privilege that he doesn't have.\n",gmroleid);
				return;
			}
			break;
		case Forbid::FBD_FORBID_SELL:
			if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORBID_SELL) )
			{
				Log::log(LOG_ERR,"WARNING: user %d try to use PRV_FORBID_SELL privilege that he doesn't have.\n",gmroleid);
				return;
			}
			break;
		case Forbid::FBD_FORBID_SELLPOINT:
			// err, can not only forbid a role to sellpoint, this limit should affect a account
		default:
			Log::log(LOG_ERR,"WARNING: user %d try to use invalid forbid type(%d)\n",gmroleid,fbd_type);
			return;
			break;	
		}
		//log
		char content[256];
		sprintf(content,"Forbid: forbid_type=%d,forbidUserid=%d,forbidtime=%d",fbd_type,dstroleid,forbid_time);
		Log::gmoperate(gmroleid,fbd_type,content);
		//send to delivery	
		localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
		lsm->Send(sid,GMForbidRole_Re(ERR_SUCCESS,fbd_type,dstroleid,forbid_time));
	}
};

};

#endif
