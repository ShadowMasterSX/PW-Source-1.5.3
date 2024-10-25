
#ifndef __GNET_ROLELIST_RE_HPP
#define __GNET_ROLELIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinventory"
#include "roleinfo"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "statusannounce.hpp"

namespace GNET
{

class RoleList_Re : public GNET::Protocol
{
	#include "rolelist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		//change state
		if (handle==_HANDLE_END)
		{
			lsm->ChangeState(localsid,&state_GSelectRoleServer);

			//add session to alivekeeper map
			lsm->SetAliveTime(localsid, _CLIENT_TTL);
		}
		else
		{
			lsm->ChangeState(localsid,&state_GRoleList);
		}
		if (GDeliveryClient::GetInstance()->IsPhoneLink())
		{
			//������ֻ�ר�õ�link����װ���Ͱ�������Ϣ���˵����Խ�ʡ����
			GNET::RpcDataVector<RoleInfo>::iterator it = rolelist.begin(), eit = rolelist.end();
			for ( ; it != eit; ++it)
			{
				(*it).custom_data.clear();
				(*it).equipment.clear();
			}
		}
		lsm->Send(localsid,this);
	}
};

};

#endif
