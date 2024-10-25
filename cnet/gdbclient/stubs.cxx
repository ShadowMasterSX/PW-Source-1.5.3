#ifdef WIN32
#include <winsock2.h>
#include "gncompress.h"
#else
#include "binder.h"
#endif
#include "dbcreaterole.hrp"
#include "dbdeleterole.hrp"
#include "dbundodeleterole.hrp"
#include "putuser.hrp"
#include "getuser.hrp"
#include "deluser.hrp"
#include "getrole.hrp"
#include "getroleinfo.hrp"
#include "delrole.hrp"
#include "putrolebase.hrp"
#include "getrolebase.hrp"
#include "getrolepocket.hrp"
#include "putrolepocket.hrp"
#include "putrolestatus.hrp"
#include "getrolestatus.hrp"
#include "putroleequipment.hrp"
#include "getroleequipment.hrp"
#include "putroletask.hrp"
#include "getroletask.hrp"
#include "putroledata.hrp"
#include "getroledata.hrp"
#include "tradeinventory.hrp"
#include "tradesave.hrp"
#include "putrole.hrp"
#include "getmoneyinventory.hrp"
#include "putmoneyinventory.hrp"
#include "getrolebasestatus.hrp"
#include "putrolestorehouse.hrp"
#include "getrolestorehouse.hrp"
#include "putroleforbid.hrp"
#include "getroleforbid.hrp"
#include "getroleid.hrp"
#include "getfriendlist.hrp"
#include "putfriendlist.hrp"
#include "putmessage.hrp"
#include "getmessage.hrp"
#include "gettaskdatarpc.hrp"
#include "puttaskdatarpc.hrp"
#include "dbverifymaster.hrp"
#include "dbgetconsumeinfos.hrp"
#include "dbmappasswordload.hrp"
#include "dbmappasswordsave.hrp"
#include "getuserroles.hrp"
#include "clearstorehousepasswd.hrp"
#include "canchangerolename.hrp"
#include "renamerole.hrp"
#include "uid2logicuid.hrp"
#include "roleid2uid.hrp"
#include "transactionacquire.hrp"
#include "transactionabort.hrp"
#include "transactioncommit.hrp"
#include "dbgetmaillist.hrp"
#include "dbgetmail.hrp"
#include "dbgetmailattach.hrp"
#include "dbsetmailattr.hrp"
#include "dbsendmail.hrp"
#include "dbdeletemail.hrp"
#include "dbsendmassmail.hrp"
#include "dbauctionopen.hrp"
#include "dbauctionbid.hrp"
#include "dbauctionget.hrp"
#include "dbauctionclose.hrp"
#include "dbauctionlist.hrp"
#include "dbauctiontimeout.hrp"
#include "dbsellpoint.hrp"
#include "dbbuypoint.hrp"
#include "dbsyncsellinfo.hrp"
#include "dbselltimeout.hrp"
#include "dbsellcancel.hrp"
#include "dbtranspointdeal.hrp"
#include "dbbattlechallenge.hrp"
#include "dbbattleload.hrp"
#include "dbbattleset.hrp"
#include "dbbattleend.hrp"
#include "dbbattlemail.hrp"
#include "dbbattlebonus.hrp"
#include "cashserial.hrp"
#include "getaddcashsn.hrp"
#include "dbstockload.hrp"
#include "dbstocktransaction.hrp"
#include "dbstockbalance.hrp"
#include "dbstockcommission.hrp"
#include "dbstockcancel.hrp"
#include "dbsetcashpassword.hrp"
#include "putspouse.hrp"
#include "dbautolockset.hrp"
#include "dbautolockget.hrp"
#include "dbautolocksetoffline.hrp"
#include "queryuserid.hrp"
#include "forbiduser.hrp"
#include "dbforbiduser.hrp"
#include "dbclearconsumable.hrp"
#include "dbrefwithdrawtrans.hrp"
#include "dbrefgetreferral.hrp"
#include "dbrefgetreferrer.hrp"
#include "dbrefupdatereferral.hrp"
#include "dbrefupdatereferrer.hrp"
#include "dbgetreward.hrp"
#include "dbputconsumepoints.hrp"
#include "dbputrewardbonus.hrp"
#include "dbrewardmature.hrp"
#include "dbexchangeconsumepoints.hrp"
#include "dbplayerpositionreset.hrp"
#include "dbwebtradeload.hrp"
#include "dbwebtradeloadsold.hrp"
#include "dbwebtradeprepost.hrp"
#include "dbwebtradeprecancelpost.hrp"
#include "dbwebtradepost.hrp"
#include "dbwebtradecancelpost.hrp"
#include "dbwebtradeshelf.hrp"
#include "dbwebtradecancelshelf.hrp"
#include "dbwebtradesold.hrp"
#include "dbwebtradepostexpire.hrp"
#include "dbwebtradegetrolesimpleinfo.hrp"
#include "dbsysauctioncashtransfer.hrp"
#include "dbsysauctioncashspend.hrp"
#include "putserverdata.hrp"
#include "getserverdata.hrp"
#include "getcashtotal.hrp"
#include "dbfactionfortressload.hrp"
#include "dbputfactionfortress.hrp"
#include "dbdelfactionfortress.hrp"
#include "dbcreatefactionfortress.hrp"
#include "dbfactionfortresschallenge.hrp"
#include "dbgametalkrolelist.hrp"
#include "dbgametalkrolerelation.hrp"
#include "dbgametalkfactioninfo.hrp"
#include "dbgametalkrolestatus.hrp"
#include "dbgametalkroleinfo.hrp"
#include "dbforceload.hrp"
#include "dbputforce.hrp"
#include "dbcountrybattlebonus.hrp"
#include "fetchplayerdata.hrp"
#include "activateplayerdata.hrp"
#include "delplayerdata.hrp"
#include "saveplayerdata.hrp"
#include "playeridentitymatch.hrp"
#include "freezeplayerdata.hrp"
#include "touchplayerdata.hrp"
#include "dbupdateplayercrossinfo.hrp"
#include "dbloadglobalcontrol.hrp"
#include "dbputglobalcontrol.hrp"
#include "dbuniquedataload.hrp"
#include "dbuniquedatasave.hrp"
#include "dbplayerrename.hrp"
#include "dbrolenamelist.hrp"
#include "dbkeload.hrp"
#include "dbkecandidateapply.hrp"
#include "dbkecandidateconfirm.hrp"
#include "dbkevoting.hrp"
#include "dbkekingconfirm.hrp"
#include "dbkedeleteking.hrp"
#include "dbkedeletecandidate.hrp"
#include "dbpshopcreate.hrp"
#include "dbpshopbuy.hrp"
#include "dbpshopsell.hrp"
#include "dbpshopcancelgoods.hrp"
#include "dbpshopplayerbuy.hrp"
#include "dbpshopplayersell.hrp"
#include "dbpshopsettype.hrp"
#include "dbpshopactive.hrp"
#include "dbpshopmanagefund.hrp"
#include "dbpshopdrawitem.hrp"
#include "dbpshopload.hrp"
#include "dbpshopget.hrp"
#include "dbpshopcleargoods.hrp"
#include "dbpshoptimeout.hrp"
#include "dbplayergivepresent.hrp"
#include "dbplayeraskforpresent.hrp"
#include "dbsysmail3.hrp"
#include "dbgetplayerprofiledata.hrp"
#include "dbputplayerprofiledata.hrp"
#include "dbtankbattlebonus.hrp"
#include "dbfactionresourcebattlebonus.hrp"
#include "dbcopyrole.hrp"
#include "dbfactionrename.hrp"
#include "delroleannounce.hpp"
#include "dbfriendextlist_re.hpp"
#include "transbuypoint.hpp"
#include "syncsellinfo.hpp"
#include "addcash.hpp"
#include "addcash_re.hpp"
#include "domaincmd_re.hpp"
#include "debugaddcash.hpp"

namespace GNET
{

static DBCreateRole __stub_DBCreateRole (RPC_DBCREATEROLE, new DBCreateRoleArg, new DBCreateRoleRes);
static DBDeleteRole __stub_DBDeleteRole (RPC_DBDELETEROLE, new DBDeleteRoleArg, new DBDeleteRoleRes);
static DBUndoDeleteRole __stub_DBUndoDeleteRole (RPC_DBUNDODELETEROLE, new DBUndoDeleteRoleArg, new DBUndoDeleteRoleRes);
static PutUser __stub_PutUser (RPC_PUTUSER, new UserPair, new RpcRetcode);
static GetUser __stub_GetUser (RPC_GETUSER, new UserArg, new UserRes);
static DelUser __stub_DelUser (RPC_DELUSER, new UserID, new RpcRetcode);
static GetRole __stub_GetRole (RPC_GETROLE, new RoleArg, new RoleRes);
static GetRoleInfo __stub_GetRoleInfo (RPC_GETROLEINFO, new RoleId, new RoleInfoRes);
static DelRole __stub_DelRole (RPC_DELROLE, new RoleId, new RpcRetcode);
static PutRoleBase __stub_PutRoleBase (RPC_PUTROLEBASE, new RoleBasePair, new RpcRetcode);
static GetRoleBase __stub_GetRoleBase (RPC_GETROLEBASE, new RoleId, new RoleBaseRes);
static GetRolePocket __stub_GetRolePocket (RPC_GETROLEPOCKET, new RoleId, new RolePocketRes);
static PutRolePocket __stub_PutRolePocket (RPC_PUTROLEPOCKET, new RolePocketPair, new RpcRetcode);
static PutRoleStatus __stub_PutRoleStatus (RPC_PUTROLESTATUS, new RoleStatusPair, new RpcRetcode);
static GetRoleStatus __stub_GetRoleStatus (RPC_GETROLESTATUS, new RoleId, new RoleStatusRes);
static PutRoleEquipment __stub_PutRoleEquipment (RPC_PUTROLEEQUIPMENT, new RoleEquipmentPair, new RpcRetcode);
static GetRoleEquipment __stub_GetRoleEquipment (RPC_GETROLEEQUIPMENT, new RoleId, new RoleEquipmentRes);
static PutRoleTask __stub_PutRoleTask (RPC_PUTROLETASK, new RoleTaskPair, new RpcRetcode);
static GetRoleTask __stub_GetRoleTask (RPC_GETROLETASK, new RoleId, new RoleTaskRes);
static PutRoleData __stub_PutRoleData (RPC_PUTROLEDATA, new RoleDataPair, new RpcRetcode);
static GetRoleData __stub_GetRoleData (RPC_GETROLEDATA, new RoleId, new RoleDataRes);
static TradeInventory __stub_TradeInventory (RPC_TRADEINVENTORY, new TradeInventoryArg, new TradeInventoryRes);
static TradeSave __stub_TradeSave (RPC_TRADESAVE, new TradeSaveArg, new TradeSaveRes);
static PutRole __stub_PutRole (RPC_PUTROLE, new RolePair, new RpcRetcode);
static GetMoneyInventory __stub_GetMoneyInventory (RPC_GETMONEYINVENTORY, new GetMoneyInventoryArg, new GetMoneyInventoryRes);
static PutMoneyInventory __stub_PutMoneyInventory (RPC_PUTMONEYINVENTORY, new PutMoneyInventoryArg, new RpcRetcode);
static GetRoleBaseStatus __stub_GetRoleBaseStatus (RPC_GETROLEBASESTATUS, new RoleId, new GetRoleBaseStatusRes);
static PutRoleStorehouse __stub_PutRoleStorehouse (RPC_PUTROLESTOREHOUSE, new RoleStorehousePair, new RpcRetcode);
static GetRoleStorehouse __stub_GetRoleStorehouse (RPC_GETROLESTOREHOUSE, new RoleId, new RoleStorehouseRes);
static PutRoleForbid __stub_PutRoleForbid (RPC_PUTROLEFORBID, new RoleForbidPair, new RpcRetcode);
static GetRoleForbid __stub_GetRoleForbid (RPC_GETROLEFORBID, new GetRoleForbidArg, new GetRoleForbidRes);
static GetRoleId __stub_GetRoleId (RPC_GETROLEID, new GetRoleIdArg, new GetRoleIdRes);
static GetFriendList __stub_GetFriendList (RPC_GETFRIENDLIST, new RoleId, new FriendListRes);
static PutFriendList __stub_PutFriendList (RPC_PUTFRIENDLIST, new FriendListPair, new RpcRetcode);
static PutMessage __stub_PutMessage (RPC_PUTMESSAGE, new Message, new RpcRetcode);
static GetMessage __stub_GetMessage (RPC_GETMESSAGE, new RoleId, new GetMessageRes);
static GetTaskDataRpc __stub_GetTaskDataRpc (RPC_GETTASKDATARPC, new GetTaskDataArg, new GetTaskDataRes);
static PutTaskDataRpc __stub_PutTaskDataRpc (RPC_PUTTASKDATARPC, new PutTaskDataArg, new PutTaskDataRes);
static DBVerifyMaster __stub_DBVerifyMaster (RPC_DBVERIFYMASTER, new DBVerifyMasterArg, new DefFactionRes);
static DBGetConsumeInfos __stub_DBGetConsumeInfos (RPC_DBGETCONSUMEINFOS, new DBGetConsumeInfosArg, new DBGetConsumeInfosRes);
static DBMapPasswordLoad __stub_DBMapPasswordLoad (RPC_DBMAPPASSWORDLOAD, new DBMapPasswordLoadArg, new DBMapPasswordLoadRes);
static DBMapPasswordSave __stub_DBMapPasswordSave (RPC_DBMAPPASSWORDSAVE, new DBMapPasswordSaveArg, new DBMapPasswordSaveRes);
static GetUserRoles __stub_GetUserRoles (RPC_GETUSERROLES, new GetUserRolesArg, new GetUserRolesRes);
static ClearStorehousePasswd __stub_ClearStorehousePasswd (RPC_CLEARSTOREHOUSEPASSWD, new ClearStorehousePasswdArg, new RpcRetcode);
static CanChangeRolename __stub_CanChangeRolename (RPC_CANCHANGEROLENAME, new CanChangeRolenameArg, new CanChangeRolenameRes);
static RenameRole __stub_RenameRole (RPC_RENAMEROLE, new RenameRoleArg, new RpcRetcode);
static Uid2Logicuid __stub_Uid2Logicuid (RPC_UID2LOGICUID, new Uid2LogicuidArg, new Uid2LogicuidRes);
static Roleid2Uid __stub_Roleid2Uid (RPC_ROLEID2UID, new Roleid2UidArg, new Roleid2UidRes);
static TransactionAcquire __stub_TransactionAcquire (RPC_TRANSACTIONACQUIRE, new TransactionTimeout, new TransactionId);
static TransactionAbort __stub_TransactionAbort (RPC_TRANSACTIONABORT, new TransactionId, new RpcRetcode);
static TransactionCommit __stub_TransactionCommit (RPC_TRANSACTIONCOMMIT, new TransactionId, new RpcRetcode);
static DBGetMailList __stub_DBGetMailList (RPC_DBGETMAILLIST, new RoleId, new DBGetMailListRes);
static DBGetMail __stub_DBGetMail (RPC_DBGETMAIL, new GMailID, new DBGetMailRes);
static DBGetMailAttach __stub_DBGetMailAttach (RPC_DBGETMAILATTACH, new DBGetMailAttachArg, new DBGetMailAttachRes);
static DBSetMailAttr __stub_DBSetMailAttr (RPC_DBSETMAILATTR, new DBSetMailAttrArg, new DBSetMailAttrRes);
static DBSendMail __stub_DBSendMail (RPC_DBSENDMAIL, new DBSendMailArg, new DBSendMailRes);
static DBDeleteMail __stub_DBDeleteMail (RPC_DBDELETEMAIL, new DBDeleteMailArg, new GMailDefRes);
static DBSendMassMail __stub_DBSendMassMail (RPC_DBSENDMASSMAIL, new DBSendMassMailArg, new DBSendMassMailRes);
static DBAuctionOpen __stub_DBAuctionOpen (RPC_DBAUCTIONOPEN, new DBAuctionOpenArg, new DBAuctionOpenRes);
static DBAuctionBid __stub_DBAuctionBid (RPC_DBAUCTIONBID, new DBAuctionBidArg, new DBAuctionBidRes);
static DBAuctionGet __stub_DBAuctionGet (RPC_DBAUCTIONGET, new DBAuctionGetArg, new DBAuctionGetRes);
static DBAuctionClose __stub_DBAuctionClose (RPC_DBAUCTIONCLOSE, new DBAuctionCloseArg, new DBAuctionCloseRes);
static DBAuctionList __stub_DBAuctionList (RPC_DBAUCTIONLIST, new DBAuctionListArg, new DBAuctionListRes);
static DBAuctionTimeout __stub_DBAuctionTimeout (RPC_DBAUCTIONTIMEOUT, new AuctionId, new DBAuctionTimeoutRes);
static DBSellPoint __stub_DBSellPoint (RPC_DBSELLPOINT, new SellPointArg, new SellPointRes);
static DBBuyPoint __stub_DBBuyPoint (RPC_DBBUYPOINT, new DBBuyPointArg, new DBBuyPointRes);
static DBSyncSellInfo __stub_DBSyncSellInfo (RPC_DBSYNCSELLINFO, new RoleId, new DBSyncSellInfoRes);
static DBSellTimeout __stub_DBSellTimeout (RPC_DBSELLTIMEOUT, new SellID, new DBSyncSellInfoRes);
static DBSellCancel __stub_DBSellCancel (RPC_DBSELLCANCEL, new SellID, new DBSyncSellInfoRes);
static DBTransPointDeal __stub_DBTransPointDeal (RPC_DBTRANSPOINTDEAL, new RoleId, new DBTransPointDealRes);
static DBBattleChallenge __stub_DBBattleChallenge (RPC_DBBATTLECHALLENGE, new DBBattleChallengeArg, new DBBattleChallengeRes);
static DBBattleLoad __stub_DBBattleLoad (RPC_DBBATTLELOAD, new DBBattleLoadArg, new DBBattleLoadRes);
static DBBattleSet __stub_DBBattleSet (RPC_DBBATTLESET, new DBBattleSetArg, new DBBattleSetRes);
static DBBattleEnd __stub_DBBattleEnd (RPC_DBBATTLEEND, new DBBattleEndArg, new DBBattleEndRes);
static DBBattleMail __stub_DBBattleMail (RPC_DBBATTLEMAIL, new DBBattleMailArg, new DBBattleMailRes);
static DBBattleBonus __stub_DBBattleBonus (RPC_DBBATTLEBONUS, new DBBattleBonusArg, new DBBattleBonusRes);
static CashSerial __stub_CashSerial (RPC_CASHSERIAL, new CashSerialArg, new CashSerialRes);
static GetAddCashSN __stub_GetAddCashSN (RPC_GETADDCASHSN, new GetAddCashSNArg, new GetAddCashSNRes);
static DBStockLoad __stub_DBStockLoad (RPC_DBSTOCKLOAD, new DBStockLoadArg, new DBStockLoadRes);
static DBStockTransaction __stub_DBStockTransaction (RPC_DBSTOCKTRANSACTION, new DBStockTransactionArg, new DBStockTransactionRes);
static DBStockBalance __stub_DBStockBalance (RPC_DBSTOCKBALANCE, new DBStockBalanceArg, new DBStockBalanceRes);
static DBStockCommission __stub_DBStockCommission (RPC_DBSTOCKCOMMISSION, new DBStockCommissionArg, new DBStockCommissionRes);
static DBStockCancel __stub_DBStockCancel (RPC_DBSTOCKCANCEL, new DBStockCancelArg, new DBStockCancelRes);
static DBSetCashPassword __stub_DBSetCashPassword (RPC_DBSETCASHPASSWORD, new DBSetCashPasswordArg, new DBSetCashPasswordRes);
static PutSpouse __stub_PutSpouse (RPC_PUTSPOUSE, new PutSpouseArg, new RpcRetcode);
static DBAutolockSet __stub_DBAutolockSet (RPC_DBAUTOLOCKSET, new DBAutolockSetArg, new Integer);
static DBAutolockGet __stub_DBAutolockGet (RPC_DBAUTOLOCKGET, new DBAutolockGetArg, new DBAutolockGetRes);
static DBAutolockSetOffline __stub_DBAutolockSetOffline (RPC_DBAUTOLOCKSETOFFLINE, new DBAutolockSetOfflineArg, new DBAutolockSetOfflineRes);
static QueryUserid __stub_QueryUserid (RPC_QUERYUSERID, new QueryUseridArg, new QueryUseridRes);
static ForbidUser __stub_ForbidUser (RPC_FORBIDUSER, new ForbidUserArg, new ForbidUserRes);
static DBForbidUser __stub_DBForbidUser (RPC_DBFORBIDUSER, new ForbidUserArg, new RpcRetcode);
static DBClearConsumable __stub_DBClearConsumable (RPC_DBCLEARCONSUMABLE, new DBClearConsumableArg, new RpcRetcode);
static DBRefWithdrawTrans __stub_DBRefWithdrawTrans (RPC_DBREFWITHDRAWTRANS, new DBRefWithdrawTransArg, new RpcRetcode);
static DBRefGetReferral __stub_DBRefGetReferral (RPC_DBREFGETREFERRAL, new Integer, new DBRefGetReferralRes);
static DBRefGetReferrer __stub_DBRefGetReferrer (RPC_DBREFGETREFERRER, new Integer, new DBRefGetReferrerRes);
static DBRefUpdateReferral __stub_DBRefUpdateReferral (RPC_DBREFUPDATEREFERRAL, new DBRefUpdateReferralArg, new RpcRetcode);
static DBRefUpdateReferrer __stub_DBRefUpdateReferrer (RPC_DBREFUPDATEREFERRER, new DBRefUpdateReferrerArg, new RpcRetcode);
static DBGetReward __stub_DBGetReward (RPC_DBGETREWARD, new DBGetRewardArg, new DBGetRewardRes);
static DBPutConsumePoints __stub_DBPutConsumePoints (RPC_DBPUTCONSUMEPOINTS, new DBPutConsumePointsArg, new RpcRetcode);
static DBPutRewardBonus __stub_DBPutRewardBonus (RPC_DBPUTREWARDBONUS, new DBPutRewardBonusArg, new RpcRetcode);
static DBRewardMature __stub_DBRewardMature (RPC_DBREWARDMATURE, new DBRewardMatureArg, new DBRewardMatureRes);
static DBExchangeConsumePoints __stub_DBExchangeConsumePoints (RPC_DBEXCHANGECONSUMEPOINTS, new DBExchangeConsumePointsArg, new DBExchangeConsumePointsRes);
static DBPlayerPositionReset __stub_DBPlayerPositionReset (RPC_DBPLAYERPOSITIONRESET, new DBPlayerPositionResetArg, new DBPlayerPositionResetRes);
static DBWebTradeLoad __stub_DBWebTradeLoad (RPC_DBWEBTRADELOAD, new DBWebTradeLoadArg, new DBWebTradeLoadRes);
static DBWebTradeLoadSold __stub_DBWebTradeLoadSold (RPC_DBWEBTRADELOADSOLD, new DBWebTradeLoadSoldArg, new DBWebTradeLoadSoldRes);
static DBWebTradePrePost __stub_DBWebTradePrePost (RPC_DBWEBTRADEPREPOST, new DBWebTradePrePostArg, new DBWebTradePrePostRes);
static DBWebTradePreCancelPost __stub_DBWebTradePreCancelPost (RPC_DBWEBTRADEPRECANCELPOST, new DBWebTradePreCancelPostArg, new DBWebTradePreCancelPostRes);
static DBWebTradePost __stub_DBWebTradePost (RPC_DBWEBTRADEPOST, new DBWebTradePostArg, new DBWebTradePostRes);
static DBWebTradeCancelPost __stub_DBWebTradeCancelPost (RPC_DBWEBTRADECANCELPOST, new DBWebTradeCancelPostArg, new DBWebTradeCancelPostRes);
static DBWebTradeShelf __stub_DBWebTradeShelf (RPC_DBWEBTRADESHELF, new DBWebTradeShelfArg, new DBWebTradeShelfRes);
static DBWebTradeCancelShelf __stub_DBWebTradeCancelShelf (RPC_DBWEBTRADECANCELSHELF, new DBWebTradeCancelShelfArg, new DBWebTradeCancelShelfRes);
static DBWebTradeSold __stub_DBWebTradeSold (RPC_DBWEBTRADESOLD, new DBWebTradeSoldArg, new DBWebTradeSoldRes);
static DBWebTradePostExpire __stub_DBWebTradePostExpire (RPC_DBWEBTRADEPOSTEXPIRE, new DBWebTradePostExpireArg, new DBWebTradePostExpireRes);
static DBWebTradeGetRoleSimpleInfo __stub_DBWebTradeGetRoleSimpleInfo (RPC_DBWEBTRADEGETROLESIMPLEINFO, new DBWebTradeGetRoleSimpleInfoArg, new DBWebTradeGetRoleSimpleInfoRes);
static DBSysAuctionCashTransfer __stub_DBSysAuctionCashTransfer (RPC_DBSYSAUCTIONCASHTRANSFER, new DBSysAuctionCashTransferArg, new DBSysAuctionCashTransferRes);
static DBSysAuctionCashSpend __stub_DBSysAuctionCashSpend (RPC_DBSYSAUCTIONCASHSPEND, new DBSysAuctionCashSpendArg, new DBSysAuctionCashSpendRes);
static PutServerData __stub_PutServerData (RPC_PUTSERVERDATA, new PutServerDataArg, new RpcRetcode);
static GetServerData __stub_GetServerData (RPC_GETSERVERDATA, new GetServerDataArg, new GetServerDataRes);
static GetCashTotal __stub_GetCashTotal (RPC_GETCASHTOTAL, new GetCashTotalArg, new GetCashTotalRes);
static DBFactionFortressLoad __stub_DBFactionFortressLoad (RPC_DBFACTIONFORTRESSLOAD, new DBFactionFortressLoadArg, new DBFactionFortressLoadRes);
static DBPutFactionFortress __stub_DBPutFactionFortress (RPC_DBPUTFACTIONFORTRESS, new DBPutFactionFortressArg, new DBPutFactionFortressRes);
static DBDelFactionFortress __stub_DBDelFactionFortress (RPC_DBDELFACTIONFORTRESS, new DBDelFactionFortressArg, new DBDelFactionFortressRes);
static DBCreateFactionFortress __stub_DBCreateFactionFortress (RPC_DBCREATEFACTIONFORTRESS, new DBCreateFactionFortressArg, new DBCreateFactionFortressRes);
static DBFactionFortressChallenge __stub_DBFactionFortressChallenge (RPC_DBFACTIONFORTRESSCHALLENGE, new DBFactionFortressChallengeArg, new DBFactionFortressChallengeRes);
static DBGameTalkRoleList __stub_DBGameTalkRoleList (RPC_DBGAMETALKROLELIST, new DBGameTalkRoleListArg, new DBGameTalkRoleListRes);
static DBGameTalkRoleRelation __stub_DBGameTalkRoleRelation (RPC_DBGAMETALKROLERELATION, new DBGameTalkRoleRelationArg, new DBGameTalkRoleRelationRes);
static DBGameTalkFactionInfo __stub_DBGameTalkFactionInfo (RPC_DBGAMETALKFACTIONINFO, new DBGameTalkFactionInfoArg, new DBGameTalkFactionInfoRes);
static DBGameTalkRoleStatus __stub_DBGameTalkRoleStatus (RPC_DBGAMETALKROLESTATUS, new DBGameTalkRoleStatusArg, new DBGameTalkRoleStatusRes);
static DBGameTalkRoleInfo __stub_DBGameTalkRoleInfo (RPC_DBGAMETALKROLEINFO, new DBGameTalkRoleInfoArg, new DBGameTalkRoleInfoRes);
static DBForceLoad __stub_DBForceLoad (RPC_DBFORCELOAD, new DBForceLoadArg, new DBForceLoadRes);
static DBPutForce __stub_DBPutForce (RPC_DBPUTFORCE, new DBPutForceArg, new DBPutForceRes);
static DBCountryBattleBonus __stub_DBCountryBattleBonus (RPC_DBCOUNTRYBATTLEBONUS, new DBCountryBattleBonusArg, new DBCountryBattleBonusRes);
static FetchPlayerData __stub_FetchPlayerData (RPC_FETCHPLAYERDATA, new FetchPlayerDataArg, new FetchPlayerDataRes);
static ActivatePlayerData __stub_ActivatePlayerData (RPC_ACTIVATEPLAYERDATA, new ActivatePlayerDataArg, new ActivatePlayerDataRes);
static DelPlayerData __stub_DelPlayerData (RPC_DELPLAYERDATA, new DelPlayerDataArg, new DelPlayerDataRes);
static SavePlayerData __stub_SavePlayerData (RPC_SAVEPLAYERDATA, new SavePlayerDataArg, new SavePlayerDataRes);
static PlayerIdentityMatch __stub_PlayerIdentityMatch (RPC_PLAYERIDENTITYMATCH, new PlayerIdentityMatchArg, new PlayerIdentityMatchRes);
static FreezePlayerData __stub_FreezePlayerData (RPC_FREEZEPLAYERDATA, new FreezePlayerDataArg, new FreezePlayerDataRes);
static TouchPlayerData __stub_TouchPlayerData (RPC_TOUCHPLAYERDATA, new TouchPlayerDataArg, new TouchPlayerDataRes);
static DBUpdatePlayerCrossInfo __stub_DBUpdatePlayerCrossInfo (RPC_DBUPDATEPLAYERCROSSINFO, new DBUpdatePlayerCrossInfoArg, new DBUpdatePlayerCrossInfoRes);
static DBLoadGlobalControl __stub_DBLoadGlobalControl (RPC_DBLOADGLOBALCONTROL, new DBLoadGlobalControlArg, new DBLoadGlobalControlRes);
static DBPutGlobalControl __stub_DBPutGlobalControl (RPC_DBPUTGLOBALCONTROL, new DBPutGlobalControlArg, new DBPutGlobalControlRes);
static DBUniqueDataLoad __stub_DBUniqueDataLoad (RPC_DBUNIQUEDATALOAD, new DBUniqueDataLoadArg, new DBUniqueDataLoadRes);
static DBUniqueDataSave __stub_DBUniqueDataSave (RPC_DBUNIQUEDATASAVE, new DBUniqueDataSaveArg, new DBUniqueDataSaveRes);
static DBPlayerRename __stub_DBPlayerRename (RPC_DBPLAYERRENAME, new DBPlayerRenameArg, new DBPlayerRenameRes);
static DBRoleNameList __stub_DBRoleNameList (RPC_DBROLENAMELIST, new DBRoleNameListArg, new DBRoleNameListRes);
static DBKELoad __stub_DBKELoad (RPC_DBKELOAD, new DBKELoadArg, new DBKELoadRes);
static DBKECandidateApply __stub_DBKECandidateApply (RPC_DBKECANDIDATEAPPLY, new DBKECandidateApplyArg, new DBKECandidateApplyRes);
static DBKECandidateConfirm __stub_DBKECandidateConfirm (RPC_DBKECANDIDATECONFIRM, new DBKECandidateConfirmArg, new DBKECandidateConfirmRes);
static DBKEVoting __stub_DBKEVoting (RPC_DBKEVOTING, new DBKEVotingArg, new DBKEVotingRes);
static DBKEKingConfirm __stub_DBKEKingConfirm (RPC_DBKEKINGCONFIRM, new DBKEKingConfirmArg, new DBKEKingConfirmRes);
static DBKEDeleteKing __stub_DBKEDeleteKing (RPC_DBKEDELETEKING, new DBKEDeleteKingArg, new DBKEDeleteKingRes);
static DBKEDeleteCandidate __stub_DBKEDeleteCandidate (RPC_DBKEDELETECANDIDATE, new DBKEDeleteCandidateArg, new DBKEDeleteCandidateRes);
static DBPShopCreate __stub_DBPShopCreate (RPC_DBPSHOPCREATE, new DBPShopCreateArg, new DBPShopCreateRes);
static DBPShopBuy __stub_DBPShopBuy (RPC_DBPSHOPBUY, new DBPShopBuyArg, new DBPShopBuyRes);
static DBPShopSell __stub_DBPShopSell (RPC_DBPSHOPSELL, new DBPShopSellArg, new DBPShopSellRes);
static DBPShopCancelGoods __stub_DBPShopCancelGoods (RPC_DBPSHOPCANCELGOODS, new DBPShopCancelGoodsArg, new DBPShopCancelGoodsRes);
static DBPShopPlayerBuy __stub_DBPShopPlayerBuy (RPC_DBPSHOPPLAYERBUY, new DBPShopPlayerBuyArg, new DBPShopPlayerBuyRes);
static DBPShopPlayerSell __stub_DBPShopPlayerSell (RPC_DBPSHOPPLAYERSELL, new DBPShopPlayerSellArg, new DBPShopPlayerSellRes);
static DBPShopSetType __stub_DBPShopSetType (RPC_DBPSHOPSETTYPE, new DBPShopSetTypeArg, new DBPShopSetTypeRes);
static DBPShopActive __stub_DBPShopActive (RPC_DBPSHOPACTIVE, new DBPShopActiveArg, new DBPShopActiveRes);
static DBPShopManageFund __stub_DBPShopManageFund (RPC_DBPSHOPMANAGEFUND, new DBPShopManageFundArg, new DBPShopManageFundRes);
static DBPShopDrawItem __stub_DBPShopDrawItem (RPC_DBPSHOPDRAWITEM, new DBPShopDrawItemArg, new DBPShopDrawItemRes);
static DBPShopLoad __stub_DBPShopLoad (RPC_DBPSHOPLOAD, new DBPShopLoadArg, new DBPShopLoadRes);
static DBPShopGet __stub_DBPShopGet (RPC_DBPSHOPGET, new DBPShopGetArg, new DBPShopGetRes);
static DBPShopClearGoods __stub_DBPShopClearGoods (RPC_DBPSHOPCLEARGOODS, new DBPShopClearGoodsArg, new DBPShopClearGoodsRes);
static DBPShopTimeout __stub_DBPShopTimeout (RPC_DBPSHOPTIMEOUT, new DBPShopTimeoutArg, new DBPShopTimeoutRes);
static DBPlayerGivePresent __stub_DBPlayerGivePresent (RPC_DBPLAYERGIVEPRESENT, new DBPlayerGivePresentArg, new DBPlayerGivePresentRes);
static DBPlayerAskForPresent __stub_DBPlayerAskForPresent (RPC_DBPLAYERASKFORPRESENT, new DBPlayerAskForPresentArg, new DBPlayerAskForPresentRes);
static DBSysMail3 __stub_DBSysMail3 (RPC_DBSYSMAIL3, new DBSysMail3Arg, new DBSysMail3Res);
static DBGetPlayerProfileData __stub_DBGetPlayerProfileData (RPC_DBGETPLAYERPROFILEDATA, new DBGetPlayerProfileDataArg, new DBGetPlayerProfileDataRes);
static DBPutPlayerProfileData __stub_DBPutPlayerProfileData (RPC_DBPUTPLAYERPROFILEDATA, new DBPutPlayerProfileDataArg, new DBPutPlayerProfileDataRes);
static DBTankBattleBonus __stub_DBTankBattleBonus (RPC_DBTANKBATTLEBONUS, new DBTankBattleBonusArg, new DBTankBattleBonusRes);
static DBFactionResourceBattleBonus __stub_DBFactionResourceBattleBonus (RPC_DBFACTIONRESOURCEBATTLEBONUS, new DBFactionResourceBattleBonusArg, new DBFactionResourceBattleBonusRes);
static DBCopyRole __stub_DBCopyRole (RPC_DBCOPYROLE, new DBCopyRoleArg, new RpcRetcode);
static DBFactionRename __stub_DBFactionRename (RPC_DBFACTIONRENAME, new DBFactionRenameArg, new DBFactionRenameRes);
static DelRoleAnnounce __stub_DelRoleAnnounce((void*)0);
static DBFriendExtList_Re __stub_DBFriendExtList_Re((void*)0);
static TransBuyPoint __stub_TransBuyPoint((void*)0);
static SyncSellInfo __stub_SyncSellInfo((void*)0);
static AddCash __stub_AddCash((void*)0);
static AddCash_Re __stub_AddCash_Re((void*)0);
static DomainCmd_Re __stub_DomainCmd_Re((void*)0);
static DebugAddCash __stub_DebugAddCash((void*)0);

};
