// this file defines a series of interface to handle I/O
// of faction logic object.

#ifndef __GNET_GSFIO_H
#define __GNET_GSFIO_H
#include "operwrapper.h"
namespace GNET
{
	/*
	class Octets;
	class FactionOPSyncInfo;
	class OperWrapper::wref_t;
	*/
	//update faction member's info for faction pvpmask change
	void gfs_update_factionpvpmask( int fid ,unsigned char mask);
	//update faction member's info for faction dismiss
	void gfs_update_factiondismiss( int fid );
	//send factionoprequest_re to linkserver or deliveryserver
	bool gfs_send_factionrequest_re(int optype,int roleid,int retcode);
	//send factioncreate_re to linkserver or deliveryserver
	bool gfs_send_factioncreate_re(int roleid,int retcode,int fid,const Octets& faction_name);
	//send factionapplyjoin_re to linkserver
	bool gfs_send_factionapplyjoin_re(int roleid,int retcode,unsigned int fid);
	//send factiondismiss_re to linkserver
	bool gfs_send_factiondismiss_re(int roleid,int retcode);
	//send factionchangeproclaim_re to linkserver
	bool gfs_send_changeproclaim_re(int roleid,int retcode);
	void gfs_broadcast_changeproclaim_re(int fid,int oper_roleid,const Octets& proclaim);
	//send factionlistmember_re to linkserver
	bool gfs_send_listmember_re( int roleid, int retcode );
	void gfs_send_singlemember(int roleid,const FMemberInfo& info);
	//send factionresign_re to linkserver
	bool gfs_send_resign_re( int roleid, int retcode );
	void gfs_broadcast_resign_re(int fid, int resigned_roleid,int old_occup);
	//send factionrename_re to linkserver
	bool gfs_send_rename_re( int roleid, int retcode, int renamed_roleid,const Octets& newname ); 
	void gfs_broadcast_rename_re( int fid,int oper_roleid, int renamed_roleid,const Octets& newname );

	//send factionacceptjoin_re to linkserver
	bool gfs_send_factionacceptjoin_re(int roleid,int retcode,int newmember,int oper_roleid,unsigned int fid=0,int level=0);
	void gfs_broadcast_factionacceptjoin_re(int fid,int oper_roleid,int newmember,int level, char cls,int reputation, unsigned char reincarn_times, unsigned char gender, const Octets&); 
	//send factionexpelmember_re to linkserver
	bool gfs_send_factionexpelmember_re(int roleid,int retcode,int expelroleid);
	void gfs_broadcast_factionexpelmember_re(int fid,int oper_roleid,int expelroleid);
	//send factioninvitejoin to linkserver
	bool gfs_send_factioninvitejoin(int roleid,int invited_roleid,unsigned int fid,const Octets& faction_name,const Octets&);
	
	//send FactionBroadcastNotice_Re to linkserver
	bool gfs_send_broadcast_re(int srcroleid,int dstroleid,const Octets& msg);
	//send FactionMasterResign_Re linkserver
	bool gfs_send_masterresign_re(int roleid,int retcode,int newmaster);
	void gfs_broadcast_masterresign_re(int fid,int newmaster);
	//send FactionAppoint_Re to linkserver
	bool gfs_send_appoint_re(int roleid,int approleid, int retcode, char newoccup);
	void gfs_broadcast_appoint_re(int fid,int oper_roleid, int approleid, char newoccup);
	//send FactionLeave_Re to linkserver
	bool gfs_send_leave_re(int roleid,int retcode);
	void gfs_broadcast_leave_re(int fid,int leave_roleid,char old_occup);
	//send FactionUpgrade_Re
	bool gfs_send_upgradefaction_re(int roleid,int retcode);
	void gfs_broadcast_upgradefaction_re(int fid);
	//send FactionDegrade_Re
	bool gfs_send_degradefaction_re(int roleid,int retcode);
	void gfs_broadcast_degradefaction_re(int fid);

	bool gfs_send_allianceapply_re(int roleid, int retcode);
	void gfs_broadcast_allianceapply_re(int fid, int dst_fid);
	
	bool gfs_send_alliancereply_re(int roleid, int retcode);
	void gfs_broadcast_alliancereply_re(int fid, int dst_fid, char agree);
	
	bool gfs_send_hostileapply_re(int roleid, int retcode);
	void gfs_broadcast_hostileapply_re(int fid, int dst_fid);
	
	bool gfs_send_hostilereply_re(int roleid, int retcode);
	void gfs_broadcast_hostilereply_re(int fid, int dst_fid, char agree);
	
	bool gfs_send_removerelationapply_re(int roleid, int retcode);
	void gfs_broadcast_removerelationapply_re(int fid, int dst_fid, char force);
	
	bool gfs_send_removerelationreply_re(int roleid, int retcode);
	void gfs_broadcast_removerelationreply_re(int fid, int dst_fid, char agree);

	//send factiondelayexpelannounce to linkserver
	bool gfs_send_factiondelayexpelannounce(int roleid,int retcode,char opttype,int oper_roleid,int expel_roleid,int time);
	void gfs_broadcast_factiondelayexpelannounce(int fid,char opttype,int oper_roleid,int expel_roleid,int time);

	void gfs_broadcast_recvrelationapply(int fid, int type, int src_fid);
	void gfs_broadcast_recvrelationreply(int fid, int type, char agree, int src_fid);
	
	//send systemannounce to linkserver or deliveryserver
	bool gfs_send_systemannounce(int roleid,int type,const Octets& msg);

	//send begin synchronize to gameserver
	bool gps_send_beginsync(unsigned int tid, int roleid);
	//send end synchronize to gameserver
	bool gps_send_endsync(unsigned int tid, int roleid,const FactionOPSyncInfo& syncdata);

	//Query UniqueName server when create or delete faction
	bool uns_send_precreatefaction( char zoneid,const Octets& fname,OperWrapper::wref_t oper );
	bool uns_send_postcreatefaction( char blsucc,char zoneid,unsigned int fid,const Octets& fname );
	bool uns_send_postdeletefaction( char zoneid,unsigned int fid,const Octets& fname );
	// faction rename 
	bool gps_send_factionrename_first_re(int roleid, int retcode,const Octets& newname);
	bool gps_send_factionrename_second_re(int roleid, int retcode,const Octets& newname,const Octets& oldname);
	bool uns_send_prefactionrename( char zoneid,unsigned int fid,const Octets& fname,OperWrapper::wref_t oper );
	bool uns_send_postfactionrename( int retcode,char zoneid,unsigned int fid,const Octets& newname , const Octets& oldname = Octets());
	bool gfs_send_dbfactionrename(int roleid,unsigned int fid,int iid,int icount,int ipos,const Octets& newname,const GMailSyncData& sync) ;
	bool gfs_send_factionrenameannounce(int roleid,int retcode,const Octets& fname);
	void gfs_broadcast_factionrenameannounce(unsigned int fid,int roleid,int retcode,const Octets& fname);
};
#endif
