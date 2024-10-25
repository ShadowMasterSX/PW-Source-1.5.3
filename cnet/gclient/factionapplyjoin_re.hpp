
#ifndef __GNET_FACTIONAPPLYJOIN_RE_HPP
#define __GNET_FACTIONAPPLYJOIN_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "factionchoice.h"
#include "glinkclient.hpp"
namespace GNET
{

class FactionApplyJoin_Re : public GNET::Protocol
{
	#include "factionapplyjoin_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		int roleid=GLinkClient::GetInstance()->roleid;
		switch (retcode)
		{
		case ERR_FC_APPLY_REJOIN:
			printf("已经是某个帮派的成员，或者已经申请加入某帮派，申请失败\n");
			break;
		case ERR_FC_JOIN_SUCCESS:
			printf("成功加入帮派%d\n",factionid);
			break;
		case ERR_FC_JOIN_REFUSE:
			printf("申请被拒绝\n");	
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
