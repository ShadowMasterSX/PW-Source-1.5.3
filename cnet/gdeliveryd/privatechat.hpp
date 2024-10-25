
#ifndef __GNET_PRIVATECHAT_HPP
#define __GNET_PRIVATECHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "rolestatusannounce.hpp"
#include "message"
#include "putmessage.hrp"
#include "gamedbclient.hpp"
#include "gamemaster.h"
#include "mapuser.h"
#include "gametalkmanager.h"
#include "gdeliveryserver.hpp"

namespace GNET
{

static char GM_Name[] = { 'G', 0, 'M', 0};

class PrivateChat : public GNET::Protocol
{
	#include "privatechat"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		unsigned int srclsid = 0;
		unsigned int srclinksid = 0;

		Octets gmname(GM_Name, 4);

		if (channel != CHANNEL_GMREPLY)
		{	
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(srcroleid);
			if(NULL == pinfo)
				return;
			srclsid = pinfo->localsid;
			src_name = pinfo->name;
			emotion = pinfo->emotion;
			srclinksid = pinfo->linksid;

			if(pinfo->IsGM() && (channel==CHANNEL_NORMAL || channel==CHANNEL_NORMALRE))
			{
				src_name.replace(GM_Name, 4);
				srcroleid = -1;
			}
		}
		else
		{
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(srcroleid);
			if((NULL == pinfo || !pinfo->IsGM()) && sid!=dsm->iweb_sid)
				return;
			src_name.replace(GM_Name, 4); 
			srcroleid = -1;
		}

		if(dst_name==gmname)
		{
			MasterContainer::Instance().Broadcast(*this, this->dstroleid);
			return;
		}

		if(dstroleid<=0)
		{
			if(dst_name.size()<MAX_NAME_SIZE)
				UserContainer::GetInstance().FindRoleId( dst_name, dstroleid );
			else
				return;
		}
		
		/*
		if(dstroleid>0)
		{
			bool gtSent = false, clientSent = false;
			GameTalkManager *gtm = GameTalkManager::GetInstance();
			char status = gtm->GetRoleStatus(dstroleid);

			//if role is online only in game, don't send msg to gt, to game instead
			if(channel == CHANNEL_GAMETALK && status == RoleStatusManager::GT_ONLINE_GAME) {
				channel = CHANNEL_NORMAL;
			} else if(channel == CHANNEL_NORMAL || channel == CHANNEL_NORMALRE || channel == CHANNEL_FRIEND || channel == CHANNEL_FRIENDRE) {
				//if role is online only in gt, don't send msg to game, to gt instead
				if(status != RoleStatusManager::GT_OFFLINE && !(status & RoleStatusManager::GT_ONLINE_GAME)) {
					channel = CHANNEL_GAMETALK;
				}
			}

			if(channel == CHANNEL_GAMETALK || channel == CHANNEL_GT_NORMAL || channel == CHANNEL_GT_FRIEND) {
				if(gtm->SendRoleMsg(dstroleid, srcroleid, src_name, msg, emotion)) {
					gtSent = true;
				}
			}

			if(channel == CHANNEL_GT_NORMAL) channel = CHANNEL_NORMAL;
			else if(channel == CHANNEL_GT_FRIEND) channel = CHANNEL_FRIEND;
			else if(channel == CHANNEL_GAMETALK && !gtSent) channel = CHANNEL_NORMAL; //if send to gt failed try to send to game

			if(channel != CHANNEL_GAMETALK) {
				PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(dstroleid);
				if (pinfo)
				{
					if(dsm->Send(pinfo->linksid,this))
						clientSent = true;
				}
			}
			if(gtSent || clientSent) return;
		}
		*/
		if(dstroleid > 0)
		{
			GameTalkManager *gtm = GameTalkManager::GetInstance();
			bool gtSent = false, clientSent = false;
			if(gtm->RoleGameOnline(dstroleid))
			{
				if(gtm->RoleGTOnline(dstroleid))
				{
					switch(channel)
					{
						case CHANNEL_NORMAL:
						case CHANNEL_NORMALRE:
							channel = CHANNEL_GT_NORMAL;
							break;
						case CHANNEL_FRIEND:
						case CHANNEL_FRIENDRE:
							channel = CHANNEL_GT_FRIEND;
							break;
						case CHANNEL_GAMETALK:
							channel = CHANNEL_GT_NORMAL;
							break;
						case CHANNEL_GT_NORMAL:
						case CHANNEL_GT_FRIEND:
						default:
							break;
					}
				}
				else
				{
					switch(channel)
					{
						case CHANNEL_NORMAL:
						case CHANNEL_NORMALRE:
						case CHANNEL_FRIEND:
						case CHANNEL_FRIENDRE:
							break;
						case CHANNEL_GAMETALK:
							channel = CHANNEL_NORMAL;
							break;
						case CHANNEL_GT_NORMAL:
							channel = CHANNEL_NORMAL;
							break;
						case CHANNEL_GT_FRIEND:
							channel = CHANNEL_FRIEND;
							break;
						default:
							break;
					}
				}
			}
			else
			{
				if(gtm->RoleGTOnline(dstroleid))
				{
					switch(channel)
					{
						case CHANNEL_NORMAL:
						case CHANNEL_NORMALRE:
						case CHANNEL_FRIEND:
						case CHANNEL_FRIENDRE:
							channel = CHANNEL_GAMETALK;
							break;
						case CHANNEL_GAMETALK:
							break;
						case CHANNEL_GT_NORMAL:
						case CHANNEL_GT_FRIEND:
							channel = CHANNEL_GAMETALK;
							break;
						default:
							break;
					}
				}
				else
				{
					switch(channel)
					{
						case CHANNEL_NORMAL:
						case CHANNEL_NORMALRE:
						case CHANNEL_FRIEND:
						case CHANNEL_FRIENDRE:
						case CHANNEL_GAMETALK:
							break;
						case CHANNEL_GT_NORMAL:
							channel = CHANNEL_NORMAL;
							break;
						case CHANNEL_GT_FRIEND:
							channel = CHANNEL_FRIEND;
							break;
						default:
							break;
					}
				}
			}

			if(channel == CHANNEL_GAMETALK || channel == CHANNEL_GT_NORMAL || channel == CHANNEL_GT_FRIEND)
			{
				if(gtm->SendRoleMsg(dstroleid,srcroleid,src_name,msg,emotion))
				{
					gtSent = true;
				}
			}

			if(channel == CHANNEL_GT_NORMAL) channel = CHANNEL_NORMAL;
			else if(channel == CHANNEL_GT_FRIEND) channel = CHANNEL_FRIEND;
			else if(channel == CHANNEL_GAMETALK && !gtSent) channel = CHANNEL_NORMAL; //if send to gt failed try to send to game

			if(channel != CHANNEL_GAMETALK) 
			{
				PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(dstroleid);
				if (pinfo)
				{
					if(dsm->Send(pinfo->linksid,this))
						clientSent = true;
				}
			}
			if(gtSent || clientSent) return;
		}

		if(channel==CHANNEL_FRIEND || channel==CHANNEL_GAMETALK)
		{
			Message arg;
			arg.channel = channel;
			arg.srcroleid = srcroleid;
			arg.dstroleid = dstroleid;
			arg.src_name.swap(src_name);
			arg.dst_name.swap(dst_name);
			arg.msg.swap(msg);
			PutMessage* rpc = (PutMessage*) Rpc::Call(RPC_PUTMESSAGE, arg);
			GameDBClient::GetInstance()->SendProtocol(rpc);
			GameTalkManager::GetInstance()->SetOfflineMessage(dstroleid);
		}
		else if (channel!=CHANNEL_GMREPLY)
			dsm->Send(srclinksid,RoleStatusAnnounce(_ZONE_INVALID,_ROLE_INVALID,srclsid,_STATUS_OFFLINE,dst_name));
	}
};

};

#endif
