#ifdef WIN32
#include <winsock2.h>
#include "gncompress.h"
#else
#include "binder.h"
#endif
#include "addfriendrqst.hrp"
#include "tradestartrqst.hrp"
#include "factioninvitejoin.hrp"
#include "playerpositionresetrqst.hrp"
#include "userlogin.hrp"
#include "userlogin2.hrp"
#include "gquerypasswd.hrp"
#include "dbsellpoint.hrp"
#include "dbbuypoint.hrp"
#include "matrixpasswd.hrp"
#include "matrixpasswd2.hrp"
#include "matrixtoken.hrp"
#include "playeridentitymatch.hrp"
#include "auctionexitbid.hpp"
#include "errorinfo.hpp"
#include "challenge.hpp"
#include "selectrole_re.hpp"
#include "playerlogin.hpp"
#include "playeroffline.hpp"
#include "playerstatussync.hpp"
#include "c2sgamedatasend.hpp"
#include "chatmessage.hpp"
#include "worldchat.hpp"
#include "announcenewmail.hpp"
#include "acwhoami.hpp"
#include "acprotostat.hpp"
#include "reportip.hpp"
#include "sellpoint_re.hpp"
#include "matrixchallenge.hpp"
#include "matrixfailure.hpp"
#include "announcelinkversion.hpp"
#include "playerrename_re.hpp"
#include "playernameupdate.hpp"
#include "getsolochallengerank_re.hpp"
#include "updateenemylist_re.hpp"
#include "playergivepresent_re.hpp"
#include "playeraskforpresent_re.hpp"
#include "announcelinktype.hpp"
#include "tankbattleplayerapply_re.hpp"
#include "tankbattleplayergetrank.hpp"
#include "tankbattleplayergetrank_re.hpp"
#include "uniquedatamodifybroadcast.hpp"
#include "mnfactionbattleapply_re.hpp"
#include "mngetdomaindata_re.hpp"
#include "mngetplayerlastenterinfo.hpp"
#include "mngetplayerlastenterinfo_re.hpp"
#include "mngetfactionbriefinfo.hpp"
#include "mngetfactionbriefinfo_re.hpp"
#include "mngettoplist_re.hpp"
#include "response.hpp"
#include "keepalive.hpp"
#include "acreport.hpp"
#include "acanswer.hpp"
#include "domainlogin.hpp"
#include "keyreestablish.hpp"
#include "selectrole.hpp"
#include "matrixresponse.hpp"
#include "keyexchange.hpp"
#include "rolelist.hpp"
#include "gamedatasend.hpp"
#include "playerbaseinfo.hpp"
#include "createrole.hpp"
#include "deleterole.hpp"
#include "undodeleterole.hpp"
#include "setcustomdata.hpp"
#include "getcustomdata.hpp"
#include "sethelpstates.hpp"
#include "gethelpstates.hpp"
#include "webtraderoleprepost.hpp"
#include "webtraderoleprecancelpost.hpp"
#include "webtraderolegetdetail.hpp"
#include "getplayeridbyname.hpp"
#include "ssogetticket.hpp"
#include "cancelwaitqueue.hpp"
#include "enterworld.hpp"
#include "publicchat.hpp"
#include "privatechat.hpp"
#include "factionchat.hpp"
#include "playerbaseinfocrc.hpp"
#include "setuiconfig.hpp"
#include "getuiconfig.hpp"
#include "getplayerbriefinfo.hpp"
#include "gmgetplayerconsumeinfo.hpp"
#include "collectclientmachineinfo.hpp"
#include "addfriend.hpp"
#include "addfriend_re.hpp"
#include "addfriendremarks.hpp"
#include "getfriends.hpp"
#include "getfriends_re.hpp"
#include "getenemylist.hpp"
#include "setgroupname.hpp"
#include "setgroupname_re.hpp"
#include "setfriendgroup.hpp"
#include "setfriendgroup_re.hpp"
#include "delfriend.hpp"
#include "delfriend_re.hpp"
#include "friendstatus.hpp"
#include "getsavedmsg.hpp"
#include "getsavedmsg_re.hpp"
#include "chatroomcreate.hpp"
#include "chatroominvite.hpp"
#include "chatroominvite_re.hpp"
#include "chatroomjoin.hpp"
#include "chatroomleave.hpp"
#include "chatroomexpel.hpp"
#include "chatroomspeak.hpp"
#include "chatroomlist.hpp"
#include "sendaumail.hpp"
#include "playerrequitefriend.hpp"
#include "tradestart.hpp"
#include "tradeaddgoods.hpp"
#include "traderemovegoods.hpp"
#include "tradesubmit.hpp"
#include "trademoveobj.hpp"
#include "tradeconfirm.hpp"
#include "tradediscard.hpp"
#include "factionoprequest.hpp"
#include "factionacceptjoin.hpp"
#include "getfactionbaseinfo.hpp"
#include "getplayerfactioninfo.hpp"
#include "battlegetmap.hpp"
#include "battlestatus.hpp"
#include "factionlistonline.hpp"
#include "gmonlinenum.hpp"
#include "gmlistonlineuser.hpp"
#include "gmkickoutuser.hpp"
#include "ackickoutuser.hpp"
#include "gmforbidsellpoint.hpp"
#include "gmkickoutrole.hpp"
#include "gmshutup.hpp"
#include "gmshutuprole.hpp"
#include "gmtogglechat.hpp"
#include "gmforbidrole.hpp"
#include "gmrestartserver.hpp"
#include "report2gm.hpp"
#include "complain2gm.hpp"
#include "gmsettimelessautolock.hpp"
#include "checknewmail.hpp"
#include "sellpoint.hpp"
#include "buypoint.hpp"
#include "getselllist.hpp"
#include "findsellpointinfo.hpp"
#include "sellcancel.hpp"
#include "cashlock.hpp"
#include "cashpasswordset.hpp"
#include "autolockset.hpp"
#include "refwithdrawbonus.hpp"
#include "reflistreferrals.hpp"
#include "refgetreferencecode.hpp"
#include "exchangeconsumepoints.hpp"
#include "getrewardlist.hpp"
#include "usercoupon.hpp"
#include "usercouponexchange.hpp"
#include "useraddcash.hpp"
#include "countrybattlemove.hpp"
#include "countrybattlegetmap.hpp"
#include "countrybattlegetplayerlocation.hpp"
#include "countrybattlegetconfig.hpp"
#include "countrybattlegetscore.hpp"
#include "countrybattlepreenter.hpp"
#include "countrybattlereturncapital.hpp"
#include "countrybattlekingassignassault.hpp"
#include "countrybattlekingresetbattlelimit.hpp"
#include "countrybattlegetbattlelimit.hpp"
#include "countrybattlegetkingcommandpoint.hpp"
#include "getcnetserverconfig.hpp"
#include "qpgetactivatedservices.hpp"
#include "qpaddcash.hpp"
#include "reportchat.hpp"
#include "playeraccuse.hpp"
#include "pshopplayerget.hpp"
#include "pshoplist.hpp"
#include "pshoplistitem.hpp"
#include "playerprofilegetprofiledata.hpp"
#include "playerprofilesetprofiledata.hpp"
#include "playerprofilegetmatchresult.hpp"
#include "factionresourcebattlegetmap.hpp"
#include "factionresourcebattlegetrecord.hpp"
#include "mngetfactioninfo.hpp"
#include "domainvalidate.hpp"
#include "domaincmd.hpp"
#include "queryuserprivilege_re.hpp"
#include "queryuserforbid_re.hpp"
#include "kickoutuser.hpp"
#include "statusannounce.hpp"
#include "onlineannounce.hpp"
#include "updateremaintime.hpp"
#include "playerlogin_re.hpp"
#include "playerlogout.hpp"
#include "rolelist_re.hpp"
#include "createrole_re.hpp"
#include "deleterole_re.hpp"
#include "undodeleterole_re.hpp"
#include "accountloginrecord.hpp"
#include "playerbaseinfo_re.hpp"
#include "playerbaseinfocrc_re.hpp"
#include "setcustomdata_re.hpp"
#include "getcustomdata_re.hpp"
#include "getplayeridbyname_re.hpp"
#include "setuiconfig_re.hpp"
#include "getuiconfig_re.hpp"
#include "sethelpstates_re.hpp"
#include "gethelpstates_re.hpp"
#include "getplayerbriefinfo_re.hpp"
#include "gmgetplayerconsumeinfo_re.hpp"
#include "disconnectplayer.hpp"
#include "waitqueuestatenotify.hpp"
#include "cancelwaitqueue_re.hpp"
#include "chatbroadcast.hpp"
#include "chatmulticast.hpp"
#include "chatsinglecast.hpp"
#include "rolestatusannounce.hpp"
#include "addfriendremarks_re.hpp"
#include "getenemylist_re.hpp"
#include "chatroomcreate_re.hpp"
#include "chatroomjoin_re.hpp"
#include "chatroomlist_re.hpp"
#include "friendextlist.hpp"
#include "sendaumail_re.hpp"
#include "tradestart_re.hpp"
#include "tradeaddgoods_re.hpp"
#include "traderemovegoods_re.hpp"
#include "tradesubmit_re.hpp"
#include "trademoveobj_re.hpp"
#include "tradeconfirm_re.hpp"
#include "tradediscard_re.hpp"
#include "tradeend.hpp"
#include "gmrestartserver_re.hpp"
#include "gmonlinenum_re.hpp"
#include "gmlistonlineuser_re.hpp"
#include "gmkickoutuser_re.hpp"
#include "gmforbidsellpoint_re.hpp"
#include "gmkickoutrole_re.hpp"
#include "gmshutup_re.hpp"
#include "gmshutuprole_re.hpp"
#include "gmtogglechat_re.hpp"
#include "gmforbidrole_re.hpp"
#include "report2gm_re.hpp"
#include "complain2gm_re.hpp"
#include "announceforbidinfo.hpp"
#include "setmaxonlinenum_re.hpp"
#include "gmcontrolgame_re.hpp"
#include "gmsettimelessautolock_re.hpp"
#include "acremotecode.hpp"
#include "acquestion.hpp"
#include "playeraccuse_re.hpp"
#include "getmaillist_re.hpp"
#include "getmail_re.hpp"
#include "getmailattachobj_re.hpp"
#include "deletemail_re.hpp"
#include "preservemail_re.hpp"
#include "playersendmail_re.hpp"
#include "auctionopen_re.hpp"
#include "auctionbid_re.hpp"
#include "auctionclose_re.hpp"
#include "auctionlist_re.hpp"
#include "auctionget_re.hpp"
#include "auctiongetitem_re.hpp"
#include "auctionattendlist_re.hpp"
#include "auctionexitbid_re.hpp"
#include "auctionlistupdate_re.hpp"
#include "battlegetmap_re.hpp"
#include "battlestatus_re.hpp"
#include "battlechallenge_re.hpp"
#include "battlechallengemap_re.hpp"
#include "battleenter_re.hpp"
#include "countrybattlemove_re.hpp"
#include "countrybattlesyncplayerlocation.hpp"
#include "countrybattlegetmap_re.hpp"
#include "countrybattlegetconfig_re.hpp"
#include "countrybattlegetscore_re.hpp"
#include "countrybattlepreenternotify.hpp"
#include "countrybattleresult.hpp"
#include "countrybattlesinglebattleresult.hpp"
#include "countrybattlekingassignassault_re.hpp"
#include "countrybattlegetbattlelimit_re.hpp"
#include "countrybattlegetkingcommandpoint_re.hpp"
#include "getcnetserverconfig_re.hpp"
#include "announceserverattribute.hpp"
#include "announcechallengealgo.hpp"
#include "announceauthdversion.hpp"
#include "getselllist_re.hpp"
#include "findsellpointinfo_re.hpp"
#include "announcesellresult.hpp"
#include "sellcancel_re.hpp"
#include "buypoint_re.hpp"
#include "stockcommission_re.hpp"
#include "stockaccount_re.hpp"
#include "stocktransaction_re.hpp"
#include "stockbill_re.hpp"
#include "stockcancel_re.hpp"
#include "cashlock_re.hpp"
#include "cashpasswordset_re.hpp"
#include "domaincmd_re.hpp"
#include "autolockset_re.hpp"
#include "forwardchat.hpp"
#include "disableautolock.hpp"
#include "refwithdrawbonus_re.hpp"
#include "reflistreferrals_re.hpp"
#include "refgetreferencecode_re.hpp"
#include "rewardmaturenotice.hpp"
#include "exchangeconsumepoints_re.hpp"
#include "getrewardlist_re.hpp"
#include "webtradeprepost_re.hpp"
#include "webtradeprecancelpost_re.hpp"
#include "webtradelist_re.hpp"
#include "webtradegetitem_re.hpp"
#include "webtradeattendlist_re.hpp"
#include "webtradegetdetail_re.hpp"
#include "webtradeupdate_re.hpp"
#include "sysauctionlist_re.hpp"
#include "sysauctiongetitem_re.hpp"
#include "sysauctionaccount_re.hpp"
#include "sysauctionbid_re.hpp"
#include "sysauctioncashtransfer_re.hpp"
#include "createfactionfortress_re.hpp"
#include "factionfortresslist_re.hpp"
#include "factionfortresschallenge_re.hpp"
#include "factionfortressbattlelist_re.hpp"
#include "factionfortressget_re.hpp"
#include "switchserverstart.hpp"
#include "usercoupon_re.hpp"
#include "usercouponexchange_re.hpp"
#include "useraddcash_re.hpp"
#include "ssogetticket_re.hpp"
#include "qpannouncediscount.hpp"
#include "qpgetactivatedservices_re.hpp"
#include "qpaddcash_re.hpp"
#include "playerchangeds_re.hpp"
#include "changeds_re.hpp"
#include "kickoutuser2.hpp"
#include "kegetstatus_re.hpp"
#include "kecandidateapply_re.hpp"
#include "kevoting_re.hpp"
#include "pshopcreate_re.hpp"
#include "pshopbuy_re.hpp"
#include "pshopsell_re.hpp"
#include "pshopcancelgoods_re.hpp"
#include "pshopplayerbuy_re.hpp"
#include "pshopplayersell_re.hpp"
#include "pshopsettype_re.hpp"
#include "pshopactive_re.hpp"
#include "pshopmanagefund_re.hpp"
#include "pshopdrawitem_re.hpp"
#include "pshopcleargoods_re.hpp"
#include "pshopselfget_re.hpp"
#include "pshopplayerget_re.hpp"
#include "pshoplist_re.hpp"
#include "pshoplistitem_re.hpp"
#include "playerprofilegetprofiledata_re.hpp"
#include "playerprofilegetmatchresult_re.hpp"
#include "autoteamsetgoal_re.hpp"
#include "autoteamplayerleave.hpp"
#include "factionresourcebattleplayerqueryresult.hpp"
#include "factionresourcebattlegetmap_re.hpp"
#include "factionresourcebattlegetrecord_re.hpp"
#include "factionresourcebattlenotifyplayerevent.hpp"
#include "mngetfactioninfo_re.hpp"
#include "announceproviderid.hpp"
#include "s2cgamedatasend.hpp"
#include "s2cmulticast.hpp"
#include "s2cbroadcast.hpp"
#include "playerheartbeat.hpp"
#include "switchservercancel.hpp"
#include "switchserversuccess.hpp"
#include "switchservertimeout.hpp"
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
#include "factionallianceapply_re.hpp"
#include "factionalliancereply_re.hpp"
#include "factionhostileapply_re.hpp"
#include "factionhostilereply_re.hpp"
#include "factionremoverelationapply_re.hpp"
#include "factionremoverelationreply_re.hpp"
#include "factionlistrelation_re.hpp"
#include "factionrelationrecvapply.hpp"
#include "factionrelationrecvreply.hpp"
#include "factiondelayexpelannounce.hpp"
#include "getfactionbaseinfo_re.hpp"
#include "getplayerfactioninfo_re.hpp"
#include "factionlistonline_re.hpp"
#include "factionrenameannounce.hpp"

namespace GNET
{

#ifndef WIN32
static ProtocolBinder __ProtocolBinder_stub(PROTOCOL_BINDER, 65536);
#endif
#ifndef WIN32
static CompressBinder __CompressBinder_stub(PROTOCOL_COMPRESSBINDER, 65536);
#endif
static AddFriendRqst __stub_AddFriendRqst (RPC_ADDFRIENDRQST, new AddFriendRqstArg, new AddFriendRqstRes);
static TradeStartRqst __stub_TradeStartRqst (RPC_TRADESTARTRQST, new TradeStartRqstArg, new TradeStartRqstRes);
static FactionInviteJoin __stub_FactionInviteJoin (RPC_FACTIONINVITEJOIN, new FactionInviteArg, new FactionInviteRes);
static PlayerPositionResetRqst __stub_PlayerPositionResetRqst (RPC_PLAYERPOSITIONRESETRQST, new PlayerPositionResetRqstArg, new PlayerPositionResetRqstRes);
static UserLogin __stub_UserLogin (RPC_USERLOGIN, new UserLoginArg, new UserLoginRes);
static UserLogin2 __stub_UserLogin2 (RPC_USERLOGIN2, new UserLogin2Arg, new UserLogin2Res);
static GQueryPasswd __stub_GQueryPasswd (RPC_GQUERYPASSWD, new GQueryPasswdArg, new GQueryPasswdRes);
static DBSellPoint __stub_DBSellPoint (RPC_DBSELLPOINT, new SellPointArg, new SellPointRes);
static DBBuyPoint __stub_DBBuyPoint (RPC_DBBUYPOINT, new DBBuyPointArg, new DBBuyPointRes);
static MatrixPasswd __stub_MatrixPasswd (RPC_MATRIXPASSWD, new MatrixPasswdArg, new MatrixPasswdRes);
static MatrixPasswd2 __stub_MatrixPasswd2 (RPC_MATRIXPASSWD2, new MatrixPasswdArg, new MatrixPasswd2Res);
static MatrixToken __stub_MatrixToken (RPC_MATRIXTOKEN, new MatrixTokenArg, new MatrixTokenRes);
static PlayerIdentityMatch __stub_PlayerIdentityMatch (RPC_PLAYERIDENTITYMATCH, new PlayerIdentityMatchArg, new PlayerIdentityMatchRes);
static AuctionExitBid __stub_AuctionExitBid((void*)0);
static ErrorInfo __stub_ErrorInfo((void*)0);
static Challenge __stub_Challenge((void*)0);
static SelectRole_Re __stub_SelectRole_Re((void*)0);
static PlayerLogin __stub_PlayerLogin((void*)0);
static PlayerOffline __stub_PlayerOffline((void*)0);
static PlayerStatusSync __stub_PlayerStatusSync((void*)0);
static C2SGamedataSend __stub_C2SGamedataSend((void*)0);
static ChatMessage __stub_ChatMessage((void*)0);
static WorldChat __stub_WorldChat((void*)0);
static AnnounceNewMail __stub_AnnounceNewMail((void*)0);
static ACWhoAmI __stub_ACWhoAmI((void*)0);
static ACProtoStat __stub_ACProtoStat((void*)0);
static ReportIP __stub_ReportIP((void*)0);
static SellPoint_Re __stub_SellPoint_Re((void*)0);
static MatrixChallenge __stub_MatrixChallenge((void*)0);
static MatrixFailure __stub_MatrixFailure((void*)0);
static AnnounceLinkVersion __stub_AnnounceLinkVersion((void*)0);
static PlayerRename_Re __stub_PlayerRename_Re((void*)0);
static PlayerNameUpdate __stub_PlayerNameUpdate((void*)0);
static GetSoloChallengeRank_Re __stub_GetSoloChallengeRank_Re((void*)0);
static UpdateEnemyList_Re __stub_UpdateEnemyList_Re((void*)0);
static PlayerGivePresent_Re __stub_PlayerGivePresent_Re((void*)0);
static PlayerAskForPresent_Re __stub_PlayerAskForPresent_Re((void*)0);
static AnnounceLinkType __stub_AnnounceLinkType((void*)0);
static TankBattlePlayerApply_Re __stub_TankBattlePlayerApply_Re((void*)0);
static TankBattlePlayerGetRank __stub_TankBattlePlayerGetRank((void*)0);
static TankBattlePlayerGetRank_Re __stub_TankBattlePlayerGetRank_Re((void*)0);
static UniqueDataModifyBroadcast __stub_UniqueDataModifyBroadcast((void*)0);
static MNFactionBattleApply_Re __stub_MNFactionBattleApply_Re((void*)0);
static MNGetDomainData_Re __stub_MNGetDomainData_Re((void*)0);
static MNGetPlayerLastEnterInfo __stub_MNGetPlayerLastEnterInfo((void*)0);
static MNGetPlayerLastEnterInfo_Re __stub_MNGetPlayerLastEnterInfo_Re((void*)0);
static MNGetFactionBriefInfo __stub_MNGetFactionBriefInfo((void*)0);
static MNGetFactionBriefInfo_Re __stub_MNGetFactionBriefInfo_Re((void*)0);
static MNGetTopList_Re __stub_MNGetTopList_Re((void*)0);
static Response __stub_Response((void*)0);
static KeepAlive __stub_KeepAlive((void*)0);
static ACReport __stub_ACReport((void*)0);
static ACAnswer __stub_ACAnswer((void*)0);
static DomainLogin __stub_DomainLogin((void*)0);
static KeyReestablish __stub_KeyReestablish((void*)0);
static SelectRole __stub_SelectRole((void*)0);
static MatrixResponse __stub_MatrixResponse((void*)0);
static KeyExchange __stub_KeyExchange((void*)0);
static RoleList __stub_RoleList((void*)0);
static GamedataSend __stub_GamedataSend((void*)0);
static PlayerBaseInfo __stub_PlayerBaseInfo((void*)0);
static CreateRole __stub_CreateRole((void*)0);
static DeleteRole __stub_DeleteRole((void*)0);
static UndoDeleteRole __stub_UndoDeleteRole((void*)0);
static SetCustomData __stub_SetCustomData((void*)0);
static GetCustomData __stub_GetCustomData((void*)0);
static SetHelpStates __stub_SetHelpStates((void*)0);
static GetHelpStates __stub_GetHelpStates((void*)0);
static WebTradeRolePrePost __stub_WebTradeRolePrePost((void*)0);
static WebTradeRolePreCancelPost __stub_WebTradeRolePreCancelPost((void*)0);
static WebTradeRoleGetDetail __stub_WebTradeRoleGetDetail((void*)0);
static GetPlayerIDByName __stub_GetPlayerIDByName((void*)0);
static SSOGetTicket __stub_SSOGetTicket((void*)0);
static CancelWaitQueue __stub_CancelWaitQueue((void*)0);
static EnterWorld __stub_EnterWorld((void*)0);
static PublicChat __stub_PublicChat((void*)0);
static PrivateChat __stub_PrivateChat((void*)0);
static FactionChat __stub_FactionChat((void*)0);
static PlayerBaseInfoCRC __stub_PlayerBaseInfoCRC((void*)0);
static SetUIConfig __stub_SetUIConfig((void*)0);
static GetUIConfig __stub_GetUIConfig((void*)0);
static GetPlayerBriefInfo __stub_GetPlayerBriefInfo((void*)0);
static GMGetPlayerConsumeInfo __stub_GMGetPlayerConsumeInfo((void*)0);
static CollectClientMachineInfo __stub_CollectClientMachineInfo((void*)0);
static AddFriend __stub_AddFriend((void*)0);
static AddFriend_Re __stub_AddFriend_Re((void*)0);
static AddFriendRemarks __stub_AddFriendRemarks((void*)0);
static GetFriends __stub_GetFriends((void*)0);
static GetFriends_Re __stub_GetFriends_Re((void*)0);
static GetEnemyList __stub_GetEnemyList((void*)0);
static SetGroupName __stub_SetGroupName((void*)0);
static SetGroupName_Re __stub_SetGroupName_Re((void*)0);
static SetFriendGroup __stub_SetFriendGroup((void*)0);
static SetFriendGroup_Re __stub_SetFriendGroup_Re((void*)0);
static DelFriend __stub_DelFriend((void*)0);
static DelFriend_Re __stub_DelFriend_Re((void*)0);
static FriendStatus __stub_FriendStatus((void*)0);
static GetSavedMsg __stub_GetSavedMsg((void*)0);
static GetSavedMsg_Re __stub_GetSavedMsg_Re((void*)0);
static ChatRoomCreate __stub_ChatRoomCreate((void*)0);
static ChatRoomInvite __stub_ChatRoomInvite((void*)0);
static ChatRoomInvite_Re __stub_ChatRoomInvite_Re((void*)0);
static ChatRoomJoin __stub_ChatRoomJoin((void*)0);
static ChatRoomLeave __stub_ChatRoomLeave((void*)0);
static ChatRoomExpel __stub_ChatRoomExpel((void*)0);
static ChatRoomSpeak __stub_ChatRoomSpeak((void*)0);
static ChatRoomList __stub_ChatRoomList((void*)0);
static SendAUMail __stub_SendAUMail((void*)0);
static PlayerRequiteFriend __stub_PlayerRequiteFriend((void*)0);
static TradeStart __stub_TradeStart((void*)0);
static TradeAddGoods __stub_TradeAddGoods((void*)0);
static TradeRemoveGoods __stub_TradeRemoveGoods((void*)0);
static TradeSubmit __stub_TradeSubmit((void*)0);
static TradeMoveObj __stub_TradeMoveObj((void*)0);
static TradeConfirm __stub_TradeConfirm((void*)0);
static TradeDiscard __stub_TradeDiscard((void*)0);
static FactionOPRequest __stub_FactionOPRequest((void*)0);
static FactionAcceptJoin __stub_FactionAcceptJoin((void*)0);
static GetFactionBaseInfo __stub_GetFactionBaseInfo((void*)0);
static GetPlayerFactionInfo __stub_GetPlayerFactionInfo((void*)0);
static BattleGetMap __stub_BattleGetMap((void*)0);
static BattleStatus __stub_BattleStatus((void*)0);
static FactionListOnline __stub_FactionListOnline((void*)0);
static GMOnlineNum __stub_GMOnlineNum((void*)0);
static GMListOnlineUser __stub_GMListOnlineUser((void*)0);
static GMKickoutUser __stub_GMKickoutUser((void*)0);
static ACKickoutUser __stub_ACKickoutUser((void*)0);
static GMForbidSellPoint __stub_GMForbidSellPoint((void*)0);
static GMKickoutRole __stub_GMKickoutRole((void*)0);
static GMShutup __stub_GMShutup((void*)0);
static GMShutupRole __stub_GMShutupRole((void*)0);
static GMToggleChat __stub_GMToggleChat((void*)0);
static GMForbidRole __stub_GMForbidRole((void*)0);
static GMRestartServer __stub_GMRestartServer((void*)0);
static Report2GM __stub_Report2GM((void*)0);
static Complain2GM __stub_Complain2GM((void*)0);
static GMSetTimelessAutoLock __stub_GMSetTimelessAutoLock((void*)0);
static CheckNewMail __stub_CheckNewMail((void*)0);
static SellPoint __stub_SellPoint((void*)0);
static BuyPoint __stub_BuyPoint((void*)0);
static GetSellList __stub_GetSellList((void*)0);
static FindSellPointInfo __stub_FindSellPointInfo((void*)0);
static SellCancel __stub_SellCancel((void*)0);
static CashLock __stub_CashLock((void*)0);
static CashPasswordSet __stub_CashPasswordSet((void*)0);
static AutolockSet __stub_AutolockSet((void*)0);
static RefWithdrawBonus __stub_RefWithdrawBonus((void*)0);
static RefListReferrals __stub_RefListReferrals((void*)0);
static RefGetReferenceCode __stub_RefGetReferenceCode((void*)0);
static ExchangeConsumePoints __stub_ExchangeConsumePoints((void*)0);
static GetRewardList __stub_GetRewardList((void*)0);
static UserCoupon __stub_UserCoupon((void*)0);
static UserCouponExchange __stub_UserCouponExchange((void*)0);
static UserAddCash __stub_UserAddCash((void*)0);
static CountryBattleMove __stub_CountryBattleMove((void*)0);
static CountryBattleGetMap __stub_CountryBattleGetMap((void*)0);
static CountryBattleGetPlayerLocation __stub_CountryBattleGetPlayerLocation((void*)0);
static CountryBattleGetConfig __stub_CountryBattleGetConfig((void*)0);
static CountryBattleGetScore __stub_CountryBattleGetScore((void*)0);
static CountryBattlePreEnter __stub_CountryBattlePreEnter((void*)0);
static CountryBattleReturnCapital __stub_CountryBattleReturnCapital((void*)0);
static CountryBattleKingAssignAssault __stub_CountryBattleKingAssignAssault((void*)0);
static CountryBattleKingResetBattleLimit __stub_CountryBattleKingResetBattleLimit((void*)0);
static CountryBattleGetBattleLimit __stub_CountryBattleGetBattleLimit((void*)0);
static CountryBattleGetKingCommandPoint __stub_CountryBattleGetKingCommandPoint((void*)0);
static GetCNetServerConfig __stub_GetCNetServerConfig((void*)0);
static QPGetActivatedServices __stub_QPGetActivatedServices((void*)0);
static QPAddCash __stub_QPAddCash((void*)0);
static ReportChat __stub_ReportChat((void*)0);
static PlayerAccuse __stub_PlayerAccuse((void*)0);
static PShopPlayerGet __stub_PShopPlayerGet((void*)0);
static PShopList __stub_PShopList((void*)0);
static PShopListItem __stub_PShopListItem((void*)0);
static PlayerProfileGetProfileData __stub_PlayerProfileGetProfileData((void*)0);
static PlayerProfileSetProfileData __stub_PlayerProfileSetProfileData((void*)0);
static PlayerProfileGetMatchResult __stub_PlayerProfileGetMatchResult((void*)0);
static FactionResourceBattleGetMap __stub_FactionResourceBattleGetMap((void*)0);
static FactionResourceBattleGetRecord __stub_FactionResourceBattleGetRecord((void*)0);
static MNGetFactionInfo __stub_MNGetFactionInfo((void*)0);
static DomainValidate __stub_DomainValidate((void*)0);
static DomainCmd __stub_DomainCmd((void*)0);
static QueryUserPrivilege_Re __stub_QueryUserPrivilege_Re((void*)0);
static QueryUserForbid_Re __stub_QueryUserForbid_Re((void*)0);
static KickoutUser __stub_KickoutUser((void*)0);
static StatusAnnounce __stub_StatusAnnounce((void*)0);
static OnlineAnnounce __stub_OnlineAnnounce((void*)0);
static UpdateRemainTime __stub_UpdateRemainTime((void*)0);
static PlayerLogin_Re __stub_PlayerLogin_Re((void*)0);
static PlayerLogout __stub_PlayerLogout((void*)0);
static RoleList_Re __stub_RoleList_Re((void*)0);
static CreateRole_Re __stub_CreateRole_Re((void*)0);
static DeleteRole_Re __stub_DeleteRole_Re((void*)0);
static UndoDeleteRole_Re __stub_UndoDeleteRole_Re((void*)0);
static AccountLoginRecord __stub_AccountLoginRecord((void*)0);
static PlayerBaseInfo_Re __stub_PlayerBaseInfo_Re((void*)0);
static PlayerBaseInfoCRC_Re __stub_PlayerBaseInfoCRC_Re((void*)0);
static SetCustomData_Re __stub_SetCustomData_Re((void*)0);
static GetCustomData_Re __stub_GetCustomData_Re((void*)0);
static GetPlayerIDByName_Re __stub_GetPlayerIDByName_Re((void*)0);
static SetUIConfig_Re __stub_SetUIConfig_Re((void*)0);
static GetUIConfig_Re __stub_GetUIConfig_Re((void*)0);
static SetHelpStates_Re __stub_SetHelpStates_Re((void*)0);
static GetHelpStates_Re __stub_GetHelpStates_Re((void*)0);
static GetPlayerBriefInfo_Re __stub_GetPlayerBriefInfo_Re((void*)0);
static GMGetPlayerConsumeInfo_Re __stub_GMGetPlayerConsumeInfo_Re((void*)0);
static DisconnectPlayer __stub_DisconnectPlayer((void*)0);
static WaitQueueStateNotify __stub_WaitQueueStateNotify((void*)0);
static CancelWaitQueue_Re __stub_CancelWaitQueue_Re((void*)0);
static ChatBroadCast __stub_ChatBroadCast((void*)0);
static ChatMultiCast __stub_ChatMultiCast((void*)0);
static ChatSingleCast __stub_ChatSingleCast((void*)0);
static RoleStatusAnnounce __stub_RoleStatusAnnounce((void*)0);
static AddFriendRemarks_Re __stub_AddFriendRemarks_Re((void*)0);
static GetEnemyList_Re __stub_GetEnemyList_Re((void*)0);
static ChatRoomCreate_Re __stub_ChatRoomCreate_Re((void*)0);
static ChatRoomJoin_Re __stub_ChatRoomJoin_Re((void*)0);
static ChatRoomList_Re __stub_ChatRoomList_Re((void*)0);
static FriendExtList __stub_FriendExtList((void*)0);
static SendAUMail_Re __stub_SendAUMail_Re((void*)0);
static TradeStart_Re __stub_TradeStart_Re((void*)0);
static TradeAddGoods_Re __stub_TradeAddGoods_Re((void*)0);
static TradeRemoveGoods_Re __stub_TradeRemoveGoods_Re((void*)0);
static TradeSubmit_Re __stub_TradeSubmit_Re((void*)0);
static TradeMoveObj_Re __stub_TradeMoveObj_Re((void*)0);
static TradeConfirm_Re __stub_TradeConfirm_Re((void*)0);
static TradeDiscard_Re __stub_TradeDiscard_Re((void*)0);
static TradeEnd __stub_TradeEnd((void*)0);
static GMRestartServer_Re __stub_GMRestartServer_Re((void*)0);
static GMOnlineNum_Re __stub_GMOnlineNum_Re((void*)0);
static GMListOnlineUser_Re __stub_GMListOnlineUser_Re((void*)0);
static GMKickoutUser_Re __stub_GMKickoutUser_Re((void*)0);
static GMForbidSellPoint_Re __stub_GMForbidSellPoint_Re((void*)0);
static GMKickoutRole_Re __stub_GMKickoutRole_Re((void*)0);
static GMShutup_Re __stub_GMShutup_Re((void*)0);
static GMShutupRole_Re __stub_GMShutupRole_Re((void*)0);
static GMToggleChat_Re __stub_GMToggleChat_Re((void*)0);
static GMForbidRole_Re __stub_GMForbidRole_Re((void*)0);
static Report2GM_Re __stub_Report2GM_Re((void*)0);
static Complain2GM_Re __stub_Complain2GM_Re((void*)0);
static AnnounceForbidInfo __stub_AnnounceForbidInfo((void*)0);
static SetMaxOnlineNum_Re __stub_SetMaxOnlineNum_Re((void*)0);
static GMControlGame_Re __stub_GMControlGame_Re((void*)0);
static GMSetTimelessAutoLock_Re __stub_GMSetTimelessAutoLock_Re((void*)0);
static ACRemoteCode __stub_ACRemoteCode((void*)0);
static ACQuestion __stub_ACQuestion((void*)0);
static PlayerAccuse_Re __stub_PlayerAccuse_Re((void*)0);
static GetMailList_Re __stub_GetMailList_Re((void*)0);
static GetMail_Re __stub_GetMail_Re((void*)0);
static GetMailAttachObj_Re __stub_GetMailAttachObj_Re((void*)0);
static DeleteMail_Re __stub_DeleteMail_Re((void*)0);
static PreserveMail_Re __stub_PreserveMail_Re((void*)0);
static PlayerSendMail_Re __stub_PlayerSendMail_Re((void*)0);
static AuctionOpen_Re __stub_AuctionOpen_Re((void*)0);
static AuctionBid_Re __stub_AuctionBid_Re((void*)0);
static AuctionClose_Re __stub_AuctionClose_Re((void*)0);
static AuctionList_Re __stub_AuctionList_Re((void*)0);
static AuctionGet_Re __stub_AuctionGet_Re((void*)0);
static AuctionGetItem_Re __stub_AuctionGetItem_Re((void*)0);
static AuctionAttendList_Re __stub_AuctionAttendList_Re((void*)0);
static AuctionExitBid_Re __stub_AuctionExitBid_Re((void*)0);
static AuctionListUpdate_Re __stub_AuctionListUpdate_Re((void*)0);
static BattleGetMap_Re __stub_BattleGetMap_Re((void*)0);
static BattleStatus_Re __stub_BattleStatus_Re((void*)0);
static BattleChallenge_Re __stub_BattleChallenge_Re((void*)0);
static BattleChallengeMap_Re __stub_BattleChallengeMap_Re((void*)0);
static BattleEnter_Re __stub_BattleEnter_Re((void*)0);
static CountryBattleMove_Re __stub_CountryBattleMove_Re((void*)0);
static CountryBattleSyncPlayerLocation __stub_CountryBattleSyncPlayerLocation((void*)0);
static CountryBattleGetMap_Re __stub_CountryBattleGetMap_Re((void*)0);
static CountryBattleGetConfig_Re __stub_CountryBattleGetConfig_Re((void*)0);
static CountryBattleGetScore_Re __stub_CountryBattleGetScore_Re((void*)0);
static CountryBattlePreEnterNotify __stub_CountryBattlePreEnterNotify((void*)0);
static CountryBattleResult __stub_CountryBattleResult((void*)0);
static CountryBattleSingleBattleResult __stub_CountryBattleSingleBattleResult((void*)0);
static CountryBattleKingAssignAssault_Re __stub_CountryBattleKingAssignAssault_Re((void*)0);
static CountryBattleGetBattleLimit_Re __stub_CountryBattleGetBattleLimit_Re((void*)0);
static CountryBattleGetKingCommandPoint_Re __stub_CountryBattleGetKingCommandPoint_Re((void*)0);
static GetCNetServerConfig_Re __stub_GetCNetServerConfig_Re((void*)0);
static AnnounceServerAttribute __stub_AnnounceServerAttribute((void*)0);
static AnnounceChallengeAlgo __stub_AnnounceChallengeAlgo((void*)0);
static AnnounceAuthdVersion __stub_AnnounceAuthdVersion((void*)0);
static GetSellList_Re __stub_GetSellList_Re((void*)0);
static FindSellPointInfo_Re __stub_FindSellPointInfo_Re((void*)0);
static AnnounceSellResult __stub_AnnounceSellResult((void*)0);
static SellCancel_Re __stub_SellCancel_Re((void*)0);
static BuyPoint_Re __stub_BuyPoint_Re((void*)0);
static StockCommission_Re __stub_StockCommission_Re((void*)0);
static StockAccount_Re __stub_StockAccount_Re((void*)0);
static StockTransaction_Re __stub_StockTransaction_Re((void*)0);
static StockBill_Re __stub_StockBill_Re((void*)0);
static StockCancel_Re __stub_StockCancel_Re((void*)0);
static CashLock_Re __stub_CashLock_Re((void*)0);
static CashPasswordSet_Re __stub_CashPasswordSet_Re((void*)0);
static DomainCmd_Re __stub_DomainCmd_Re((void*)0);
static AutolockSet_Re __stub_AutolockSet_Re((void*)0);
static ForwardChat __stub_ForwardChat((void*)0);
static DisableAutolock __stub_DisableAutolock((void*)0);
static RefWithdrawBonus_Re __stub_RefWithdrawBonus_Re((void*)0);
static RefListReferrals_Re __stub_RefListReferrals_Re((void*)0);
static RefGetReferenceCode_Re __stub_RefGetReferenceCode_Re((void*)0);
static RewardMatureNotice __stub_RewardMatureNotice((void*)0);
static ExchangeConsumePoints_Re __stub_ExchangeConsumePoints_Re((void*)0);
static GetRewardList_Re __stub_GetRewardList_Re((void*)0);
static WebTradePrePost_Re __stub_WebTradePrePost_Re((void*)0);
static WebTradePreCancelPost_Re __stub_WebTradePreCancelPost_Re((void*)0);
static WebTradeList_Re __stub_WebTradeList_Re((void*)0);
static WebTradeGetItem_Re __stub_WebTradeGetItem_Re((void*)0);
static WebTradeAttendList_Re __stub_WebTradeAttendList_Re((void*)0);
static WebTradeGetDetail_Re __stub_WebTradeGetDetail_Re((void*)0);
static WebTradeUpdate_Re __stub_WebTradeUpdate_Re((void*)0);
static SysAuctionList_Re __stub_SysAuctionList_Re((void*)0);
static SysAuctionGetItem_Re __stub_SysAuctionGetItem_Re((void*)0);
static SysAuctionAccount_Re __stub_SysAuctionAccount_Re((void*)0);
static SysAuctionBid_Re __stub_SysAuctionBid_Re((void*)0);
static SysAuctionCashTransfer_Re __stub_SysAuctionCashTransfer_Re((void*)0);
static CreateFactionFortress_Re __stub_CreateFactionFortress_Re((void*)0);
static FactionFortressList_Re __stub_FactionFortressList_Re((void*)0);
static FactionFortressChallenge_Re __stub_FactionFortressChallenge_Re((void*)0);
static FactionFortressBattleList_Re __stub_FactionFortressBattleList_Re((void*)0);
static FactionFortressGet_Re __stub_FactionFortressGet_Re((void*)0);
static SwitchServerStart __stub_SwitchServerStart((void*)0);
static UserCoupon_Re __stub_UserCoupon_Re((void*)0);
static UserCouponExchange_Re __stub_UserCouponExchange_Re((void*)0);
static UserAddCash_Re __stub_UserAddCash_Re((void*)0);
static SSOGetTicket_Re __stub_SSOGetTicket_Re((void*)0);
static QPAnnounceDiscount __stub_QPAnnounceDiscount((void*)0);
static QPGetActivatedServices_Re __stub_QPGetActivatedServices_Re((void*)0);
static QPAddCash_Re __stub_QPAddCash_Re((void*)0);
static PlayerChangeDS_Re __stub_PlayerChangeDS_Re((void*)0);
static ChangeDS_Re __stub_ChangeDS_Re((void*)0);
static KickoutUser2 __stub_KickoutUser2((void*)0);
static KEGetStatus_Re __stub_KEGetStatus_Re((void*)0);
static KECandidateApply_Re __stub_KECandidateApply_Re((void*)0);
static KEVoting_Re __stub_KEVoting_Re((void*)0);
static PShopCreate_Re __stub_PShopCreate_Re((void*)0);
static PShopBuy_Re __stub_PShopBuy_Re((void*)0);
static PShopSell_Re __stub_PShopSell_Re((void*)0);
static PShopCancelGoods_Re __stub_PShopCancelGoods_Re((void*)0);
static PShopPlayerBuy_Re __stub_PShopPlayerBuy_Re((void*)0);
static PShopPlayerSell_Re __stub_PShopPlayerSell_Re((void*)0);
static PShopSetType_Re __stub_PShopSetType_Re((void*)0);
static PShopActive_Re __stub_PShopActive_Re((void*)0);
static PShopManageFund_Re __stub_PShopManageFund_Re((void*)0);
static PShopDrawItem_Re __stub_PShopDrawItem_Re((void*)0);
static PShopClearGoods_Re __stub_PShopClearGoods_Re((void*)0);
static PShopSelfGet_Re __stub_PShopSelfGet_Re((void*)0);
static PShopPlayerGet_Re __stub_PShopPlayerGet_Re((void*)0);
static PShopList_Re __stub_PShopList_Re((void*)0);
static PShopListItem_Re __stub_PShopListItem_Re((void*)0);
static PlayerProfileGetProfileData_Re __stub_PlayerProfileGetProfileData_Re((void*)0);
static PlayerProfileGetMatchResult_Re __stub_PlayerProfileGetMatchResult_Re((void*)0);
static AutoTeamSetGoal_Re __stub_AutoTeamSetGoal_Re((void*)0);
static AutoTeamPlayerLeave __stub_AutoTeamPlayerLeave((void*)0);
static FactionResourceBattlePlayerQueryResult __stub_FactionResourceBattlePlayerQueryResult((void*)0);
static FactionResourceBattleGetMap_Re __stub_FactionResourceBattleGetMap_Re((void*)0);
static FactionResourceBattleGetRecord_Re __stub_FactionResourceBattleGetRecord_Re((void*)0);
static FactionResourceBattleNotifyPlayerEvent __stub_FactionResourceBattleNotifyPlayerEvent((void*)0);
static MNGetFactionInfo_Re __stub_MNGetFactionInfo_Re((void*)0);
static AnnounceProviderID __stub_AnnounceProviderID((void*)0);
static S2CGamedataSend __stub_S2CGamedataSend((void*)0);
static S2CMulticast __stub_S2CMulticast((void*)0);
static S2CBroadcast __stub_S2CBroadcast((void*)0);
static PlayerHeartBeat __stub_PlayerHeartBeat((void*)0);
static SwitchServerCancel __stub_SwitchServerCancel((void*)0);
static SwitchServerSuccess __stub_SwitchServerSuccess((void*)0);
static SwitchServerTimeout __stub_SwitchServerTimeout((void*)0);
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
static FactionAllianceApply_Re __stub_FactionAllianceApply_Re((void*)0);
static FactionAllianceReply_Re __stub_FactionAllianceReply_Re((void*)0);
static FactionHostileApply_Re __stub_FactionHostileApply_Re((void*)0);
static FactionHostileReply_Re __stub_FactionHostileReply_Re((void*)0);
static FactionRemoveRelationApply_Re __stub_FactionRemoveRelationApply_Re((void*)0);
static FactionRemoveRelationReply_Re __stub_FactionRemoveRelationReply_Re((void*)0);
static FactionListRelation_Re __stub_FactionListRelation_Re((void*)0);
static FactionRelationRecvApply __stub_FactionRelationRecvApply((void*)0);
static FactionRelationRecvReply __stub_FactionRelationRecvReply((void*)0);
static FactionDelayExpelAnnounce __stub_FactionDelayExpelAnnounce((void*)0);
static GetFactionBaseInfo_Re __stub_GetFactionBaseInfo_Re((void*)0);
static GetPlayerFactionInfo_Re __stub_GetPlayerFactionInfo_Re((void*)0);
static FactionListOnline_Re __stub_FactionListOnline_Re((void*)0);
static FactionRenameAnnounce __stub_FactionRenameAnnounce((void*)0);

};
