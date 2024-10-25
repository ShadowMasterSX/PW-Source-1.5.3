
#ifndef __GNET_FACTIONCREATE_RE_HPP
#define __GNET_FACTIONCREATE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"
#include "factionchoice.h"
namespace GNET
{

class FactionCreate_Re : public GNET::Protocol
{
	#include "factioncreate_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch(retcode)
		{
		case ERR_SUCCESS:
			printf("玩家%d创建帮派－%.*s成功.\n",roleid,faction_name.size(),(char*)faction_name.begin());
			break;
		case ERR_FC_CREATE_ALREADY:
			printf("玩家%d已经是某个帮派的成员。不能再创建帮派.\n",roleid);
			break;
		case ERR_FC_CREATE_DUP:
			printf("帮派名称重复.\n");
			break;	
		case ERR_FC_DBFAILURE:
			printf("数据库读写错误\n");
			break;	
		case ERR_FC_NO_PRIVILEGE:
			printf("没有权限。可能已经是某个帮派的成员\n");
			break;	
		}	
		int roleid=GLinkClient::GetInstance()->roleid;
		FactionChoice(roleid,manager,sid);
	}
};

};

#endif
