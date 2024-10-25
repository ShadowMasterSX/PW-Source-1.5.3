#include "gfs_io.h"
#include "macros.h"
/*
#include "factionlistapplicant_re.hpp"
*/
#include "ids.hxx"
#include "factionapplyjoin_re.hpp"
#include "factioncreate_re.hpp"
#include "factionacceptjoin_re.hpp"
#include "factionoprequest_re.hpp"
#include "factionexpel_re.hpp"
#include "factiondismiss_re.hpp"
#include "factionleave_re.hpp"
#include "factionlistmember_re.hpp"
#include "factionrename_re.hpp"
#include "factionappoint_re.hpp"
#include "factionchangproclaim_re.hpp"
#include "factionresign_re.hpp"
#include "factionmasterresign_re.hpp"
#include "factionupgrade_re.hpp"
#include "playerfactioninfo_re.hpp"
#include "factioninvitejoin.hrp"
#include "factionbeginsync.hpp"
#include "factionendsync.hpp"
#include "getuserfaction.hrp"

#include "gfactionserver.hpp"
#include "gproviderserver.hpp"

#include "operwrapper.h"
#include "precreatefaction.hrp"
#include "postcreatefaction.hrp"
#include "postdeletefaction.hrp"
#include "uniquenameclient.hpp"
#include "maperaser.h"

#include "factionallianceapply_re.hpp"
#include "factionalliancereply_re.hpp"
#include "factionhostileapply_re.hpp"
#include "factionhostilereply_re.hpp"
#include "factionremoverelationapply_re.hpp"
#include "factionremoverelationreply_re.hpp"
#include "factionrelationrecvapply.hpp"
#include "factionrelationrecvreply.hpp"
#include "factiondelayexpelannounce.hpp"
#include "factionrenamegsverify.hpp"
#include "prefactionrename.hrp"
#include "postfactionrename.hpp"
#include "dbfactionrename.hrp"
#include "factionrenameannounce.hpp"

#define _SEND_PROTOCOL_(__roleid,__protocol) \
	bool ret=false;\
	GFactionServer::Player player;\
	GFactionServer* gfs=GFactionServer::GetInstance();\
	if (ret=gfs->GetPlayer((__roleid),player))\
	{\
		ret=gfs->DispatchProtocol(player.linkid, (__protocol));\
	}\
	return ret;

#define _BROADCAST_PROTOCOL_(__fid,__protocol)\
	GFactionServer* gfs=GFactionServer::GetInstance();\
	Thread::Mutex::Scoped l(gfs->locker_player);\
	for ( GFactionServer::FactionMemberMap::iterator \
				itb=gfs->factionmembermap.lower_bound(__fid),\
				ite=gfs->factionmembermap.upper_bound(__fid);\
				itb!=ite; ++itb )\
	{\
		if ( !(*itb).second ) continue;\
		GFactionServer::Player &player=*(*itb).second;\
		gfs->DispatchProtocol( player.linkid,\
				__protocol );\
	}
/*
 * 说明：单播借口和广播接口同时存在的情况下，单播借口用于发送错误，广播接口用于发送成功的结果
 */ 
namespace GNET
{
	//update for faction pvpmask
	void gfs_update_factionpvpmask( int fid ,unsigned char mask)
	{
		
		GFactionServer* gfs=GFactionServer::GetInstance();
		Thread::Mutex::Scoped l(gfs->locker_player);
		{
			int cur_gid=-1;
			PlayerFactionInfo_Re pfi_re(ERR_SUCCESS,PFactionInfoVector());
			for ( GFactionServer::FactionMemberMap::iterator 
					itb=gfs->factionmembermap.lower_bound(fid),
					ite=gfs->factionmembermap.upper_bound(fid);
					itb!=ite; ++itb )
			{
				if ( (*itb).second )
				{
					if ( (*itb).second->gsid != cur_gid )
					{
						if ( pfi_re.faction_info.size()>0 )
						{
							GProviderServer::GetInstance()->DispatchProtocol(cur_gid,pfi_re);
							pfi_re.faction_info.GetVector().clear();
						}
						cur_gid=(*itb).second->gsid;
					}
					pfi_re.faction_info.add(PFactionInfo((*itb).second->roleid,fid, (*itb).second->froleid,mask ));
				}
			}
			if ( pfi_re.faction_info.size() )
				GProviderServer::GetInstance()->DispatchProtocol(cur_gid,pfi_re);
		}
	}
	//update for faction dismiss
	void gfs_update_factiondismiss( int fid )
	{
		GFactionServer* gfs=GFactionServer::GetInstance();
		Thread::Mutex::Scoped l(gfs->locker_player);
		{ // begin of scope
			MapEraser<GFactionServer::FactionMemberMap> delFMem(gfs->factionmembermap);
			int cur_gid=-1;
			PlayerFactionInfo_Re pfi_re(ERR_SUCCESS,PFactionInfoVector());
			for ( GFactionServer::FactionMemberMap::iterator 
					itb=gfs->factionmembermap.lower_bound(fid),
					ite=gfs->factionmembermap.upper_bound(fid);
					itb!=ite; ++itb )
			{
				if ( (*itb).second )
				{
					//update member info
					(*itb).second->faction_id=_FACTION_ID_INVALID;
					(*itb).second->froleid=_R_UNMEMBER;
					//notify member
					gfs->DispatchProtocol( (*itb).second->linkid, 
							FactionDismiss_Re(ERR_SUCCESS,(*itb).second->roleid,(*itb).second->localsid) );	
					//notify gameserver
					if ( (*itb).second->gsid != cur_gid )
					{
						if ( pfi_re.faction_info.size()>0 )
						{
							GProviderServer::GetInstance()->DispatchProtocol(cur_gid,pfi_re);
							pfi_re.faction_info.GetVector().clear();
						}
						cur_gid=(*itb).second->gsid;
					}
					pfi_re.faction_info.add(
							PFactionInfo((*itb).second->roleid,
							 	         _FACTION_ID_INVALID,
								         _R_UNMEMBER) );
					delFMem.push(itb); //erase player from factionmemberMap
				}
			}
			if ( pfi_re.faction_info.size() )
				GProviderServer::GetInstance()->DispatchProtocol(cur_gid,pfi_re);
		} //end of scope
	}
	//send  factionrename_re to linkserver
	bool gfs_send_rename_re( int roleid, int retcode, int renamed_roleid,const Octets& newname )
	{
		DEBUG_PRINT("Send factionRename_re  to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionRename_Re(retcode,roleid,player.localsid,renamed_roleid,newname,roleid) );
	}
	void gfs_broadcast_rename_re( int fid,int oper_roleid, int renamed_roleid,const Octets& newname )
	{
		//DEBUG_PRINT("Broadcast factionRename_re to all member. fid=%d, renamed_roleid=%d\n",fid,renamed_roleid);
		/*
		GFactionServer* gfs=GFactionServer::GetInstance();
		Thread::Mutex::Scoped l(gfs->locker_player);
		for ( GFactionServer::FactionMemberMap::iterator 
					itb=gfs->factionmembermap.lower_bound(fid),
					ite=gfs->factionmembermap.upper_bound(fid);
					itb!=ite; ++itb )
		{
			if ( !(*itb).second ) continue;
			GFactionServer::Player &player=*(*itb).second;
			//notify member
			gfs->DispatchProtocol( player.linkid, 
					FactionRename_Re(ERR_SUCCESS,player.roleid,player.localsid,renamed_roleid,newname,oper_roleid) );	

		}
		*/
		_BROADCAST_PROTOCOL_( fid, FactionRename_Re(
										ERR_SUCCESS,
										player.roleid,
										player.localsid,
										renamed_roleid,
										newname,
										oper_roleid) 
							);
	}
	//send factionappoint_re to linkserver
	bool gfs_send_appoint_re(int roleid,int approleid, int retcode, char newoccup)
	{
		DEBUG_PRINT("Send factionAppoint_re  to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionAppoint_Re(retcode,roleid,player.localsid,approleid,newoccup,roleid) );
	}
	void gfs_broadcast_appoint_re(int fid, int oper_roleid, int approleid, char newoccup)
	{
		DEBUG_PRINT("Broadcast factionAppoint_re to all member. fid=%d, app_roleid=%d,newoccup=%d\n",fid,approleid,newoccup);
		_BROADCAST_PROTOCOL_( fid, FactionAppoint_Re(
										ERR_SUCCESS,
										player.roleid,
										player.localsid,
										approleid,
										newoccup,
										oper_roleid) 
							);
	}
	//send factionoprequest_re to linkserver or deliveryserver
	bool gfs_send_factionrequest_re(int optype,int roleid,int retcode)
	{
		//DEBUG_PRINT("Send factionOprequest_re  to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionOPRequest_Re(retcode,optype,roleid,player.linkid,player.localsid) )
	}
	//send factionresign_re to linkserver
	bool gfs_send_resign_re( int roleid,int retcode )
	{
		DEBUG_PRINT("Send factionresign_re  to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionResign_Re(retcode,roleid,player.localsid,roleid,player.froleid/*old occup*/) );
	}
	void gfs_broadcast_resign_re(int fid, int resigned_roleid,int old_occup)
	{
		DEBUG_PRINT("Broadcast factionResign_re to all member. fid=%d, resigned_roleid=%d,oldoccup=%d\n",
				fid,resigned_roleid,old_occup);
		_BROADCAST_PROTOCOL_( fid, FactionResign_Re(
										ERR_SUCCESS,
										player.roleid,
										player.localsid,
										resigned_roleid,
										old_occup)
							);
	}
	//send factioncreate_re to linkserver or deliveryserver
	bool gfs_send_factioncreate_re(int roleid,int retcode,int fid, const Octets& faction_name)
	{
		DEBUG_PRINT("Send Create faction to client(roleid=%d). retcode=%d\n",roleid,retcode);
		if(retcode==45)
			retcode = 105;
		_SEND_PROTOCOL_( roleid, FactionCreate_Re(retcode,faction_name,fid,roleid,player.localsid) );
	}
	//send factionchangeproclaim_re to linkserver
	bool gfs_send_changeproclaim_re(int roleid,int retcode)
	{
		DEBUG_PRINT("Send ChangeProclaim_Re to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionChangProclaim_Re(retcode,roleid,player.localsid,roleid,Octets()) );
	}
	void gfs_broadcast_changeproclaim_re(int fid,int oper_roleid,const Octets& proclaim)
	{
		DEBUG_PRINT("Broadcast factionChangeProclaim_re to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionChangProclaim_Re(
										ERR_SUCCESS,
										player.roleid,
										player.localsid,
										oper_roleid,
										proclaim)
							);
	}
	//send factionlistmember_re to linkserver
	bool gfs_send_listmember_re( int roleid, int retcode )
	{
		DEBUG_PRINT("Send list faction member to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionListMember_Re(
									-1,
									player.roleid,
									player.localsid,
									Octets(),
									FMemberInfoVector(),
									IntVector()) 
				       );
	}
	void gfs_send_singlemember(int roleid,const FMemberInfo& info)
	{
		FactionListMember_Re protocol;	
	    	    
		protocol.handle = 2; // mean single update
		protocol.member_list.add(info);

		GFactionServer* gfs=GFactionServer::GetInstance();
		Thread::Mutex::Scoped l(gfs->locker_player);
		GFactionServer::Player player;
		if (gfs->GetPlayer((roleid),player))
		{
			protocol.localsid = player.localsid;
			gfs->DispatchProtocol( player.linkid, protocol );
		}

	}
	//send factionapplyjoin_re to linkserver
	bool gfs_send_factionapplyjoin_re(int roleid,int retcode,unsigned int fid)
	{
		DEBUG_PRINT("Send apply faction(fid=%d) to client(roleid=%d). retcode=%d\n",fid,roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionApplyJoin_Re(retcode,roleid,player.localsid,fid) ); 
	}
    //send factiondismiss_re to linkserver
    bool gfs_send_factiondismiss_re(int roleid,int retcode)
	{
		DEBUG_PRINT("Send faction dismiss to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionDismiss_Re(retcode,roleid,player.localsid) ); 
	}
	//send factionmasterresign_re to linkserver
	bool gfs_send_masterresign_re(int roleid,int retcode,int newmaster)
	{
		DEBUG_PRINT("Send faction master_resign_re to client(roleid=%d), retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionMasterResign_Re(retcode,roleid,player.localsid,newmaster) );
	}
	void gfs_broadcast_masterresign_re(int fid,int newmaster)
	{
		DEBUG_PRINT("Broadcast factionMaster_re to all member. fid=%d,new master=%d\n",fid,newmaster );
		_BROADCAST_PROTOCOL_( fid, FactionMasterResign_Re(
										ERR_SUCCESS,
										player.roleid,
										player.localsid,
										newmaster)
							);
	}
	//send factionleave_re to linkserver
	bool gfs_send_leave_re(int roleid,int retcode)
	{
		DEBUG_PRINT("Send leave faction result to player %d, retcode %d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionLeave_Re( retcode,roleid,player.localsid,roleid,player.froleid/*old occup*/ ) );
	}
	void gfs_broadcast_leave_re(int fid,int leave_roleid,char old_occup)
	{
		DEBUG_PRINT("Broadcast factionLeave_re to all member. fid=%d,leave_role=%d,old_occup=%d\n",fid,leave_roleid,old_occup );
		_BROADCAST_PROTOCOL_( fid, FactionLeave_Re(
										ERR_SUCCESS,
										player.roleid,
										player.localsid,
										leave_roleid,
										old_occup)
							);
	}
	//send factionacceptjoin_re to linkserver
	bool gfs_send_factionacceptjoin_re(int roleid,int retcode,int newmember,int oper_roleid,unsigned int fid,int level)
	{
		DEBUG_PRINT("Send accept player(roleid=%d) to faction to client(roleid=%d). retcode=%d\n",newmember,roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionAcceptJoin_Re(retcode,roleid,player.localsid,newmember,oper_roleid,fid,level) ); 
	}
	void gfs_broadcast_factionacceptjoin_re(int fid,int oper_roleid,int newmember,int level, char cls, int reputation, unsigned char reincarn_times, unsigned char gender, const Octets& name)
	{
		DEBUG_PRINT("Broadcast factionAcceptJoin_re to all member. fid=%d,newmember=%d\n",fid,newmember );
		_BROADCAST_PROTOCOL_( fid, FactionAcceptJoin_Re(
										ERR_SUCCESS,
										player.roleid,
										player.localsid,
										newmember,
										oper_roleid,
										fid,
										level,
										cls,
										name,
										reputation,
										reincarn_times,
										gender)
							);
	}
	//send factionexpelmember_re to linkserver
	bool gfs_send_factionexpelmember_re(int roleid,int retcode,int expelroleid)
	{
		DEBUG_PRINT("Send expel member %d to client(roleid=%d). retcode=%d\n",expelroleid,roleid,retcode);
		_SEND_PROTOCOL_( roleid,FactionExpel_Re(retcode,roleid,player.localsid,expelroleid,roleid) ); 
	}
	void gfs_broadcast_factionexpelmember_re(int fid,int oper_roleid,int expelroleid)
	{
		DEBUG_PRINT("Broadcast factionExpel_re to all member. fid=%d,expelroleid=%d\n",fid,expelroleid );
		_BROADCAST_PROTOCOL_( fid, FactionExpel_Re(
										ERR_SUCCESS,
										player.roleid,
										player.localsid,
										expelroleid,
										oper_roleid)
							);
	}
    //send FactionUpgrade_Re
    bool gfs_send_upgradefaction_re(int roleid,int retcode)
	{
		DEBUG_PRINT("Send FactionUpgrade_Re to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionUpgrade_Re(retcode,roleid,player.localsid) );
	}
    void gfs_broadcast_upgradefaction_re(int fid)
	{
		DEBUG_PRINT("Broadcast FactionUpgrade_Re to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionUpgrade_Re( 
										ERR_SUCCESS,
										player.roleid,
										player.localsid)
							);
	}
	//send factioninvitejoin to linkserver
	bool gfs_send_factioninvitejoin(int roleid,int invitee,unsigned int fid,const Octets& faction_name,const Octets& name)
	{
		DEBUG_PRINT("Send factioninvite RPC roleid(%d), invited_roleid(%d) to faction(fid=%d)\n",roleid,invitee,fid);
		_SEND_PROTOCOL_( invitee, Rpc::Call( RPC_FACTIONINVITEJOIN, 
					FactionInviteArg(roleid,invitee,fid,faction_name,name)));
	}

	bool gfs_send_allianceapply_re(int roleid, int retcode)
	{
		DEBUG_PRINT("Send FactionAllianceApply_Re to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionAllianceApply_Re(retcode,0,player.localsid) );
	}

	void gfs_broadcast_allianceapply_re(int fid, int dst_fid)
	{
		DEBUG_PRINT("Broadcast FactionAllianceApply_Re to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionAllianceApply_Re( 
										ERR_SUCCESS,
										dst_fid,
										player.localsid)
							);
	}

	bool gfs_send_alliancereply_re(int roleid, int retcode)
	{
		DEBUG_PRINT("Send FactionAllianceReply_Re to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionAllianceReply_Re(retcode,0,0,player.localsid) );
	}
	
	void gfs_broadcast_alliancereply_re(int fid, int dst_fid, char agree)
	{
		DEBUG_PRINT("Broadcast FactionAllianceReply_Re to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionAllianceReply_Re( 
										ERR_SUCCESS,
										dst_fid,
										agree,
										player.localsid)
							);
	}

	bool gfs_send_hostileapply_re(int roleid, int retcode)
	{
		DEBUG_PRINT("Send FactionHostileApply_Re to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionHostileApply_Re(retcode,0,player.localsid) );
	}
	
	void gfs_broadcast_hostileapply_re(int fid, int dst_fid)
	{
		DEBUG_PRINT("Broadcast FactionHostileApply_Re to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionHostileApply_Re( 
										ERR_SUCCESS,
										dst_fid,
										player.localsid)
							);
	}

	bool gfs_send_hostilereply_re(int roleid, int retcode)
	{
		DEBUG_PRINT("Send FactionHostileReply_Re to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionHostileReply_Re(retcode,0,0,player.localsid) );
	}
	
	void gfs_broadcast_hostilereply_re(int fid, int dst_fid, char agree)
	{
		DEBUG_PRINT("Broadcast FactionHostileReply_Re to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionHostileReply_Re( 
										ERR_SUCCESS,
										dst_fid,
										agree,
										player.localsid)
							);
	}

	bool gfs_send_removerelationapply_re(int roleid, int retcode)
	{
		DEBUG_PRINT("Send FactionRemoveRelationApply_Re to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionRemoveRelationApply_Re(retcode,0,0,player.localsid) );
	}
	
	void gfs_broadcast_removerelationapply_re(int fid, int dst_fid, char force)
	{
		DEBUG_PRINT("Broadcast FactionRemoveRelationApply_Re to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionRemoveRelationApply_Re( 
										ERR_SUCCESS,
										dst_fid,
										force,
										player.localsid)
							);
	}

	bool gfs_send_removerelationreply_re(int roleid, int retcode)
	{
		DEBUG_PRINT("Send FactionRemoveRelationReply_Re to client(roleid=%d). retcode=%d\n",roleid,retcode);
		_SEND_PROTOCOL_( roleid, FactionRemoveRelationReply_Re(retcode,0,0,player.localsid) );
	}
	
	void gfs_broadcast_removerelationreply_re(int fid, int dst_fid, char agree)
	{
		DEBUG_PRINT("Broadcast FactionRemoveRelationReply_Re to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionRemoveRelationReply_Re( 
										ERR_SUCCESS,
										dst_fid,
										agree,
										player.localsid)
							);
	}

	bool gfs_send_factiondelayexpelannounce(int roleid,int retcode,char opttype,int oper_roleid,int expel_roleid,int time)
	{
	    	DEBUG_PRINT("Send FactionDelayExpelAnnounce to client(roleid=%d).ret=%d,opt=%d,opter=%d,expel=%d,time=%d\n",
			roleid,retcode,opttype,oper_roleid,expel_roleid,time);
		_SEND_PROTOCOL_( roleid, 
			FactionDelayExpelAnnounce(retcode,opttype,oper_roleid,expel_roleid,time,player.localsid) );
	}

	void gfs_broadcast_factiondelayexpelannounce(int fid,char opttype,int oper_roleid,int expel_roleid,int time)
	{
	    	DEBUG_PRINT("Broadcast FactionDelayExpelAnnounce to all member.fid=%d,opt=%d,opter=%d,expel=%d,time=%d\n",
			fid,opttype,oper_roleid,expel_roleid,time);
		_BROADCAST_PROTOCOL_( fid, 
			FactionDelayExpelAnnounce(ERR_SUCCESS,opttype,oper_roleid,expel_roleid,time,player.localsid) );

	}

	void gfs_broadcast_recvrelationapply(int fid, int type, int src_fid)
	{
		DEBUG_PRINT("Broadcast FactionRelationRecvApply to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionRelationRecvApply( 
										type,
										src_fid,
										player.localsid)
							);
	}
	
	void gfs_broadcast_recvrelationreply(int fid, int type, char agree, int src_fid)
	{
		DEBUG_PRINT("Broadcast FactionRelationRecvReply to all member. fid=%d\n",fid );
		_BROADCAST_PROTOCOL_( fid, FactionRelationRecvReply( 
										type,
										agree,
										src_fid,
										player.localsid)
							);
	}
	
	//send systemannounce to linkserver or deliveryserver
	bool gfs_send_systemannounce(int roleid,int localsid,int type,const Octets& msg);
	
	//send begin synchronize to gameserver
	bool gps_send_beginsync(unsigned int tid, int roleid)
	{
		
		bool ret=false;
		GFactionServer* gfs=GFactionServer::GetInstance();
		GFactionServer::Player player;
		if (ret=gfs->GetPlayer(roleid,player))
		{
			ret=GProviderServer::GetInstance()->DispatchProtocol(player.gsid,FactionBeginSync(tid,roleid));
		}
		return ret;
		
	}
	
	//send end synchronize to gameserver
	bool gps_send_endsync(unsigned int tid, int roleid,const FactionOPSyncInfo& syncdata)
	{
		bool ret=false;
		GFactionServer* gfs=GFactionServer::GetInstance();
		GFactionServer::Player player;
		if (ret=gfs->GetPlayer(roleid,player))
		{
			ret=GProviderServer::GetInstance()->DispatchProtocol(player.gsid,
				FactionEndSync(tid,roleid,syncdata));
		}
		return ret;
	}
    //Query UniqueName server when create or delete faction
    bool uns_send_precreatefaction( char zoneid,const Octets& fname,OperWrapper::wref_t oper )
	{
		PreCreateFaction* rpc=(PreCreateFaction*) Rpc::Call(
				RPC_PRECREATEFACTION,
				PreCreateFactionArg(zoneid,fname)
			);
		rpc->oper=oper;
		return UniqueNameClient::GetInstance()->SendProtocol( rpc );
	}
    bool uns_send_postcreatefaction( char blsucc,char zoneid,unsigned int fid,const Octets& fname )
	{
		return UniqueNameClient::GetInstance()->IsConnect() &&
			   UniqueNameClient::GetInstance()->SendProtocol(
				Rpc::Call(
					RPC_POSTCREATEFACTION,
					PostCreateFactionArg(blsucc,zoneid,fid,fname)
					)
				);	
	}
    bool uns_send_postdeletefaction( char zoneid,unsigned int fid,const Octets& fname )
	{
		return  UniqueNameClient::GetInstance()->IsConnect() &&
				UniqueNameClient::GetInstance()->SendProtocol(
				Rpc::Call(
					RPC_POSTDELETEFACTION,
					PostDeleteFactionArg(zoneid,fid,fname)
					)
				);	
	}
	bool gps_send_factionrename_first_re(int roleid, int retcode,const Octets& newname)
	{
		bool ret=false;
		GFactionServer* gfs=GFactionServer::GetInstance();
		GFactionServer::Player player;
		if (ret=gfs->GetPlayer(roleid,player))
		{
			ret=GProviderServer::GetInstance()->DispatchProtocol(player.gsid,
				FactionRenameGsVerify(0,roleid,player.faction_id,retcode,newname));
		}
		return ret;
	}
	bool gps_send_factionrename_second_re(int roleid, int retcode,const Octets& newname,const Octets& oldname)
	{
		bool ret=false;
		GFactionServer* gfs=GFactionServer::GetInstance();
		GFactionServer::Player player;
		if (ret=gfs->GetPlayer(roleid,player))
		{
			ret=GProviderServer::GetInstance()->DispatchProtocol(player.gsid,
				FactionRenameGsVerify(1,roleid,player.faction_id,retcode,newname,oldname));
		}
		return ret;
	}
	bool uns_send_prefactionrename( char zoneid,unsigned int fid,const Octets& fname,OperWrapper::wref_t oper )
	{
		if(!UniqueNameClient::GetInstance()->IsConnect())
			return false;
		PreFactionRename* rpc=(PreFactionRename*) Rpc::Call(
				RPC_PREFACTIONRENAME,
				PreFactionRenameArg(zoneid,fid,fname)
			);
		rpc->oper=oper;
		return UniqueNameClient::GetInstance()->SendProtocol( rpc );
	}
	bool uns_send_postfactionrename( int retcode,char zoneid,unsigned int fid,const Octets& newname , const Octets& oldname)
	{
		return UniqueNameClient::GetInstance()->IsConnect() &&
			UniqueNameClient::GetInstance()->SendProtocol(
			PostFactionRename(zoneid,fid,retcode,newname,oldname));
	}
	bool gfs_send_dbfactionrename(int roleid,unsigned int fid,int iid,int icount,int ipos,const Octets& newname,const GMailSyncData& sync) 
	{
		DBFactionRename* rpc=(DBFactionRename*) Rpc::Call(
				RPC_DBFACTIONRENAME,
				DBFactionRenameArg(roleid,fid,iid,icount,ipos,newname,sync));
		return GFactionServer::GetInstance()->DispatchProtocol(0, rpc );

	}
	bool gfs_send_factionrenameannounce(int roleid,int retcode,const Octets& fname)
	{
		_SEND_PROTOCOL_( roleid, FactionRenameAnnounce(retcode,roleid,fname,player.localsid)) ;
	}
	void gfs_broadcast_factionrenameannounce(unsigned int fid,int roleid, int retcode,const Octets& fname)
	{
		_BROADCAST_PROTOCOL_( fid, FactionRenameAnnounce(retcode,roleid,fname,player.localsid)); 
	}

};

