#ifdef WIN32
#include <winsock2.h>
#include "gncompress.h"
#else
#include "binder.h"
#endif
#include "prefactionrename.hrp"
#include "dbfactionrename.hrp"
#include "announcefactionroledel.hrp"
#include "factioninvitejoin.hrp"
#include "addfaction.hrp"
#include "delfaction.hrp"
#include "addmember.hrp"
#include "delmember.hrp"
#include "delmemberschedule.hrp"
#include "updateuserfaction.hrp"
#include "updatefaction.hrp"
#include "dbfactionupgrade.hrp"
#include "dbfactionpromote.hrp"
#include "getfactioninfo.hrp"
#include "getuserfaction.hrp"
#include "getfactiondetail.hrp"
#include "dbfactionallianceapply.hrp"
#include "dbfactionalliancereply.hrp"
#include "dbfactionhostileapply.hrp"
#include "dbfactionhostilereply.hrp"
#include "dbfactionremoverelationapply.hrp"
#include "dbfactionremoverelationreply.hrp"
#include "dbfactionrelationtimeout.hrp"
#include "dbfactionrelationapplytimeout.hrp"
#include "precreaterole.hrp"
#include "postcreaterole.hrp"
#include "postdeleterole.hrp"
#include "precreatefaction.hrp"
#include "postcreatefaction.hrp"
#include "postdeletefaction.hrp"
#include "precreatefamily.hrp"
#include "postcreatefamily.hrp"
#include "postdeletefamily.hrp"
#include "preplayerrename.hrp"
#include "playerfactioninfo_re.hpp"
#include "factionoprequest_re.hpp"
#include "factioncreate_re.hpp"
#include "factionapplyjoin_re.hpp"
#include "factionlistmember_re.hpp"
#include "factionacceptjoin_re.hpp"
#include "factionexpel_re.hpp"
#include "factionbroadcastnotice_re.hpp"
#include "factionmasterresign_re.hpp"
#include "factionappoint_re.hpp"
#include "factionresign_re.hpp"
#include "factionchangproclaim_re.hpp"
#include "factionleave_re.hpp"
#include "factiondismiss_re.hpp"
#include "factionrename_re.hpp"
#include "factionupgrade_re.hpp"
#include "factiondegrade_re.hpp"
#include "getfactionbaseinfo.hpp"
#include "getfactionbaseinfo_re.hpp"
#include "getplayerfactioninfo.hpp"
#include "getplayerfactioninfo_re.hpp"
#include "factionlistonline_re.hpp"
#include "factionallianceapply_re.hpp"
#include "factionalliancereply_re.hpp"
#include "factionhostileapply_re.hpp"
#include "factionhostilereply_re.hpp"
#include "factionremoverelationapply_re.hpp"
#include "factionremoverelationreply_re.hpp"
#include "factionlistrelation_re.hpp"
#include "factionrelationrecvapply.hpp"
#include "factionrelationrecvreply.hpp"
#include "getplayerfactionrelation_re.hpp"
#include "factiondelayexpelannounce.hpp"
#include "keepalive.hpp"
#include "factionbeginsync_re.hpp"
#include "factionbeginsync.hpp"
#include "factionendsync.hpp"
#include "delfactionannounce.hpp"
#include "battleenter_re.hpp"
#include "worldchat.hpp"
#include "factionmemberupdate.hpp"
#include "factioninfoupdate.hpp"
#include "factionmsg.hpp"
#include "factionforbidupdate.hpp"
#include "playernameupdate.hpp"
#include "factionrenamegsverify.hpp"
#include "postfactionrename.hpp"
#include "factionrenameannounce.hpp"
#include "announcezoneid.hpp"
#include "announceproviderid.hpp"
#include "factioncreate.hpp"
#include "playerstatusannounce.hpp"
#include "playerfactioninfo.hpp"
#include "factionoprequest.hpp"
#include "factionacceptjoin.hpp"
#include "factionlistonline.hpp"
#include "battlechallengemap_re.hpp"
#include "chatbroadcast.hpp"
#include "factionchat.hpp"
#include "setchatemotion.hpp"
#include "battlefactionnotice.hpp"
#include "notifyfactionfortressid.hpp"
#include "announcecentraldelivery.hpp"
#include "notifyfactionplayerrename.hpp"
#include "factionresourcebattlelimitnotice.hpp"
#include "factionuniqueidannounce.hpp"
#include "battleenter.hpp"
#include "sendbattlechallenge.hpp"
#include "createfactionfortress.hpp"
#include "getplayerfactionrelation.hpp"
#include "factioncongregaterequest.hpp"
#include "playersendmassmail.hpp"
#include "factionrenamegsverify_re.hpp"

namespace GNET
{

static PreFactionRename __stub_PreFactionRename (RPC_PREFACTIONRENAME, new PreFactionRenameArg, new PreFactionRenameRes);
static DBFactionRename __stub_DBFactionRename (RPC_DBFACTIONRENAME, new DBFactionRenameArg, new DBFactionRenameRes);
static AnnounceFactionRoleDel __stub_AnnounceFactionRoleDel (RPC_ANNOUNCEFACTIONROLEDEL, new AnnounceFactionRoleDelArg, new AnnounceFactionRoleDelRes);
static FactionInviteJoin __stub_FactionInviteJoin (RPC_FACTIONINVITEJOIN, new FactionInviteArg, new FactionInviteRes);
static AddFaction __stub_AddFaction (RPC_ADDFACTION, new AddFactionArg, new AddFactionRes);
static DelFaction __stub_DelFaction (RPC_DELFACTION, new FactionId, new DelFactionRes);
static AddMember __stub_AddMember (RPC_ADDMEMBER, new AddMemberArg, new AddMemberRes);
static DelMember __stub_DelMember (RPC_DELMEMBER, new DelMemberArg, new DefFactionRes);
static DelMemberSchedule __stub_DelMemberSchedule (RPC_DELMEMBERSCHEDULE, new DelMemberScheduleArg, new DefFactionRes);
static UpdateUserFaction __stub_UpdateUserFaction (RPC_UPDATEUSERFACTION, new UpdateUserFactionArg, new UserFactionRes);
static UpdateFaction __stub_UpdateFaction (RPC_UPDATEFACTION, new UpdateFactionArg, new DefFactionRes);
static DBFactionUpgrade __stub_DBFactionUpgrade (RPC_DBFACTIONUPGRADE, new DBFactionUpgradeArg, new DBFactionUpgradeRes);
static DBFactionPromote __stub_DBFactionPromote (RPC_DBFACTIONPROMOTE, new DBFactionPromoteArg, new DBFactionPromoteRes);
static GetFactionInfo __stub_GetFactionInfo (RPC_GETFACTIONINFO, new FactionId, new FactionInfoRes);
static GetUserFaction __stub_GetUserFaction (RPC_GETUSERFACTION, new UserFactionArg, new UserFactionRes);
static GetFactionDetail __stub_GetFactionDetail (RPC_GETFACTIONDETAIL, new FactionId, new FactionDetailRes);
static DBFactionAllianceApply __stub_DBFactionAllianceApply (RPC_DBFACTIONALLIANCEAPPLY, new DBFactionAllianceApplyArg, new RpcRetcode);
static DBFactionAllianceReply __stub_DBFactionAllianceReply (RPC_DBFACTIONALLIANCEREPLY, new DBFactionAllianceReplyArg, new DBFactionRelationRetcode);
static DBFactionHostileApply __stub_DBFactionHostileApply (RPC_DBFACTIONHOSTILEAPPLY, new DBFactionHostileApplyArg, new RpcRetcode);
static DBFactionHostileReply __stub_DBFactionHostileReply (RPC_DBFACTIONHOSTILEREPLY, new DBFactionHostileReplyArg, new DBFactionRelationRetcode);
static DBFactionRemoveRelationApply __stub_DBFactionRemoveRelationApply (RPC_DBFACTIONREMOVERELATIONAPPLY, new DBFactionRemoveRelationApplyArg, new DBFactionRelationRetcode);
static DBFactionRemoveRelationReply __stub_DBFactionRemoveRelationReply (RPC_DBFACTIONREMOVERELATIONREPLY, new DBFactionRemoveRelationReplyArg, new DBFactionRelationRetcode);
static DBFactionRelationTimeout __stub_DBFactionRelationTimeout (RPC_DBFACTIONRELATIONTIMEOUT, new DBFactionRelationTimeoutArg, new DBFactionRelationRetcode);
static DBFactionRelationApplyTimeout __stub_DBFactionRelationApplyTimeout (RPC_DBFACTIONRELATIONAPPLYTIMEOUT, new DBFactionRelationApplyTimeoutArg, new DBFactionRelationRetcode);
static PreCreateRole __stub_PreCreateRole (RPC_PRECREATEROLE, new PreCreateRoleArg, new PreCreateRoleRes);
static PostCreateRole __stub_PostCreateRole (RPC_POSTCREATEROLE, new PostCreateRoleArg, new PostCreateRoleRes);
static PostDeleteRole __stub_PostDeleteRole (RPC_POSTDELETEROLE, new PostDeleteRoleArg, new PostDeleteRoleRes);
static PreCreateFaction __stub_PreCreateFaction (RPC_PRECREATEFACTION, new PreCreateFactionArg, new PreCreateFactionRes);
static PostCreateFaction __stub_PostCreateFaction (RPC_POSTCREATEFACTION, new PostCreateFactionArg, new PostCreateFactionRes);
static PostDeleteFaction __stub_PostDeleteFaction (RPC_POSTDELETEFACTION, new PostDeleteFactionArg, new PostDeleteFactionRes);
static PreCreateFamily __stub_PreCreateFamily (RPC_PRECREATEFAMILY, new PreCreateFamilyArg, new PreCreateFamilyRes);
static PostCreateFamily __stub_PostCreateFamily (RPC_POSTCREATEFAMILY, new PostCreateFamilyArg, new PostCreateFamilyRes);
static PostDeleteFamily __stub_PostDeleteFamily (RPC_POSTDELETEFAMILY, new PostDeleteFamilyArg, new PostDeleteFamilyRes);
static PrePlayerRename __stub_PrePlayerRename (RPC_PREPLAYERRENAME, new PrePlayerRenameArg, new PrePlayerRenameRes);
static PlayerFactionInfo_Re __stub_PlayerFactionInfo_Re((void*)0);
static FactionOPRequest_Re __stub_FactionOPRequest_Re((void*)0);
static FactionCreate_Re __stub_FactionCreate_Re((void*)0);
static FactionApplyJoin_Re __stub_FactionApplyJoin_Re((void*)0);
static FactionListMember_Re __stub_FactionListMember_Re((void*)0);
static FactionAcceptJoin_Re __stub_FactionAcceptJoin_Re((void*)0);
static FactionExpel_Re __stub_FactionExpel_Re((void*)0);
static FactionBroadcastNotice_Re __stub_FactionBroadcastNotice_Re((void*)0);
static FactionMasterResign_Re __stub_FactionMasterResign_Re((void*)0);
static FactionAppoint_Re __stub_FactionAppoint_Re((void*)0);
static FactionResign_Re __stub_FactionResign_Re((void*)0);
static FactionChangProclaim_Re __stub_FactionChangProclaim_Re((void*)0);
static FactionLeave_Re __stub_FactionLeave_Re((void*)0);
static FactionDismiss_Re __stub_FactionDismiss_Re((void*)0);
static FactionRename_Re __stub_FactionRename_Re((void*)0);
static FactionUpgrade_Re __stub_FactionUpgrade_Re((void*)0);
static FactionDegrade_Re __stub_FactionDegrade_Re((void*)0);
static GetFactionBaseInfo __stub_GetFactionBaseInfo((void*)0);
static GetFactionBaseInfo_Re __stub_GetFactionBaseInfo_Re((void*)0);
static GetPlayerFactionInfo __stub_GetPlayerFactionInfo((void*)0);
static GetPlayerFactionInfo_Re __stub_GetPlayerFactionInfo_Re((void*)0);
static FactionListOnline_Re __stub_FactionListOnline_Re((void*)0);
static FactionAllianceApply_Re __stub_FactionAllianceApply_Re((void*)0);
static FactionAllianceReply_Re __stub_FactionAllianceReply_Re((void*)0);
static FactionHostileApply_Re __stub_FactionHostileApply_Re((void*)0);
static FactionHostileReply_Re __stub_FactionHostileReply_Re((void*)0);
static FactionRemoveRelationApply_Re __stub_FactionRemoveRelationApply_Re((void*)0);
static FactionRemoveRelationReply_Re __stub_FactionRemoveRelationReply_Re((void*)0);
static FactionListRelation_Re __stub_FactionListRelation_Re((void*)0);
static FactionRelationRecvApply __stub_FactionRelationRecvApply((void*)0);
static FactionRelationRecvReply __stub_FactionRelationRecvReply((void*)0);
static GetPlayerFactionRelation_Re __stub_GetPlayerFactionRelation_Re((void*)0);
static FactionDelayExpelAnnounce __stub_FactionDelayExpelAnnounce((void*)0);
static KeepAlive __stub_KeepAlive((void*)0);
static FactionBeginSync_Re __stub_FactionBeginSync_Re((void*)0);
static FactionBeginSync __stub_FactionBeginSync((void*)0);
static FactionEndSync __stub_FactionEndSync((void*)0);
static DelFactionAnnounce __stub_DelFactionAnnounce((void*)0);
static BattleEnter_Re __stub_BattleEnter_Re((void*)0);
static WorldChat __stub_WorldChat((void*)0);
static FactionMemberUpdate __stub_FactionMemberUpdate((void*)0);
static FactionInfoUpdate __stub_FactionInfoUpdate((void*)0);
static FactionMsg __stub_FactionMsg((void*)0);
static FactionForbidUpdate __stub_FactionForbidUpdate((void*)0);
static PlayerNameUpdate __stub_PlayerNameUpdate((void*)0);
static FactionRenameGsVerify __stub_FactionRenameGsVerify((void*)0);
static PostFactionRename __stub_PostFactionRename((void*)0);
static FactionRenameAnnounce __stub_FactionRenameAnnounce((void*)0);
static AnnounceZoneid __stub_AnnounceZoneid((void*)0);
static AnnounceProviderID __stub_AnnounceProviderID((void*)0);
static FactionCreate __stub_FactionCreate((void*)0);
static PlayerStatusAnnounce __stub_PlayerStatusAnnounce((void*)0);
static PlayerFactionInfo __stub_PlayerFactionInfo((void*)0);
static FactionOPRequest __stub_FactionOPRequest((void*)0);
static FactionAcceptJoin __stub_FactionAcceptJoin((void*)0);
static FactionListOnline __stub_FactionListOnline((void*)0);
static BattleChallengeMap_Re __stub_BattleChallengeMap_Re((void*)0);
static ChatBroadCast __stub_ChatBroadCast((void*)0);
static FactionChat __stub_FactionChat((void*)0);
static SetChatEmotion __stub_SetChatEmotion((void*)0);
static BattleFactionNotice __stub_BattleFactionNotice((void*)0);
static NotifyFactionFortressID __stub_NotifyFactionFortressID((void*)0);
static AnnounceCentralDelivery __stub_AnnounceCentralDelivery((void*)0);
static NotifyFactionPlayerRename __stub_NotifyFactionPlayerRename((void*)0);
static FactionResourceBattleLimitNotice __stub_FactionResourceBattleLimitNotice((void*)0);
static FactionUniqueIdAnnounce __stub_FactionUniqueIdAnnounce((void*)0);
static BattleEnter __stub_BattleEnter((void*)0);
static SendBattleChallenge __stub_SendBattleChallenge((void*)0);
static CreateFactionFortress __stub_CreateFactionFortress((void*)0);
static GetPlayerFactionRelation __stub_GetPlayerFactionRelation((void*)0);
static FactionCongregateRequest __stub_FactionCongregateRequest((void*)0);
static PlayerSendMassMail __stub_PlayerSendMassMail((void*)0);
static FactionRenameGsVerify_Re __stub_FactionRenameGsVerify_Re((void*)0);

};
