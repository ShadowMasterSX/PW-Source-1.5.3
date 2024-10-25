#ifndef __GNET_OPUPGRADE_H
#define __GNET_OPUPGRADE_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"
#include "dbfactionupgrade.hrp"

namespace GNET
{

class OpUpgrade : public Operation
{
#include "opupgrade.inl"
private:
	bool ConditionCheck() { return true;}
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_upgradefaction_re( oper_roleid,retcode );
			return retcode;
		}
		retcode = Factiondb::GetInstance()->UpgradeFaction(player.faction_id, oper_roleid, params.money, WRAPPER_WREF);
		return retcode;
	}
	void SetResult(void* pResult)
	{
		DBFactionUpgradeRes* res = (DBFactionUpgradeRes*)pResult;
		int retcode = res->retcode;
		DEBUG_PRINT("opupgrade: roleid=%d, retcode=%d, fid=%d\n",oper_roleid,retcode,player.faction_id);
		if ( retcode==ERR_SUCCESS )
		{
			__SYNCDATA__.player_money = res->money;
			__SYNCDATA__.player_sp=0;
			if (__SYNCDATA__.player_money<0) __SYNCDATA__.player_money=0;
			gfs_broadcast_upgradefaction_re( player.faction_id );
		}
		else
		{
			__SYNCDATA__.player_sp=0;
			gfs_send_upgradefaction_re( oper_roleid,retcode );
		}
	}
};

};//end of namespace
#endif
