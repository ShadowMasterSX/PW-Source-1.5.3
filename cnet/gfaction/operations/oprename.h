#ifndef __GNET_OPRENAME_H
#define __GNET_OPRENAME_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpRename : public Operation
{
#include "oprename.inl"
private:
	bool ConditionCheck() { return !Factiondb::GetInstance()->AlreadyDelayExpel(params.dst_roleid)
	    		&&Factiondb::GetInstance()->IsSpecialMember(player.faction_id,params.dst_roleid); }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_rename_re( oper_roleid,retcode,params.dst_roleid,params.new_name );
			return retcode;
		}
		//TODO: do operation here
		retcode=Factiondb::GetInstance()->UpdateNickname( player.faction_id,params.dst_roleid,params.new_name,WRAPPER_WREF );
		if ( retcode!=ERR_SUCCESS )
			gfs_send_rename_re( oper_roleid,retcode,params.dst_roleid,params.new_name );
		return retcode;
	}
	void SetResult(void* pResult)
	{
		int retcode=( (UserFactionRes*) pResult )->retcode;
		DEBUG_PRINT("oprename: roleid=%d, retcode=%d(dstroleid=%d)\n",oper_roleid,retcode,
				params.dst_roleid );
		if ( retcode == ERR_SUCCESS ) 
		{
			gfs_broadcast_rename_re( player.faction_id,oper_roleid,params.dst_roleid,params.new_name );
		}
		else
		{
			gfs_send_rename_re( oper_roleid,retcode,params.dst_roleid,params.new_name );
		}
	}
};

};//end of namespace
#endif
