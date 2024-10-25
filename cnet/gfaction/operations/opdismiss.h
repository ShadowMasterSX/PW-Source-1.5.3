#ifndef __GNET_OPDISMISS_H
#define __GNET_OPDISMISS_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"
#include "gfactionserver.hpp"
#include "gfs_io.h"	
namespace GNET
{

class OpDismiss : public Operation
{
#include "opdismiss.inl"
private:
	bool ConditionCheck() { return true; }
	//send to uniquename server
	void Send2UNS(unsigned int fid,const Octets& fname)
	{
		uns_send_postdeletefaction(
				GFactionServer::GetInstance()->zoneid,
				fid,
				fname
			);	
	}
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_factiondismiss_re(oper_roleid,retcode);
			return retcode;
		}
		//TODO: do operation here
		DEBUG_PRINT("opdismiss:: call RemoveFaction. roleid=%d,fid=%d\n",oper_roleid,player.faction_id);
		retcode=Factiondb::GetInstance()->RemoveFaction(player.faction_id,WRAPPER_WREF);
		return retcode;
	}
	void SetResult(void* pResult)
	{
		int retcode=( (DelFactionRes*) pResult )->retcode;
		DEBUG_PRINT("opdismiss: roleid=%d, retcode=%d\n",oper_roleid,retcode);
		if ( retcode == ERR_SUCCESS )
		{
			gfs_update_factiondismiss( player.faction_id );
			Send2UNS( player.faction_id,((DelFactionRes*) pResult)->fname );
		}
		else
			gfs_send_factiondismiss_re(oper_roleid,retcode);
	}
};

};//end of namespace
#endif
