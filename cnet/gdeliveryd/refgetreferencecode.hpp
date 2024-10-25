
#ifndef __GNET_REFGETREFERENCECODE_HPP
#define __GNET_REFGETREFERENCECODE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "refgetreferencecode_re.hpp"
#include "mapuser.h"
#include "gdeliveryserver.hpp"
#include "localmacro.h"
#include "referencemanager.h"


namespace GNET
{

class RefGetReferenceCode : public GNET::Protocol
{
	#include "refgetreferencecode"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("refgetreferencecode roleid %d", roleid);
		RefGetReferenceCode_Re re;
		re.roleid = roleid;
		re.localsid = localsid;
		re.retcode = ERR_SUCCESS;
		if (ReferenceManager::GetInstance()->GetRefCode(roleid, re.refcode))
			GDeliveryServer::GetInstance()->Send(sid, re);
		else
		{
			PlayerInfo *playerinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
			if (playerinfo != NULL)
				GProviderServer::GetInstance()->DispatchProtocol(playerinfo->gameid, this);
		}
/*
		GRoleInfo *pinfo = RoleInfoCache::Instance().Get(roleid);
		if (pinfo!=NULL && playerinfo!=NULL)
		{
			GDeliveryServer *dsm = GDeliveryServer::GetInstance();
			RefGetReferenceCode_Re re;
			re.roleid = roleid;
			re.localsid = localsid;
			if (pinfo->reborn_cnt==0 && pinfo->level<REF_LIMIT_REFERRERLEVEL)
				re.retcode = REF_ERR_REFREFERRERLEVEL;
			else
			{
				Octets temp;
				RefSpreadCode::Encode(playerinfo->userid, dsm->district_id, roleid, temp);
				CharsetConverter::conv_charset_t2u(temp, re.refcode);
				re.retcode = REF_ERR_SUCCESS;
			}
			dsm->Send(sid, re);
		}
*/
	}
};

};

#endif
