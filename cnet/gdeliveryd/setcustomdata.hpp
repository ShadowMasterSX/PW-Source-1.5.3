
#ifndef __GNET_SETCUSTOMDATA_HPP
#define __GNET_SETCUSTOMDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "setcustomdata_re.hpp"

#include "getrolebase.hrp"
#include "putrolebase.hrp"
#include "gamedbclient.hpp"
#include "gproviderserver.hpp"
#include "mapuser.h"
namespace GNET
{

class SetCustomData : public GNET::Protocol
{
	#include "setcustomdata"
	void SendFaceModifyResult(GDeliveryServer* dsm, int gs_id, int retcode,unsigned int new_crc=0)
	{
		FaceTicketSet::face_ticket_t ticket;
		if ( FaceTicketSet::GetInstance().FindTicket(roleid,ticket) )
		{
			GProviderServer::GetInstance()->DispatchProtocol(
					gs_id,
					FaceModify_Re(retcode,roleid,ticket.id,ticket.pos,new_crc)
					);
		}
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("gdelivery::receive setcustom_data. roleid=%d,localsid=%d\n",roleid,localsid);
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		int status=0;
		bool newrole = false;
		int gs_id=-1;
		int userid = UidConverter::Instance().Roleid2Uid(roleid);
		{
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			UserInfo* ui=UserContainer::GetInstance().FindUser(userid);
			if (ui==NULL || ui->localsid!=localsid || ui->linksid!=sid)
				return;
			status  = ui->status;
			gs_id   = ui->gameid;
			newrole = (time(NULL)-ui->create_time[roleid &0x0f] < 86400*2);
		}
		if ( status==_STATUS_ONGAME && !(FaceTicketSet::GetInstance().HasTicket(roleid)) )
		{
			dsm->Send(sid,SetCustomData_Re(ERR_NOFACETICKET,0/*invalid CRC*/,roleid,localsid));
			return;
		}
		else if ( status!=_STATUS_ONGAME && !newrole )
		{
			dsm->Send(sid,SetCustomData_Re(ERR_NOFACETICKET,0/*invalid CRC*/,roleid,localsid));
			return;
		}

		dsm->rbcache.Lock();
		GRoleBase* grb=dsm->rbcache.GetDirectly(roleid);
		if (grb!=NULL) 
		{
			grb->custom_data.swap(custom_data);
			grb->custom_stamp++;
			dsm->Send(sid,SetCustomData_Re(ERR_SUCCESS,grb->custom_stamp,roleid,localsid));
			SendFaceModifyResult(dsm,gs_id,ERR_SUCCESS,grb->custom_stamp);
			//set rolebase to db
			RoleBasePair rbp;
			rbp.key.id=roleid;
			rbp.value=*grb;
			GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_PUTROLEBASE,&rbp));
		}
		else
		{
			//get GRoleBase from DB
			GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(roleid));
			rpc->data.swap(custom_data);
			rpc->response_type= _RESPONSE_SET_CUSM;
			rpc->need_send2client = true;
			rpc->save_roleid = roleid;
			rpc->save_link_sid = sid;
			rpc->save_localsid = localsid;
			if (!GameDBClient::GetInstance()->SendProtocol(rpc))
				dsm->Send(sid,SetCustomData_Re(ERR_COMMUNICATION,0/*invalid CRC*/,roleid,localsid));
		}
		dsm->rbcache.UnLock();

	}
};

};

#endif
