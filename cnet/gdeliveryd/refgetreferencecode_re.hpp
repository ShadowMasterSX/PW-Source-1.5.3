
#ifndef __GNET_REFGETREFERENCECODE_RE_HPP
#define __GNET_REFGETREFERENCECODE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "refspreadcode.h"
#include "conv_charset.h"
#include "referencemanager.h"

namespace GNET
{

class RefGetReferenceCode_Re : public GNET::Protocol
{
	#include "refgetreferencecode_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("refgetreferencecode_re roleid %d level %d reputation %d", roleid, level, reputation);
		GDeliveryServer *dsm = GDeliveryServer::GetInstance();
		PlayerInfo *playerinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (playerinfo == NULL)
			return;
		if (level < REF_LIMIT_REFERRERLEVEL)
			retcode = REF_ERR_REFREFERRERLEVEL;
		else if (reputation < REF_LIMIT_REPUTATION)
			retcode = REF_ERR_REPUTATION;
		else
		{
			Octets temp;
			//char ref_link[900] = {0};
			RefSpreadCode::Encode(playerinfo->userid, dsm->district_id, roleid, temp);
			//snprintf(ref_link, 899, "http://172.16.3.171:8088/index.jsp?spreadcode=%.*s&rolename=%.*s&zoneid=%d", temp.size(), (char*)temp.begin(), playerinfo->name.size(), (char*)playerinfo->name.begin(), dsm->zoneid);
			//LOG_TRACE("%s\n", ref_link);
			CharsetConverter::conv_charset_t2u(temp, refcode);
//			snprintf(ref_link, 899, "http://172.16.3.171:8088");
//			Octets ref_link_os(ref_link, 899);
			ReferenceManager::GetInstance()->InsertRefCode(roleid, refcode);
			retcode = ERR_SUCCESS;
		}
		dsm->Send(playerinfo->linksid, this);
	}
};

};

#endif
