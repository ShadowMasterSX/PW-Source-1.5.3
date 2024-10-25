
#ifndef __GNET_FACTIONRENAMEGSVERIFY_RE_HPP
#define __GNET_FACTIONRENAMEGSVERIFY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"
#include "postfactionrename.hpp"
#include "dbfactionrename.hrp"
#include "gfactionserver.hpp"
#include "gfs_io.h"
#include "log.h"

namespace GNET
{

class FactionRenameGsVerify_Re : public GNET::Protocol
{
	#include "factionrenamegsverify_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if(retcode == ERR_SUCCESS)
		{
			DEBUG_PRINT("faction:%d rename gs verify role:%d success.\n",fid,rid);
			gfs_send_dbfactionrename(rid,fid,item_id,item_num,item_pos,newname,syncdata);				
		}
		else
		{
			Log::log(LOG_ERR,"faction:%d rename gs verify role:%d failed.\n",fid,rid);
			gfs_send_factionrenameannounce(rid,retcode,newname);
			uns_send_postfactionrename(retcode,
					GFactionServer::GetInstance()->zoneid,
					fid,newname);
		}
	}	
};

};

#endif
