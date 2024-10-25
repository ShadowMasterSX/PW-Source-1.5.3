
#ifndef __GNET_ADDFRIEND_HPP
#define __GNET_ADDFRIEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapuser.h"
namespace GNET
{

class AddFriend : public GNET::Protocol
{
	#include "addfriend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		unsigned int dstlsid = 0, dstsid = 0;
		int dstcls = 0;
		AddFriendRqstArg arg;
		int ret = 0;

		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		if(dsm->IsCentralDS())
		{
			DEBUG_PRINT("gdelivery::tradestart:try to add friend on central delivery, refuse him.roleid=%d,srcdstroleid=%d\n", srcroleid, dstroleid);
			return;
		}

		{
			if(dstroleid<=0)
				UserContainer::GetInstance().FindRoleId( dstname, dstroleid );
			if(dstroleid==srcroleid)
				return;

			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfosrc = UserContainer::GetInstance().FindRoleOnline( srcroleid );
			if( NULL == pinfosrc )
				return;
			arg.srcname = pinfosrc->name;

			if(pinfosrc->friend_ver<0)
				ret = ERR_FS_NOTINITIALIZED;
			else if(dstroleid>0)
			{
				GFriendInfoVector* plist = &(pinfosrc->friends);
				PlayerInfo * pinfodst = UserContainer::GetInstance().FindRoleOnline(dstroleid);
				if (NULL != pinfodst)
				{
					dstlsid = pinfodst->localsid;
					dstsid = pinfodst->linksid;
					dstcls = pinfodst->cls;
					dstname = pinfodst->name;
					for(GFriendInfoVector::iterator i=plist->begin(), ie=plist->end(); i != ie; ++i)
					{
						if(i->rid==dstroleid && dstname==i->name)
						{
							ret = ERR_FS_DUPLICATE;
							break;
						}
					}
				}
				else
					ret = ERR_FS_OFFLINE;
			}
			else
				ret = ERR_FS_OFFLINE;
		}

		if(ret!=0)
		{
			AddFriend_Re re;
			re.retcode = ret;
			re.info.rid = dstroleid;
			re.info.name.swap(dstname);
			re.srclsid = srclsid;
			dsm->Send(sid,re);
			return;
		}
		arg.srcroleid = srcroleid;
		arg.dstlsid = dstlsid;
		AddFriendRqst* rqst = (AddFriendRqst*)Rpc::Call(RPC_ADDFRIENDRQST, &arg);
		rqst->srclsid = srclsid;
		rqst->srcsid = sid;
		rqst->dstlsid = dstlsid;
		rqst->dstsid = dstsid;
		rqst->dstname = dstname;
		rqst->dstroleid = dstroleid;
		rqst->dstcls = dstcls;
		dsm->Send(dstsid, rqst);
	}
};

};

#endif
