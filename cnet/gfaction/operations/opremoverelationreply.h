#ifndef __GNET_OPREMOVERELATIONREPLY_H
#define __GNET_OPREMOVERELATIONREPLY_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpRemoverelationreply : public Operation
{
#include "opremoverelationreply.inl"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			DEBUG_PRINT("opremoverelationreply: checkall failed. retcode=%d. rid=%d dst_fid=%d agree=%d\n",retcode,oper_roleid,params.dst_fid,params.agree);
			gfs_send_removerelationreply_re( oper_roleid,retcode );
			return retcode;
		}
		//TODO: do operation here
		retcode = Factiondb::GetInstance()->RemoveRelationReply(player.faction_id, params.dst_fid, params.agree, WRAPPER_WREF);
		if(retcode != ERR_SUCCESS)
		{
			DEBUG_PRINT("opremoverelationreply: factiondb failed. retcode=%d. rid=%d dst_fid=%d agree=%d\n",retcode,oper_roleid,params.dst_fid,params.agree);
			gfs_send_removerelationreply_re( oper_roleid,retcode );
		}
		return retcode;
	}
	void SetResult(void* pResult)
	{
		RpcRetcode * res = (RpcRetcode *)pResult;
		int retcode = res->retcode;
		DEBUG_PRINT("opremoverelationreply: roleid=%d, retcode=%d, fid=%d, dst_fid=%d, agree=%d\n",oper_roleid,retcode,player.faction_id,params.dst_fid, params.agree);
		if ( retcode==ERR_SUCCESS )
		{
			gfs_broadcast_removerelationreply_re( player.faction_id, params.dst_fid, params.agree );
		}
		else
		{
			gfs_send_removerelationreply_re( oper_roleid,retcode );
		}
	}
};

};//end of namespace
#endif
