
#ifndef __GNET_ACACCUSERE_HPP
#define __GNET_ACACCUSERE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "playeraccuse_re.hpp"
#include "dbsendmail.hrp"

namespace GNET
{

class ACAccuseRe : public GNET::Protocol
{
	#include "acaccusere"
	void MakeMail(GMail& mail)
	{
		mail.header.id        = 0;
		mail.header.sender    = 0;
		mail.header.sndr_type = _MST_ANTICHEAT;
		mail.header.receiver  = (int)roleid;
		mail.header.send_time = time(NULL);
		mail.header.attribute = (1<<_MA_UNREAD);
		mail.header.title = Marshal::OctetsStream() << (int)accusation_roleid << result;		
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{		
		// TODO
		if(result != 2) 
		{
			Log::log(LOG_INFO,"gdelveryd:acaccusere: no notice client, roleid=%d,destid=%d,result=%d,localid=%d", 
					(int)roleid, (int)accusation_roleid, result, zoneid);		
			return; // 非成功无客户端反馈
		}
		
		if(zoneid == GDeliveryServer::GetInstance()->GetZoneid())
		{
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline((int)roleid);

			if(pinfo)
			{
				PlayerAccuse_Re proto(pinfo->localsid, (int)accusation_roleid, result);
				GDeliveryServer::GetInstance()->Send(pinfo->linksid, proto);			
			}
			else
			{
				DBSendMailArg arg;
				MakeMail(arg.mail);
				DBSendMail* rpc=(DBSendMail*) Rpc::Call( RPC_DBSENDMAIL, arg);
				GameDBClient::GetInstance()->SendProtocol( rpc );
			}
		}
		else
		{
			Log::log(LOG_ERR,"gdelveryd:acaccusere: invalid zoneid, zoneid=%d,localid=%d", 
					zoneid , GDeliveryServer::GetInstance()->zoneid);		
		}
	}
};

};

#endif
