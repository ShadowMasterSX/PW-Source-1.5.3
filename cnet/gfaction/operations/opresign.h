#ifndef __GNET_OPRESIGN_H
#define __GNET_OPRESIGN_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"
#include "gfactionserver.hpp"
namespace GNET
{

class OpResign : public Operation
{
#include "opresign.inl"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_resign_re( oper_roleid,retcode );
			return retcode;
		}
		//TODO: do operation here
		return Factiondb::GetInstance()->UpdateRole(player.faction_id,oper_roleid, oper_roleid, 
				player.froleid, _R_MEMBER, WRAPPER_WREF);
	}
	void SetResult(void* pResult)
	{
		int retcode=( (UserFactionRes*) pResult )->retcode;
		DEBUG_PRINT("opresign: roleid=%d, retcode=%d\n",oper_roleid,retcode);
		if ( retcode==ERR_SUCCESS )
		{
			gfs_broadcast_resign_re(player.faction_id,oper_roleid,player.froleid);
			GFactionServer::GetInstance()->UpdatePlayer(oper_roleid,player.faction_id,_R_MEMBER);
		}
		else
		{
			gfs_send_resign_re( oper_roleid,retcode );
		}	
	}
};

};//end of namespace
#endif
