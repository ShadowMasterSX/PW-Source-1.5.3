#ifndef __GNET_OPLEAVE_H
#define __GNET_OPLEAVE_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpLeave : public Operation
{
#include "opleave.inl"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_leave_re(oper_roleid,retcode);
			return retcode;
		}
		//TODO: do operation here

		Factiondb::GetInstance()->DismissMember( player.faction_id,player.roleid, WRAPPER_WREF );
		return retcode;
	}
	void SetResult(void* pResult)
	{
		int retcode=( (DefFactionRes*) pResult )->retcode;
		DEBUG_PRINT("opleave:: player %d leave from faction %d, retcode=%d\n",player.roleid,player.faction_id,retcode);
		if ( retcode==ERR_SUCCESS )
		{
			gfs_broadcast_leave_re(player.faction_id,oper_roleid,player.froleid);
			GFactionServer::GetInstance()->UpdatePlayer(oper_roleid,_FACTION_ID_INVALID,_R_UNMEMBER);
		}
		else
		{
			gfs_send_leave_re(oper_roleid,retcode);
		}
	}
};

};//end of namespace
#endif
