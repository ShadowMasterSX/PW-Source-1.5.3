#ifndef __GNET_OPCHANGEPROCLAIM_H
#define __GNET_OPCHANGEPROCLAIM_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpChangeproclaim : public Operation
{
#include "opchangeproclaim.inl"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_changeproclaim_re(oper_roleid,retcode);
			return retcode;
		}
		//TODO: do operation here
		retcode=Factiondb::GetInstance()->UpdateAnnounce(player.faction_id, params.proclaim, WRAPPER_WREF);
		if (retcode!=ERR_SUCCESS)
			gfs_send_changeproclaim_re(oper_roleid,retcode);
		return retcode;
	}
	void SetResult(void* pResult)
	{
		int retcode=( (UserFactionRes*) pResult )->retcode;
		DEBUG_PRINT("opchangeproclaim: roleid=%d, retcode=%d\n",oper_roleid,retcode);
		if ( retcode==ERR_SUCCESS )
			gfs_broadcast_changeproclaim_re(player.faction_id,oper_roleid,params.proclaim);
		else
			gfs_send_changeproclaim_re(oper_roleid,retcode);
	}
};

};//end of namespace
#endif
