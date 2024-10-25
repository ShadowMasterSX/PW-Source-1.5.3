
#ifndef __GNET_PLAYERRENAME_HPP
#define __GNET_PLAYERRENAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "playerrename_re.hpp"
#include "preplayerrename.hrp"

namespace GNET
{

class PlayerRename : public GNET::Protocol
{
	#include "playerrename"
	void SyncGameServer(const PlayerInfo * pinfo, const GMailSyncData& sync, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(
				pinfo->gameid,
				GMailEndSync(0/*tid,must be 0*/,retcode,roleid,sync)
			);
	}
	
	void SendResult( int retcode, Octets & newname, const PlayerInfo * pinfo )
	{
		GDeliveryServer::GetInstance()->Send(
				pinfo->linksid,
				PlayerRename_Re(pinfo->localsid, roleid, newname, retcode)
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("PlayerRename roleid %d nawname.size %d itemid %d itempos %d",roleid,newname.size(),attach_obj_id,attach_obj_pos);
		GMailSyncData sync;
		try{
			Marshal::OctetsStream os(syncdata);
			os >> sync;
		}
		catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"PlayerRename unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}
		GDeliveryServer * dsm=GDeliveryServer::GetInstance();
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (NULL == pinfo)
			return;
		//如果是跨服服务器，屏蔽掉改名服务
		if (dsm->IsCentralDS())
		{
			// 解锁玩家交易状态
			sync.inventory.items.clear();
			SyncGameServer(pinfo, sync, ERR_PR_OUTOFSERVICE);
			return;
		}
		//检查新名字是否已经存在以及合法性
		int ret = NameManager::GetInstance()->CheckNewName(newname);
		if (ERR_SUCCESS != ret)
		{
			LOG_TRACE("PlayerRename newname check failed. ret=%d",ret);
			// 解锁玩家交易状态
			sync.inventory.items.clear();
			SyncGameServer(pinfo, sync, ret);
			// 发送失败通知
			SendResult(ret, newname, pinfo);
			return;
		}
		PrePlayerRename * rpc=(PrePlayerRename *)Rpc::Call( RPC_PREPLAYERRENAME, PrePlayerRenameArg(roleid, dsm->zoneid,pinfo->userid, newname));
		rpc->save_linksid = pinfo->linksid;
		rpc->save_localsid = pinfo->localsid;
		rpc->save_gsid = pinfo->gameid;
		rpc->item_id = attach_obj_id;
		rpc->item_pos = attach_obj_pos;
		rpc->item_num = attach_obj_num;
		rpc->syncdata = sync;
		UniqueNameClient::GetInstance()->SendProtocol(rpc);
		LOG_TRACE("PlayerRename roleid %d send to uniquenamed", roleid);
	}
};

};

#endif
