
#ifndef __GNET_FACTIONACCEPTJOIN_RE_HPP
#define __GNET_FACTIONACCEPTJOIN_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionchoice.h"
#include "glinkclient.hpp"
namespace GNET
{

class FactionAcceptJoin_Re : public GNET::Protocol
{
	#include "factionacceptjoin_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		int roleid=GLinkClient::GetInstance()->roleid;
		switch (retcode)
		{
			case ERR_SUCCESS:
				printf("成功提交接受请求\n");
				break;
			case ERR_FC_ACCEPT_REACCEPT:
				printf("被批准加入本帮的玩家已经加入帮派\n"); 
				break;
			case ERR_FC_FACTION_NOTEXIST:
				printf("帮派不存在or玩家没有申请过本帮派\n");  
				break;
			case ERR_FC_NO_PRIVILEGE:
				printf("没有批准权限\n");	
				break;
			case ERR_FC_DBFAILURE:
				printf("数据库读写失败\n");	
				break;
		}
		FactionChoice(roleid,manager,sid);
	}
};

};

#endif
