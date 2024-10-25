
#ifndef __GNET_FACTIONEXPEL_RE_HPP
#define __GNET_FACTIONEXPEL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkclient.hpp"
namespace GNET
{

class FactionExpel_Re : public GNET::Protocol
{
	#include "factionexpel_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		int roleid=GLinkClient::GetInstance()->roleid;
		switch (retcode)
		{
			case ERR_SUCCESS:
				if ((unsigned int)roleid==expelroleid)
					printf("你被驱逐出帮派\n");
				else 
					printf("成功驱逐玩家%d\n",expelroleid);
				break;
			case ERR_FC_NO_PRIVILEGE:
				printf("你没有驱逐帮众的权限.\n");
				break;	
			case ERR_FC_NOTAMEMBER:
				printf("被驱逐者不是本帮成员.\n");	
				break;
			case ERR_FC_DBFAILURE:
				printf("数据库读写错误\n");
				break;	
		}
		FactionChoice(roleid,manager,sid);
	}
};

};

#endif
