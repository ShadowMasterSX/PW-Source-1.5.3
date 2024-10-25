#ifndef __GNET_OPFACTIONRENAME_H
#define __GNET_OPFACTIONRENAME_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"
#include "gfs_io.h"

namespace GNET
{

class OpFactionrename : public Operation
{
#include "opfactionrename.inl"
private:
	bool ConditionCheck() { return true; }
public:
	void SetResult(void* pResult)
	{
		FactionOPAddInfo addinfo=pWrapper->GetAddInfo();
		DEBUG_PRINT("opfactionrename uname callback%d\n",addinfo.retcode);
		if ( addinfo.retcode!=ERR_SUCCESS )
		{
			DEBUG_PRINT("opfactionrename: uniquename failed(err:%d). rid=%d\n",addinfo.retcode,oper_roleid);
			gfs_send_factionrenameannounce(oper_roleid,addinfo.retcode,params.faction_name);
			return;
		}
		DEBUG_PRINT("opfactionrename gamed sendback%d\n",addinfo.retcode);
		if(!gps_send_factionrename_first_re(oper_roleid,addinfo.retcode,params.faction_name))
		{
			Log::log(LOG_ERR,"faction:%d rename prefactionrename oper%d offline.\n",player.faction_id,oper_roleid);
			uns_send_postfactionrename(ERR_FC_OFFLINE,
				GFactionServer::GetInstance()->zoneid,
				player.faction_id,
				params.faction_name);
		}
		//TODO: do operation here
		return ;
	}
	
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
			return retcode;
		if ( !blDecodeContext && !DecodeContext() )
			return ERR_FC_DATAERROR;
	    if( params.faction_name.size()>18 || Matcher::GetInstance()->Match((char*)params.faction_name.begin(),params.faction_name.size())!=0)
		{
			gfs_send_factionrenameannounce(oper_roleid,ERR_FC_INVALIDNAME,params.faction_name);
	        return ERR_FC_INVALIDNAME;
		}
		DEBUG_PRINT("opfactionrename: query faction name to uniquename server. roleid=%d,zoneid=%d,fname(sz:%d)\n",
				oper_roleid,GFactionServer::GetInstance()->zoneid,params.faction_name.size() );

		uns_send_prefactionrename(
				GFactionServer::GetInstance()->zoneid,
				player.faction_id,
				params.faction_name,
				WRAPPER_WREF
			);
		return ERR_SUCCESS; 

	}
};

};//end of namespace
#endif
