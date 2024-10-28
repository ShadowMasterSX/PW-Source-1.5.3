#ifdef WIN32
#include <winsock2.h>
#include "gncompress.h"
#else
#include "binder.h"
#endif
#include "putspouse.hrp"
#include "dbplayerchangegender.hrp"
#include "dbsellpoint.hrp"
#include "dbbuypoint.hrp"
#include "battleend.hrp"
#include "sysauctionprepareitem.hrp"
#include "getfactionfortress.hrp"
#include "putfactionfortress.hrp"
#include "playerkickout_re.hpp"
#include "playerlogin_re.hpp"
#include "playeroffline_re.hpp"
#include "playerlogout.hpp"
#include "playerinfoupdate.hpp"
#include "playerteamop.hpp"
#include "playerteammemberop.hpp"
#include "queryplayerstatus.hpp"
#include "gettaskdata.hpp"
#include "settaskdata.hpp"
#include "s2cgamedatasend.hpp"
#include "s2cmulticast.hpp"
#include "s2cbroadcast.hpp"
#include "chatmulticast.hpp"
#include "chatbroadcast.hpp"
#include "chatsinglecast.hpp"
#include "keepalive.hpp"
#include "playerheartbeat.hpp"
#include "disconnectplayer.hpp"
#include "gtradestart_re.hpp"
#include "gtradediscard.hpp"
#include "setchatemotion.hpp"
#include "facemodify.hpp"
#include "facemodifycancel.hpp"
#include "synmutadata.hpp"
#include "switchserversuccess.hpp"
#include "switchservertimeout.hpp"
#include "factionoprequest.hpp"
#include "factionbeginsync_re.hpp"
#include "playerfactioninfo.hpp"
#include "getplayerfactionrelation.hpp"
#include "battlechallenge.hpp"
#include "battlechallengemap.hpp"
#include "battleenter.hpp"
#include "battleserverregister.hpp"
#include "battlestart_re.hpp"
#include "debugcommand.hpp"
#include "gmrestartserver_re.hpp"
#include "gmcontrolgame_re.hpp"
#include "getmaillist.hpp"
#include "getmail.hpp"
#include "getmailattachobj.hpp"
#include "deletemail.hpp"
#include "preservemail.hpp"
#include "playersendmail.hpp"
#include "playersendmassmail.hpp"
#include "auctionopen.hpp"
#include "auctionbid.hpp"
#include "sendauctionbid.hpp"
#include "sendbattlechallenge.hpp"
#include "auctionlist.hpp"
#include "auctionclose.hpp"
#include "auctionget.hpp"
#include "auctiongetitem.hpp"
#include "auctionattendlist.hpp"
#include "auctionexitbid.hpp"
#include "auctionlistupdate.hpp"
#include "queryrewardtype.hpp"
#include "queryrewardtype_re.hpp"
#include "actriggerquestion.hpp"
#include "querygameserverattr.hpp"
#include "sellpoint.hpp"
#include "buypoint.hpp"
#include "getselllist.hpp"
#include "sellcancel.hpp"
#include "stockcommission.hpp"
#include "stockaccount.hpp"
#include "stocktransaction.hpp"
#include "stockbill.hpp"
#include "stockcancel.hpp"
#include "sendrefcashused.hpp"
#include "sendtaskreward.hpp"
#include "refgetreferencecode_re.hpp"
#include "billingbalancesa.hpp"
#include "billingconfirm.hpp"
#include "billingcancel.hpp"
#include "webtradeprepost.hpp"
#include "webtradeprecancelpost.hpp"
#include "webtradelist.hpp"
#include "webtradegetitem.hpp"
#include "webtradeattendlist.hpp"
#include "webtradegetdetail.hpp"
#include "webtradeupdate.hpp"
#include "sysauctionlist.hpp"
#include "sysauctiongetitem.hpp"
#include "sysauctionaccount.hpp"
#include "sysauctionbid.hpp"
#include "sysauctioncashtransfer.hpp"
#include "createfactionfortress.hpp"
#include "factionserverregister.hpp"
#include "notifyfactionfortressstate.hpp"
#include "factionfortressenter.hpp"
#include "factionfortresslist.hpp"
#include "factionfortresschallenge.hpp"
#include "factionfortressbattlelist.hpp"
#include "factionfortressget.hpp"
#include "snsrolebriefupdate.hpp"
#include "notifyplayerjoinorleaveforce.hpp"
#include "increaseforceactivity.hpp"
#include "countrybattleapply.hpp"
#include "countrybattlejoinnotice.hpp"
#include "countrybattleleavenotice.hpp"
#include "countrybattleonlinenotice.hpp"
#include "countrybattleofflinenotice.hpp"
#include "countrybattleentermapnotice.hpp"
#include "countrybattleserverregister.hpp"
#include "countrybattlestart_re.hpp"
#include "countrybattleend.hpp"
#include "playerrename.hpp"
#include "updatesolochallengerank.hpp"
#include "getsolochallengerank.hpp"
#include "updateenemylist.hpp"
#include "trychangeds.hpp"
#include "playerchangeds_re.hpp"
#include "kegetstatus.hpp"
#include "kecandidateapply.hpp"
#include "kevoting.hpp"
#include "pshopcreate.hpp"
#include "pshopbuy.hpp"
#include "pshopsell.hpp"
#include "pshopcancelgoods.hpp"
#include "pshopplayerbuy.hpp"
#include "pshopplayersell.hpp"
#include "pshopsettype.hpp"
#include "pshopactive.hpp"
#include "pshopmanagefund.hpp"
#include "pshopdrawitem.hpp"
#include "pshopcleargoods.hpp"
#include "pshopselfget.hpp"
#include "playergivepresent.hpp"
#include "playeraskforpresent.hpp"
#include "touchpointquery.hpp"
#include "touchpointcost.hpp"
#include "giftcoderedeem.hpp"
#include "mobileserverregister.hpp"
#include "uniquedatamodifyrequire.hpp"
#include "tankbattleserverregister.hpp"
#include "tankbattleplayerapply.hpp"
#include "tankbattleplayerenter.hpp"
#include "tankbattleplayerleave.hpp"
#include "tankbattlestart_re.hpp"
#include "tankbattleend.hpp"
#include "tankbattleplayerscoreupdate.hpp"
#include "autoteamconfigregister.hpp"
#include "autoteamsetgoal.hpp"
#include "autoteamplayerready_re.hpp"
#include "factionresourcebattleserverregister.hpp"
#include "factionresourcebattleeventnotice.hpp"
#include "factionresourcebattleplayerquery.hpp"
#include "factionresourcebattlerequestconfig_re.hpp"
#include "factionrenamegsverify_re.hpp"
#include "mnfactionbattleapply.hpp"
#include "mnbattleserverregister.hpp"
#include "mndomainbattlestart_re.hpp"
#include "mndomainbattleenter.hpp"
#include "mndomainbattleentersuccessnotice.hpp"
#include "mndomainbattleleavenotice.hpp"
#include "mndomainbattleend.hpp"
#include "mngettoplist.hpp"
#include "mngetdomaindata.hpp"
#include "announceproviderid.hpp"
#include "playerkickout.hpp"
#include "playerlogin.hpp"
#include "playeroffline.hpp"
#include "playerstatussync.hpp"
#include "playerstatusannounce.hpp"
#include "gettaskdata_re.hpp"
#include "settaskdata_re.hpp"
#include "enterworld.hpp"
#include "c2sgamedatasend.hpp"
#include "publicchat.hpp"
#include "gtradestart.hpp"
#include "gtradeend.hpp"
#include "switchserverstart.hpp"
#include "switchservercancel.hpp"
#include "announcegm.hpp"
#include "gmrestartserver.hpp"
#include "facemodify_re.hpp"
#include "factionbeginsync.hpp"
#include "factionendsync.hpp"
#include "playerfactioninfo_re.hpp"
#include "getplayerfactionrelation_re.hpp"
#include "factioncongregaterequest.hpp"
#include "gmailendsync.hpp"
#include "querygameserverattr_re.hpp"
#include "acreportcheater.hpp"
#include "gmcontrolgame.hpp"
#include "battlestart.hpp"
#include "battleenternotice.hpp"
#include "battlemapnotice.hpp"
#include "addictioncontrol.hpp"
#include "ondivorce.hpp"
#include "billingrequest.hpp"
#include "billingbalance.hpp"
#include "billingbalancesa_re.hpp"
#include "billingconfirm_re.hpp"
#include "sendrefaddbonus.hpp"
#include "sendrewardaddbonus.hpp"
#include "refgetreferencecode.hpp"
#include "privatechat.hpp"
#include "factionchat.hpp"
#include "notifyfactionfortressinfo2.hpp"
#include "factionfortressenternotice.hpp"
#include "addcashnotify.hpp"
#include "syncforceglobaldata.hpp"
#include "aumailsended.hpp"
#include "countrybattleapply_re.hpp"
#include "countrybattleconfignotify.hpp"
#include "countrybattlestart.hpp"
#include "countrybattleenter.hpp"
#include "countrybattlesyncplayerpos.hpp"
#include "countrybattledestroyinstance.hpp"
#include "ssogetticket.hpp"
#include "playerenterleavegt.hpp"
#include "autolockchanged.hpp"
#include "playerchangeds.hpp"
#include "cashmoneyexchangenotify.hpp"
#include "serverforbidnotify.hpp"
#include "servertriggernotify.hpp"
#include "playerrename_re.hpp"
#include "factionrenamegsverify.hpp"
#include "kekingnotify.hpp"
#include "touchpointquery_re.hpp"
#include "touchpointcost_re.hpp"
#include "auaddupmoneyquery_re.hpp"
#include "giftcoderedeem_re.hpp"
#include "uniquedatamodifynotice.hpp"
#include "uniquedatasynch.hpp"
#include "tankbattleenter.hpp"
#include "tankbattlestart.hpp"
#include "autoteamplayerready.hpp"
#include "autoteamcomposestart.hpp"
#include "autoteamcomposefailed.hpp"
#include "factionresourcebattlerequestconfig.hpp"
#include "factionresourcebattlestatusnotice.hpp"
#include "mndomainbattlestart.hpp"
#include "mndomainbattleenter_re.hpp"
#include "mndomaininfogsnotice.hpp"

namespace GNET
{

static PutSpouse __stub_PutSpouse (RPC_PUTSPOUSE, new PutSpouseArg, new RpcRetcode);
static DBPlayerChangeGender __stub_DBPlayerChangeGender (RPC_DBPLAYERCHANGEGENDER, new DBPlayerChangeGenderArg, new DBPlayerChangeGenderRes);
static DBSellPoint __stub_DBSellPoint (RPC_DBSELLPOINT, new SellPointArg, new SellPointRes);
static DBBuyPoint __stub_DBBuyPoint (RPC_DBBUYPOINT, new DBBuyPointArg, new DBBuyPointRes);
static BattleEnd __stub_BattleEnd (RPC_BATTLEEND, new BattleEndArg, new BattleEndRes);
static SysAuctionPrepareItem __stub_SysAuctionPrepareItem (RPC_SYSAUCTIONPREPAREITEM, new SysAuctionPrepareItemArg, new SysAuctionPrepareItemRes);
static GetFactionFortress __stub_GetFactionFortress (RPC_GETFACTIONFORTRESS, new GetFactionFortressArg, new GetFactionFortressRes);
static PutFactionFortress __stub_PutFactionFortress (RPC_PUTFACTIONFORTRESS, new PutFactionFortressArg, new PutFactionFortressRes);
static PlayerKickout_Re __stub_PlayerKickout_Re((void*)0);
static PlayerLogin_Re __stub_PlayerLogin_Re((void*)0);
static PlayerOffline_Re __stub_PlayerOffline_Re((void*)0);
static PlayerLogout __stub_PlayerLogout((void*)0);
static PlayerInfoUpdate __stub_PlayerInfoUpdate((void*)0);
static PlayerTeamOp __stub_PlayerTeamOp((void*)0);
static PlayerTeamMemberOp __stub_PlayerTeamMemberOp((void*)0);
static QueryPlayerStatus __stub_QueryPlayerStatus((void*)0);
static GetTaskData __stub_GetTaskData((void*)0);
static SetTaskData __stub_SetTaskData((void*)0);
static S2CGamedataSend __stub_S2CGamedataSend((void*)0);
static S2CMulticast __stub_S2CMulticast((void*)0);
static S2CBroadcast __stub_S2CBroadcast((void*)0);
static ChatMultiCast __stub_ChatMultiCast((void*)0);
static ChatBroadCast __stub_ChatBroadCast((void*)0);
static ChatSingleCast __stub_ChatSingleCast((void*)0);
static KeepAlive __stub_KeepAlive((void*)0);
static PlayerHeartBeat __stub_PlayerHeartBeat((void*)0);
static DisconnectPlayer __stub_DisconnectPlayer((void*)0);
static GTradeStart_Re __stub_GTradeStart_Re((void*)0);
static GTradeDiscard __stub_GTradeDiscard((void*)0);
static SetChatEmotion __stub_SetChatEmotion((void*)0);
static FaceModify __stub_FaceModify((void*)0);
static FaceModifyCancel __stub_FaceModifyCancel((void*)0);
static SynMutaData __stub_SynMutaData((void*)0);
static SwitchServerSuccess __stub_SwitchServerSuccess((void*)0);
static SwitchServerTimeout __stub_SwitchServerTimeout((void*)0);
static FactionOPRequest __stub_FactionOPRequest((void*)0);
static FactionBeginSync_Re __stub_FactionBeginSync_Re((void*)0);
static PlayerFactionInfo __stub_PlayerFactionInfo((void*)0);
static GetPlayerFactionRelation __stub_GetPlayerFactionRelation((void*)0);
static BattleChallenge __stub_BattleChallenge((void*)0);
static BattleChallengeMap __stub_BattleChallengeMap((void*)0);
static BattleEnter __stub_BattleEnter((void*)0);
static BattleServerRegister __stub_BattleServerRegister((void*)0);
static BattleStart_Re __stub_BattleStart_Re((void*)0);
static DebugCommand __stub_DebugCommand((void*)0);
static GMRestartServer_Re __stub_GMRestartServer_Re((void*)0);
static GMControlGame_Re __stub_GMControlGame_Re((void*)0);
static GetMailList __stub_GetMailList((void*)0);
static GetMail __stub_GetMail((void*)0);
static GetMailAttachObj __stub_GetMailAttachObj((void*)0);
static DeleteMail __stub_DeleteMail((void*)0);
static PreserveMail __stub_PreserveMail((void*)0);
static PlayerSendMail __stub_PlayerSendMail((void*)0);
static PlayerSendMassMail __stub_PlayerSendMassMail((void*)0);
static AuctionOpen __stub_AuctionOpen((void*)0);
static AuctionBid __stub_AuctionBid((void*)0);
static SendAuctionBid __stub_SendAuctionBid((void*)0);
static SendBattleChallenge __stub_SendBattleChallenge((void*)0);
static AuctionList __stub_AuctionList((void*)0);
static AuctionClose __stub_AuctionClose((void*)0);
static AuctionGet __stub_AuctionGet((void*)0);
static AuctionGetItem __stub_AuctionGetItem((void*)0);
static AuctionAttendList __stub_AuctionAttendList((void*)0);
static AuctionExitBid __stub_AuctionExitBid((void*)0);
static AuctionListUpdate __stub_AuctionListUpdate((void*)0);
static QueryRewardType __stub_QueryRewardType((void*)0);
static QueryRewardType_Re __stub_QueryRewardType_Re((void*)0);
static ACTriggerQuestion __stub_ACTriggerQuestion((void*)0);
static QueryGameServerAttr __stub_QueryGameServerAttr((void*)0);
static SellPoint __stub_SellPoint((void*)0);
static BuyPoint __stub_BuyPoint((void*)0);
static GetSellList __stub_GetSellList((void*)0);
static SellCancel __stub_SellCancel((void*)0);
static StockCommission __stub_StockCommission((void*)0);
static StockAccount __stub_StockAccount((void*)0);
static StockTransaction __stub_StockTransaction((void*)0);
static StockBill __stub_StockBill((void*)0);
static StockCancel __stub_StockCancel((void*)0);
static SendRefCashUsed __stub_SendRefCashUsed((void*)0);
static SendTaskReward __stub_SendTaskReward((void*)0);
static RefGetReferenceCode_Re __stub_RefGetReferenceCode_Re((void*)0);
static BillingBalanceSA __stub_BillingBalanceSA((void*)0);
static BillingConfirm __stub_BillingConfirm((void*)0);
static BillingCancel __stub_BillingCancel((void*)0);
static WebTradePrePost __stub_WebTradePrePost((void*)0);
static WebTradePreCancelPost __stub_WebTradePreCancelPost((void*)0);
static WebTradeList __stub_WebTradeList((void*)0);
static WebTradeGetItem __stub_WebTradeGetItem((void*)0);
static WebTradeAttendList __stub_WebTradeAttendList((void*)0);
static WebTradeGetDetail __stub_WebTradeGetDetail((void*)0);
static WebTradeUpdate __stub_WebTradeUpdate((void*)0);
static SysAuctionList __stub_SysAuctionList((void*)0);
static SysAuctionGetItem __stub_SysAuctionGetItem((void*)0);
static SysAuctionAccount __stub_SysAuctionAccount((void*)0);
static SysAuctionBid __stub_SysAuctionBid((void*)0);
static SysAuctionCashTransfer __stub_SysAuctionCashTransfer((void*)0);
static CreateFactionFortress __stub_CreateFactionFortress((void*)0);
static FactionServerRegister __stub_FactionServerRegister((void*)0);
static NotifyFactionFortressState __stub_NotifyFactionFortressState((void*)0);
static FactionFortressEnter __stub_FactionFortressEnter((void*)0);
static FactionFortressList __stub_FactionFortressList((void*)0);
static FactionFortressChallenge __stub_FactionFortressChallenge((void*)0);
static FactionFortressBattleList __stub_FactionFortressBattleList((void*)0);
static FactionFortressGet __stub_FactionFortressGet((void*)0);
static SNSRoleBriefUpdate __stub_SNSRoleBriefUpdate((void*)0);
static NotifyPlayerJoinOrLeaveForce __stub_NotifyPlayerJoinOrLeaveForce((void*)0);
static IncreaseForceActivity __stub_IncreaseForceActivity((void*)0);
static CountryBattleApply __stub_CountryBattleApply((void*)0);
static CountryBattleJoinNotice __stub_CountryBattleJoinNotice((void*)0);
static CountryBattleLeaveNotice __stub_CountryBattleLeaveNotice((void*)0);
static CountryBattleOnlineNotice __stub_CountryBattleOnlineNotice((void*)0);
static CountryBattleOfflineNotice __stub_CountryBattleOfflineNotice((void*)0);
static CountryBattleEnterMapNotice __stub_CountryBattleEnterMapNotice((void*)0);
static CountryBattleServerRegister __stub_CountryBattleServerRegister((void*)0);
static CountryBattleStart_Re __stub_CountryBattleStart_Re((void*)0);
static CountryBattleEnd __stub_CountryBattleEnd((void*)0);
static PlayerRename __stub_PlayerRename((void*)0);
static UpdateSoloChallengeRank __stub_UpdateSoloChallengeRank((void*)0);
static GetSoloChallengeRank __stub_GetSoloChallengeRank((void*)0);
static UpdateEnemyList __stub_UpdateEnemyList((void*)0);
static TryChangeDS __stub_TryChangeDS((void*)0);
static PlayerChangeDS_Re __stub_PlayerChangeDS_Re((void*)0);
static KEGetStatus __stub_KEGetStatus((void*)0);
static KECandidateApply __stub_KECandidateApply((void*)0);
static KEVoting __stub_KEVoting((void*)0);
static PShopCreate __stub_PShopCreate((void*)0);
static PShopBuy __stub_PShopBuy((void*)0);
static PShopSell __stub_PShopSell((void*)0);
static PShopCancelGoods __stub_PShopCancelGoods((void*)0);
static PShopPlayerBuy __stub_PShopPlayerBuy((void*)0);
static PShopPlayerSell __stub_PShopPlayerSell((void*)0);
static PShopSetType __stub_PShopSetType((void*)0);
static PShopActive __stub_PShopActive((void*)0);
static PShopManageFund __stub_PShopManageFund((void*)0);
static PShopDrawItem __stub_PShopDrawItem((void*)0);
static PShopClearGoods __stub_PShopClearGoods((void*)0);
static PShopSelfGet __stub_PShopSelfGet((void*)0);
static PlayerGivePresent __stub_PlayerGivePresent((void*)0);
static PlayerAskForPresent __stub_PlayerAskForPresent((void*)0);
static TouchPointQuery __stub_TouchPointQuery((void*)0);
static TouchPointCost __stub_TouchPointCost((void*)0);
static GiftCodeRedeem __stub_GiftCodeRedeem((void*)0);
static MobileServerRegister __stub_MobileServerRegister((void*)0);
static UniqueDataModifyRequire __stub_UniqueDataModifyRequire((void*)0);
static TankBattleServerRegister __stub_TankBattleServerRegister((void*)0);
static TankBattlePlayerApply __stub_TankBattlePlayerApply((void*)0);
static TankBattlePlayerEnter __stub_TankBattlePlayerEnter((void*)0);
static TankBattlePlayerLeave __stub_TankBattlePlayerLeave((void*)0);
static TankBattleStart_Re __stub_TankBattleStart_Re((void*)0);
static TankBattleEnd __stub_TankBattleEnd((void*)0);
static TankBattlePlayerScoreUpdate __stub_TankBattlePlayerScoreUpdate((void*)0);
static AutoTeamConfigRegister __stub_AutoTeamConfigRegister((void*)0);
static AutoTeamSetGoal __stub_AutoTeamSetGoal((void*)0);
static AutoTeamPlayerReady_Re __stub_AutoTeamPlayerReady_Re((void*)0);
static FactionResourceBattleServerRegister __stub_FactionResourceBattleServerRegister((void*)0);
static FactionResourceBattleEventNotice __stub_FactionResourceBattleEventNotice((void*)0);
static FactionResourceBattlePlayerQuery __stub_FactionResourceBattlePlayerQuery((void*)0);
static FactionResourceBattleRequestConfig_Re __stub_FactionResourceBattleRequestConfig_Re((void*)0);
static FactionRenameGsVerify_Re __stub_FactionRenameGsVerify_Re((void*)0);
static MNFactionBattleApply __stub_MNFactionBattleApply((void*)0);
static MNBattleServerRegister __stub_MNBattleServerRegister((void*)0);
static MNDomainBattleStart_Re __stub_MNDomainBattleStart_Re((void*)0);
static MNDomainBattleEnter __stub_MNDomainBattleEnter((void*)0);
static MNDomainBattleEnterSuccessNotice __stub_MNDomainBattleEnterSuccessNotice((void*)0);
static MNDomainBattleLeaveNotice __stub_MNDomainBattleLeaveNotice((void*)0);
static MNDomainBattleEnd __stub_MNDomainBattleEnd((void*)0);
static MNGetTopList __stub_MNGetTopList((void*)0);
static MNGetDomainData __stub_MNGetDomainData((void*)0);
static AnnounceProviderID __stub_AnnounceProviderID((void*)0);
static PlayerKickout __stub_PlayerKickout((void*)0);
static PlayerLogin __stub_PlayerLogin((void*)0);
static PlayerOffline __stub_PlayerOffline((void*)0);
static PlayerStatusSync __stub_PlayerStatusSync((void*)0);
static PlayerStatusAnnounce __stub_PlayerStatusAnnounce((void*)0);
static GetTaskData_Re __stub_GetTaskData_Re((void*)0);
static SetTaskData_Re __stub_SetTaskData_Re((void*)0);
static EnterWorld __stub_EnterWorld((void*)0);
static C2SGamedataSend __stub_C2SGamedataSend((void*)0);
static PublicChat __stub_PublicChat((void*)0);
static GTradeStart __stub_GTradeStart((void*)0);
static GTradeEnd __stub_GTradeEnd((void*)0);
static SwitchServerStart __stub_SwitchServerStart((void*)0);
static SwitchServerCancel __stub_SwitchServerCancel((void*)0);
static AnnounceGM __stub_AnnounceGM((void*)0);
static GMRestartServer __stub_GMRestartServer((void*)0);
static FaceModify_Re __stub_FaceModify_Re((void*)0);
static FactionBeginSync __stub_FactionBeginSync((void*)0);
static FactionEndSync __stub_FactionEndSync((void*)0);
static PlayerFactionInfo_Re __stub_PlayerFactionInfo_Re((void*)0);
static GetPlayerFactionRelation_Re __stub_GetPlayerFactionRelation_Re((void*)0);
static FactionCongregateRequest __stub_FactionCongregateRequest((void*)0);
static GMailEndSync __stub_GMailEndSync((void*)0);
static QueryGameServerAttr_Re __stub_QueryGameServerAttr_Re((void*)0);
static ACReportCheater __stub_ACReportCheater((void*)0);
static GMControlGame __stub_GMControlGame((void*)0);
static BattleStart __stub_BattleStart((void*)0);
static BattleEnterNotice __stub_BattleEnterNotice((void*)0);
static BattleMapNotice __stub_BattleMapNotice((void*)0);
static AddictionControl __stub_AddictionControl((void*)0);
static OnDivorce __stub_OnDivorce((void*)0);
static BillingRequest __stub_BillingRequest((void*)0);
static BillingBalance __stub_BillingBalance((void*)0);
static BillingBalanceSA_Re __stub_BillingBalanceSA_Re((void*)0);
static BillingConfirm_Re __stub_BillingConfirm_Re((void*)0);
static SendRefAddBonus __stub_SendRefAddBonus((void*)0);
static SendRewardAddBonus __stub_SendRewardAddBonus((void*)0);
static RefGetReferenceCode __stub_RefGetReferenceCode((void*)0);
static PrivateChat __stub_PrivateChat((void*)0);
static FactionChat __stub_FactionChat((void*)0);
static NotifyFactionFortressInfo2 __stub_NotifyFactionFortressInfo2((void*)0);
static FactionFortressEnterNotice __stub_FactionFortressEnterNotice((void*)0);
static AddCashNotify __stub_AddCashNotify((void*)0);
static SyncForceGlobalData __stub_SyncForceGlobalData((void*)0);
static AUMailSended __stub_AUMailSended((void*)0);
static CountryBattleApply_Re __stub_CountryBattleApply_Re((void*)0);
static CountryBattleConfigNotify __stub_CountryBattleConfigNotify((void*)0);
static CountryBattleStart __stub_CountryBattleStart((void*)0);
static CountryBattleEnter __stub_CountryBattleEnter((void*)0);
static CountryBattleSyncPlayerPos __stub_CountryBattleSyncPlayerPos((void*)0);
static CountryBattleDestroyInstance __stub_CountryBattleDestroyInstance((void*)0);
static SSOGetTicket __stub_SSOGetTicket((void*)0);
static PlayerEnterLeaveGT __stub_PlayerEnterLeaveGT((void*)0);
static AutolockChanged __stub_AutolockChanged((void*)0);
static PlayerChangeDS __stub_PlayerChangeDS((void*)0);
static CashMoneyExchangeNotify __stub_CashMoneyExchangeNotify((void*)0);
static ServerForbidNotify __stub_ServerForbidNotify((void*)0);
static ServerTriggerNotify __stub_ServerTriggerNotify((void*)0);
static PlayerRename_Re __stub_PlayerRename_Re((void*)0);
static FactionRenameGsVerify __stub_FactionRenameGsVerify((void*)0);
static KEKingNotify __stub_KEKingNotify((void*)0);
static TouchPointQuery_Re __stub_TouchPointQuery_Re((void*)0);
static TouchPointCost_Re __stub_TouchPointCost_Re((void*)0);
static AuAddupMoneyQuery_Re __stub_AuAddupMoneyQuery_Re((void*)0);
static GiftCodeRedeem_Re __stub_GiftCodeRedeem_Re((void*)0);
static UniqueDataModifyNotice __stub_UniqueDataModifyNotice((void*)0);
static UniqueDataSynch __stub_UniqueDataSynch((void*)0);
static TankBattleEnter __stub_TankBattleEnter((void*)0);
static TankBattleStart __stub_TankBattleStart((void*)0);
static AutoTeamPlayerReady __stub_AutoTeamPlayerReady((void*)0);
static AutoTeamComposeStart __stub_AutoTeamComposeStart((void*)0);
static AutoTeamComposeFailed __stub_AutoTeamComposeFailed((void*)0);
static FactionResourceBattleRequestConfig __stub_FactionResourceBattleRequestConfig((void*)0);
static FactionResourceBattleStatusNotice __stub_FactionResourceBattleStatusNotice((void*)0);
static MNDomainBattleStart __stub_MNDomainBattleStart((void*)0);
static MNDomainBattleEnter_Re __stub_MNDomainBattleEnter_Re((void*)0);
static MNDomainInfoGSNotice __stub_MNDomainInfoGSNotice((void*)0);

};
