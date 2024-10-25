#ifdef WIN32
#include <winsock2.h>
#include "gncompress.h"
#else
#include "binder.h"
#endif
#include "gquerypasswd.hrp"
#include "userlogin.hrp"
#include "userlogin2.hrp"
#include "cashserial.hrp"
#include "getaddcashsn.hrp"
#include "matrixpasswd.hrp"
#include "matrixpasswd2.hrp"
#include "matrixtoken.hrp"
#include "putspouse.hrp"
#include "queryuserid.hrp"
#include "forbiduser.hrp"
#include "playerpositionresetrqst.hrp"
#include "dbfactionrename.hrp"
#include "dbsellpoint.hrp"
#include "dbbuypoint.hrp"
#include "dbgametalkrolelist.hrp"
#include "dbgametalkrolerelation.hrp"
#include "dbgametalkfactioninfo.hrp"
#include "dbgametalkrolestatus.hrp"
#include "dbgametalkroleinfo.hrp"
#include "playeridentitymatch.hrp"
#include "addfriendrqst.hrp"
#include "tradestartrqst.hrp"
#include "gmqueryroleinfo.hrp"
#include "getmaxonlinenum.hrp"
#include "gmgetgameattri.hrp"
#include "gmsetgameattri.hrp"
#include "cashmoneyexchangecontrol.hrp"
#include "serverforbidcontrol.hrp"
#include "userlogout.hrp"
#include "getusercoupon.hrp"
#include "couponexchange.hrp"
#include "instantaddcash.hrp"
#include "battleend.hrp"
#include "sysauctionprepareitem.hrp"
#include "getfactionfortress.hrp"
#include "putfactionfortress.hrp"
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
#include "dbstockload.hrp"
#include "dbstocktransaction.hrp"
#include "dbstockbalance.hrp"
#include "dbstockcommission.hrp"
#include "dbstockcancel.hrp"
#include "dbsetcashpassword.hrp"
#include "dbautolockset.hrp"
#include "dbautolockget.hrp"
#include "dbautolocksetoffline.hrp"
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
#include "dbforceload.hrp"
#include "dbputforce.hrp"
#include "dbcountrybattlebonus.hrp"
#include "fetchplayerdata.hrp"
#include "activateplayerdata.hrp"
#include "delplayerdata.hrp"
#include "saveplayerdata.hrp"
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
#include "prefactionrename.hrp"
#include "accountaddrole.hrp"
#include "accountdelrole.hrp"
#include "announcefactionroledel.hrp"
#include "queryuserprivilege.hpp"
#include "queryuserforbid.hpp"
#include "playerkickout.hpp"
#include "playeroffline.hpp"
#include "gettaskdata_re.hpp"
#include "settaskdata_re.hpp"
#include "playerstatusannounce.hpp"
#include "onlineannounce.hpp"
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
#include "gmcontrolgame.hpp"
#include "gmcontrolgame_re.hpp"
#include "facemodify_re.hpp"
#include "accountingrequest.hpp"
#include "chatbroadcast.hpp"
#include "worldchat.hpp"
#include "factionchat.hpp"
#include "chatmulticast.hpp"
#include "chatsinglecast.hpp"
#include "rolestatusannounce.hpp"
#include "addfriend_re.hpp"
#include "addfriendremarks_re.hpp"
#include "getfriends_re.hpp"
#include "setgroupname_re.hpp"
#include "setfriendgroup_re.hpp"
#include "getsavedmsg_re.hpp"
#include "chatroomcreate_re.hpp"
#include "chatroominvite_re.hpp"
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
#include "gtradestart.hpp"
#include "gtradeend.hpp"
#include "playerrename_re.hpp"
#include "notifyfactionplayerrename.hpp"
#include "postplayerrename.hpp"
#include "playernameupdate.hpp"
#include "playergivepresent_re.hpp"
#include "playeraskforpresent_re.hpp"
#include "announcegm.hpp"
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
#include "verifymaster.hpp"
#include "verifymaster_re.hpp"
#include "gmsettimelessautolock_re.hpp"
#include "iwebautolockget_re.hpp"
#include "iwebautolockset_re.hpp"
#include "acwhoami.hpp"
#include "acstatusannounce.hpp"
#include "acstatusannounce2.hpp"
#include "acaccuse.hpp"
#include "playeraccuse_re.hpp"
#include "announcenewmail.hpp"
#include "getmaillist_re.hpp"
#include "getmail_re.hpp"
#include "getmailattachobj_re.hpp"
#include "deletemail_re.hpp"
#include "preservemail_re.hpp"
#include "playersendmail_re.hpp"
#include "gmailendsync.hpp"
#include "syssendmail_re.hpp"
#include "syssendmail3_re.hpp"
#include "sysrecoveredobjmail_re.hpp"
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
#include "battlestart_re.hpp"
#include "battleenternotice.hpp"
#include "battlemapnotice.hpp"
#include "battlestart.hpp"
#include "battlefactionnotice.hpp"
#include "queryrewardtype.hpp"
#include "queryrewardtype_re.hpp"
#include "querygameserverattr_re.hpp"
#include "announceserverattribute.hpp"
#include "announcechallengealgo.hpp"
#include "announceauthdversion.hpp"
#include "getselllist_re.hpp"
#include "findsellpointinfo_re.hpp"
#include "announcesellresult.hpp"
#include "sellcancel_re.hpp"
#include "buypoint_re.hpp"
#include "sellpoint_re.hpp"
#include "announcezoneid.hpp"
#include "announcezoneid2.hpp"
#include "announcezoneid3.hpp"
#include "stockcommission_re.hpp"
#include "stockaccount_re.hpp"
#include "stocktransaction_re.hpp"
#include "stockbill_re.hpp"
#include "stockcancel_re.hpp"
#include "cashlock_re.hpp"
#include "cashpasswordset_re.hpp"
#include "ondivorce.hpp"
#include "autolockset_re.hpp"
#include "forwardchat.hpp"
#include "disableautolock.hpp"
#include "acforbidcheater.hpp"
#include "autolockchanged.hpp"
#include "sendrefaddbonus.hpp"
#include "sendrewardaddbonus.hpp"
#include "refwithdrawbonus_re.hpp"
#include "reflistreferrals_re.hpp"
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
#include "post.hpp"
#include "gamepostcancel.hpp"
#include "webpostcancel_re.hpp"
#include "shelf_re.hpp"
#include "shelfcancel_re.hpp"
#include "sold_re.hpp"
#include "postexpire_re.hpp"
#include "webgetrolelist_re.hpp"
#include "newkeepalive.hpp"
#include "sysauctionlist_re.hpp"
#include "sysauctiongetitem_re.hpp"
#include "sysauctionaccount_re.hpp"
#include "sysauctionbid_re.hpp"
#include "sysauctioncashtransfer_re.hpp"
#include "createfactionfortress_re.hpp"
#include "notifyfactionfortressinfo2.hpp"
#include "factionfortressenternotice.hpp"
#include "factionfortresslist_re.hpp"
#include "factionfortresschallenge_re.hpp"
#include "factionfortressbattlelist_re.hpp"
#include "factionfortressget_re.hpp"
#include "notifyfactionfortressid.hpp"
#include "announcezoneidtoim.hpp"
#include "gamesysmsg.hpp"
#include "gamedatareq.hpp"
#include "gamedataresp.hpp"
#include "imkeepalive.hpp"
#include "announceresp.hpp"
#include "rolelistreq.hpp"
#include "rolelistresp.hpp"
#include "rolerelationreq.hpp"
#include "rolerelationresp.hpp"
#include "rolestatusreq.hpp"
#include "rolestatusresp.hpp"
#include "roleinforeq.hpp"
#include "roleinforesp.hpp"
#include "rolestatusupdate.hpp"
#include "rolegroupupdate.hpp"
#include "rolefriendupdate.hpp"
#include "roleblacklistupdate.hpp"
#include "rolemsg.hpp"
#include "roleofflinemessages.hpp"
#include "roleactivation.hpp"
#include "removerole.hpp"
#include "roleinfoupdate.hpp"
#include "roleforbidupdate.hpp"
#include "factioninforeq.hpp"
#include "factioninforesp.hpp"
#include "factionmemberupdate.hpp"
#include "factioninfoupdate.hpp"
#include "factionmsg.hpp"
#include "removefaction.hpp"
#include "factionforbidupdate.hpp"
#include "syncteams.hpp"
#include "teamcreate.hpp"
#include "teamdismiss.hpp"
#include "teammemberupdate.hpp"
#include "roleentervoicechannel.hpp"
#include "roleentervoicechannelack.hpp"
#include "roleleavevoicechannel.hpp"
#include "roleleavevoicechannelack.hpp"
#include "usercoupon_re.hpp"
#include "usercouponexchange_re.hpp"
#include "addcashnotify.hpp"
#include "useraddcash_re.hpp"
#include "ssogetticket_re.hpp"
#include "ssogetticketreq.hpp"
#include "syncforceglobaldata.hpp"
#include "aumailsended.hpp"
#include "game2au.hpp"
#include "dbfriendextlist.hpp"
#include "countrybattleapply_re.hpp"
#include "countrybattleconfignotify.hpp"
#include "countrybattlemove_re.hpp"
#include "countrybattlesyncplayerlocation.hpp"
#include "countrybattlestart.hpp"
#include "countrybattleenter.hpp"
#include "countrybattlegetmap_re.hpp"
#include "countrybattlesyncplayerpos.hpp"
#include "countrybattlegetconfig_re.hpp"
#include "countrybattlegetscore_re.hpp"
#include "countrybattlepreenternotify.hpp"
#include "countrybattleresult.hpp"
#include "countrybattlesinglebattleresult.hpp"
#include "countrybattlekingassignassault_re.hpp"
#include "countrybattlegetbattlelimit_re.hpp"
#include "countrybattlegetkingcommandpoint_re.hpp"
#include "getcnetserverconfig_re.hpp"
#include "countrybattledestroyinstance.hpp"
#include "qpannouncediscount.hpp"
#include "qpgetactivatedservices_re.hpp"
#include "qpaddcash_re.hpp"
#include "playerenterleavegt.hpp"
#include "playerchangeds.hpp"
#include "changeds_re.hpp"
#include "announcecentraldelivery.hpp"
#include "kickoutuser2.hpp"
#include "cashmoneyexchangenotify.hpp"
#include "serverforbidnotify.hpp"
#include "servertriggernotify.hpp"
#include "kegetstatus_re.hpp"
#include "kecandidateapply_re.hpp"
#include "kevoting_re.hpp"
#include "kekingnotify.hpp"
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
#include "touchpointquery_re.hpp"
#include "touchpointcost_re.hpp"
#include "auaddupmoneyquery_re.hpp"
#include "giftcoderedeem_re.hpp"
#include "uniquedatamodifynotice.hpp"
#include "uniquedatasynch.hpp"
#include "playerprofilegetprofiledata_re.hpp"
#include "playerprofilegetmatchresult_re.hpp"
#include "uniquedatamodifybroadcast.hpp"
#include "tankbattleenter.hpp"
#include "tankbattlestart.hpp"
#include "tankbattleplayerapply_re.hpp"
#include "tankbattleplayergetrank_re.hpp"
#include "autoteamsetgoal_re.hpp"
#include "autoteamplayerready.hpp"
#include "autoteamcomposestart.hpp"
#include "autoteamcomposefailed.hpp"
#include "autoteamplayerleave.hpp"
#include "factionresourcebattlerequestconfig.hpp"
#include "factionresourcebattlestatusnotice.hpp"
#include "factionresourcebattleplayerqueryresult.hpp"
#include "factionresourcebattlelimitnotice.hpp"
#include "factionresourcebattlegetmap_re.hpp"
#include "factionresourcebattlegetrecord_re.hpp"
#include "factionresourcebattlenotifyplayerevent.hpp"
#include "playerlogin.hpp"
#include "playerstatussync.hpp"
#include "enterworld.hpp"
#include "statusannounce.hpp"
#include "rolelist.hpp"
#include "createrole.hpp"
#include "deleterole.hpp"
#include "undodeleterole.hpp"
#include "playerbaseinfo.hpp"
#include "playerbaseinfocrc.hpp"
#include "getplayeridbyname.hpp"
#include "setcustomdata.hpp"
#include "getcustomdata.hpp"
#include "setuiconfig.hpp"
#include "getuiconfig.hpp"
#include "sethelpstates.hpp"
#include "gethelpstates.hpp"
#include "getplayerbriefinfo.hpp"
#include "gmgetplayerconsumeinfo.hpp"
#include "collectclientmachineinfo.hpp"
#include "publicchat.hpp"
#include "privatechat.hpp"
#include "addfriend.hpp"
#include "addfriendremarks.hpp"
#include "getfriends.hpp"
#include "setgroupname.hpp"
#include "setfriendgroup.hpp"
#include "delfriend.hpp"
#include "delfriend_re.hpp"
#include "friendstatus.hpp"
#include "getsavedmsg.hpp"
#include "chatroomcreate.hpp"
#include "chatroominvite.hpp"
#include "chatroomjoin.hpp"
#include "chatroomleave.hpp"
#include "chatroomexpel.hpp"
#include "chatroomspeak.hpp"
#include "chatroomlist.hpp"
#include "sendaumail.hpp"
#include "tradestart.hpp"
#include "tradeaddgoods.hpp"
#include "traderemovegoods.hpp"
#include "tradesubmit.hpp"
#include "trademoveobj.hpp"
#include "tradeconfirm.hpp"
#include "tradediscard.hpp"
#include "switchservercancel.hpp"
#include "gmrestartserver.hpp"
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
#include "gmprivilegechange.hpp"
#include "report2gm.hpp"
#include "complain2gm.hpp"
#include "announcelinktype.hpp"
#include "setmaxonlinenum.hpp"
#include "gmsettimelessautolock.hpp"
#include "iwebautolockget.hpp"
#include "iwebautolockset.hpp"
#include "acreport.hpp"
#include "acanswer.hpp"
#include "acprotostat.hpp"
#include "reportip.hpp"
#include "checknewmail.hpp"
#include "syssendmail.hpp"
#include "sysrecoveredobjmail.hpp"
#include "getselllist.hpp"
#include "findsellpointinfo.hpp"
#include "sellcancel.hpp"
#include "battlegetmap.hpp"
#include "battlestatus.hpp"
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
#include "cashlock.hpp"
#include "cashpasswordset.hpp"
#include "matrixfailure.hpp"
#include "autolockset.hpp"
#include "refwithdrawbonus.hpp"
#include "reflistreferrals.hpp"
#include "refgetreferencecode.hpp"
#include "exchangeconsumepoints.hpp"
#include "getrewardlist.hpp"
#include "webtraderoleprepost.hpp"
#include "webtraderoleprecancelpost.hpp"
#include "webtraderolegetdetail.hpp"
#include "usercoupon.hpp"
#include "usercouponexchange.hpp"
#include "useraddcash.hpp"
#include "ssogetticket.hpp"
#include "qpgetactivatedservices.hpp"
#include "qpaddcash.hpp"
#include "reportchat.hpp"
#include "playeraccuse.hpp"
#include "announcelinkversion.hpp"
#include "pshopplayerget.hpp"
#include "pshoplist.hpp"
#include "pshoplistitem.hpp"
#include "playerprofilegetprofiledata.hpp"
#include "playerprofilesetprofiledata.hpp"
#include "playerprofilegetmatchresult.hpp"
#include "tankbattleplayergetrank.hpp"
#include "factionresourcebattlegetmap.hpp"
#include "factionresourcebattlegetrecord.hpp"
#include "keyexchange.hpp"
#include "kickoutuser.hpp"
#include "accountingresponse.hpp"
#include "queryuserprivilege_re.hpp"
#include "queryuserforbid_re.hpp"
#include "updateremaintime.hpp"
#include "transbuypoint_re.hpp"
#include "addcash.hpp"
#include "addcash_re.hpp"
#include "syssendmail3.hpp"
#include "addictioncontrol.hpp"
#include "billingrequest.hpp"
#include "billingbalance.hpp"
#include "billingbalancesa_re.hpp"
#include "billingconfirm_re.hpp"
#include "netbarannounce.hpp"
#include "authdversion.hpp"
#include "ssogetticketrep.hpp"
#include "au2game.hpp"
#include "discountannounce.hpp"
#include "announceproviderid.hpp"
#include "playerlogin_re.hpp"
#include "playerkickout_re.hpp"
#include "playerlogout.hpp"
#include "playeroffline_re.hpp"
#include "queryplayerstatus.hpp"
#include "gettaskdata.hpp"
#include "settaskdata.hpp"
#include "playerinfoupdate.hpp"
#include "playerteamop.hpp"
#include "playerteammemberop.hpp"
#include "gtradestart_re.hpp"
#include "gtradediscard.hpp"
#include "keepalive.hpp"
#include "disconnectplayer.hpp"
#include "switchserverstart.hpp"
#include "switchserversuccess.hpp"
#include "switchservertimeout.hpp"
#include "setchatemotion.hpp"
#include "facemodify.hpp"
#include "facemodifycancel.hpp"
#include "gmrestartserver_re.hpp"
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
#include "querygameserverattr.hpp"
#include "acreportcheater.hpp"
#include "actriggerquestion.hpp"
#include "battlechallenge.hpp"
#include "battlechallengemap.hpp"
#include "battleenter.hpp"
#include "battleserverregister.hpp"
#include "debugcommand.hpp"
#include "stockcommission.hpp"
#include "stockaccount.hpp"
#include "stocktransaction.hpp"
#include "stockbill.hpp"
#include "stockcancel.hpp"
#include "billingbalancesa.hpp"
#include "billingconfirm.hpp"
#include "billingcancel.hpp"
#include "sendrefcashused.hpp"
#include "sendtaskreward.hpp"
#include "refgetreferencecode_re.hpp"
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
#include "synmutadata.hpp"
#include "countrybattleapply.hpp"
#include "countrybattlejoinnotice.hpp"
#include "countrybattleleavenotice.hpp"
#include "countrybattleonlinenotice.hpp"
#include "countrybattleofflinenotice.hpp"
#include "countrybattleentermapnotice.hpp"
#include "countrybattleserverregister.hpp"
#include "countrybattlestart_re.hpp"
#include "countrybattleend.hpp"
#include "playerchangeds_re.hpp"
#include "trychangeds.hpp"
#include "playerrename.hpp"
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
#include "factionresourcebattlerequestconfig_re.hpp"
#include "factionresourcebattleserverregister.hpp"
#include "factionresourcebattleeventnotice.hpp"
#include "factionresourcebattleplayerquery.hpp"
#include "delroleannounce.hpp"
#include "dbfriendextlist_re.hpp"
#include "transbuypoint.hpp"
#include "syncsellinfo.hpp"
#include "debugaddcash.hpp"
#include "delfactionannounce.hpp"
#include "createfactionfortress.hpp"
#include "acremotecode.hpp"
#include "acquestion.hpp"
#include "acaccusere.hpp"
#include "post_re.hpp"
#include "gamepostcancel_re.hpp"
#include "webpostcancel.hpp"
#include "shelf.hpp"
#include "shelfcancel.hpp"
#include "sold.hpp"
#include "postexpire.hpp"
#include "webgetrolelist.hpp"
#include "loadexchange.hpp"
#include "dsannounceidentity.hpp"
#include "senddataandidentity.hpp"
#include "senddataandidentity_re.hpp"
#include "remoteloginquery.hpp"
#include "remotelogout.hpp"
#include "kickoutremoteuser_re.hpp"
#include "getremoteroleinfo_re.hpp"
#include "getremotecnetserverconfig_re.hpp"
#include "remoteloginquery_re.hpp"
#include "kickoutremoteuser.hpp"
#include "getremoteroleinfo.hpp"
#include "getremotecnetserverconfig.hpp"

namespace GNET
{

static GQueryPasswd __stub_GQueryPasswd (RPC_GQUERYPASSWD, new GQueryPasswdArg, new GQueryPasswdRes);
static UserLogin __stub_UserLogin (RPC_USERLOGIN, new UserLoginArg, new UserLoginRes);
static UserLogin2 __stub_UserLogin2 (RPC_USERLOGIN2, new UserLogin2Arg, new UserLogin2Res);
static CashSerial __stub_CashSerial (RPC_CASHSERIAL, new CashSerialArg, new CashSerialRes);
static GetAddCashSN __stub_GetAddCashSN (RPC_GETADDCASHSN, new GetAddCashSNArg, new GetAddCashSNRes);
static MatrixPasswd __stub_MatrixPasswd (RPC_MATRIXPASSWD, new MatrixPasswdArg, new MatrixPasswdRes);
static MatrixPasswd2 __stub_MatrixPasswd2 (RPC_MATRIXPASSWD2, new MatrixPasswdArg, new MatrixPasswd2Res);
static MatrixToken __stub_MatrixToken (RPC_MATRIXTOKEN, new MatrixTokenArg, new MatrixTokenRes);
static PutSpouse __stub_PutSpouse (RPC_PUTSPOUSE, new PutSpouseArg, new RpcRetcode);
static QueryUserid __stub_QueryUserid (RPC_QUERYUSERID, new QueryUseridArg, new QueryUseridRes);
static ForbidUser __stub_ForbidUser (RPC_FORBIDUSER, new ForbidUserArg, new ForbidUserRes);
static PlayerPositionResetRqst __stub_PlayerPositionResetRqst (RPC_PLAYERPOSITIONRESETRQST, new PlayerPositionResetRqstArg, new PlayerPositionResetRqstRes);
static DBFactionRename __stub_DBFactionRename (RPC_DBFACTIONRENAME, new DBFactionRenameArg, new DBFactionRenameRes);
static DBSellPoint __stub_DBSellPoint (RPC_DBSELLPOINT, new SellPointArg, new SellPointRes);
static DBBuyPoint __stub_DBBuyPoint (RPC_DBBUYPOINT, new DBBuyPointArg, new DBBuyPointRes);
static DBGameTalkRoleList __stub_DBGameTalkRoleList (RPC_DBGAMETALKROLELIST, new DBGameTalkRoleListArg, new DBGameTalkRoleListRes);
static DBGameTalkRoleRelation __stub_DBGameTalkRoleRelation (RPC_DBGAMETALKROLERELATION, new DBGameTalkRoleRelationArg, new DBGameTalkRoleRelationRes);
static DBGameTalkFactionInfo __stub_DBGameTalkFactionInfo (RPC_DBGAMETALKFACTIONINFO, new DBGameTalkFactionInfoArg, new DBGameTalkFactionInfoRes);
static DBGameTalkRoleStatus __stub_DBGameTalkRoleStatus (RPC_DBGAMETALKROLESTATUS, new DBGameTalkRoleStatusArg, new DBGameTalkRoleStatusRes);
static DBGameTalkRoleInfo __stub_DBGameTalkRoleInfo (RPC_DBGAMETALKROLEINFO, new DBGameTalkRoleInfoArg, new DBGameTalkRoleInfoRes);
static PlayerIdentityMatch __stub_PlayerIdentityMatch (RPC_PLAYERIDENTITYMATCH, new PlayerIdentityMatchArg, new PlayerIdentityMatchRes);
static AddFriendRqst __stub_AddFriendRqst (RPC_ADDFRIENDRQST, new AddFriendRqstArg, new AddFriendRqstRes);
static TradeStartRqst __stub_TradeStartRqst (RPC_TRADESTARTRQST, new TradeStartRqstArg, new TradeStartRqstRes);
static GMQueryRoleInfo __stub_GMQueryRoleInfo (RPC_GMQUERYROLEINFO, new RoleId, new GMQueryRoleInfoRes);
static GetMaxOnlineNum __stub_GetMaxOnlineNum (RPC_GETMAXONLINENUM, new GetMaxOnlineNumArg, new GetMaxOnlineNumRes);
static GMGetGameAttri __stub_GMGetGameAttri (RPC_GMGETGAMEATTRI, new GMGetGameAttriArg, new GMGetGameAttriRes);
static GMSetGameAttri __stub_GMSetGameAttri (RPC_GMSETGAMEATTRI, new GMSetGameAttriArg, new GMSetGameAttriRes);
static CashMoneyExchangeControl __stub_CashMoneyExchangeControl (RPC_CASHMONEYEXCHANGECONTROL, new CashMoneyExchangeControlArg, new CashMoneyExchangeControlRes);
static ServerForbidControl __stub_ServerForbidControl (RPC_SERVERFORBIDCONTROL, new ServerForbidControlArg, new ServerForbidControlRes);
static UserLogout __stub_UserLogout (RPC_USERLOGOUT, new UserLogoutArg, new UserLogoutRes);
static GetUserCoupon __stub_GetUserCoupon (RPC_GETUSERCOUPON, new GetUserCouponArg, new GetUserCouponRes);
static CouponExchange __stub_CouponExchange (RPC_COUPONEXCHANGE, new CouponExchangeArg, new CouponExchangeRes);
static InstantAddCash __stub_InstantAddCash (RPC_INSTANTADDCASH, new InstantAddCashArg, new InstantAddCashRes);
static BattleEnd __stub_BattleEnd (RPC_BATTLEEND, new BattleEndArg, new BattleEndRes);
static SysAuctionPrepareItem __stub_SysAuctionPrepareItem (RPC_SYSAUCTIONPREPAREITEM, new SysAuctionPrepareItemArg, new SysAuctionPrepareItemRes);
static GetFactionFortress __stub_GetFactionFortress (RPC_GETFACTIONFORTRESS, new GetFactionFortressArg, new GetFactionFortressRes);
static PutFactionFortress __stub_PutFactionFortress (RPC_PUTFACTIONFORTRESS, new PutFactionFortressArg, new PutFactionFortressRes);
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
static DBStockLoad __stub_DBStockLoad (RPC_DBSTOCKLOAD, new DBStockLoadArg, new DBStockLoadRes);
static DBStockTransaction __stub_DBStockTransaction (RPC_DBSTOCKTRANSACTION, new DBStockTransactionArg, new DBStockTransactionRes);
static DBStockBalance __stub_DBStockBalance (RPC_DBSTOCKBALANCE, new DBStockBalanceArg, new DBStockBalanceRes);
static DBStockCommission __stub_DBStockCommission (RPC_DBSTOCKCOMMISSION, new DBStockCommissionArg, new DBStockCommissionRes);
static DBStockCancel __stub_DBStockCancel (RPC_DBSTOCKCANCEL, new DBStockCancelArg, new DBStockCancelRes);
static DBSetCashPassword __stub_DBSetCashPassword (RPC_DBSETCASHPASSWORD, new DBSetCashPasswordArg, new DBSetCashPasswordRes);
static DBAutolockSet __stub_DBAutolockSet (RPC_DBAUTOLOCKSET, new DBAutolockSetArg, new Integer);
static DBAutolockGet __stub_DBAutolockGet (RPC_DBAUTOLOCKGET, new DBAutolockGetArg, new DBAutolockGetRes);
static DBAutolockSetOffline __stub_DBAutolockSetOffline (RPC_DBAUTOLOCKSETOFFLINE, new DBAutolockSetOfflineArg, new DBAutolockSetOfflineRes);
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
static DBForceLoad __stub_DBForceLoad (RPC_DBFORCELOAD, new DBForceLoadArg, new DBForceLoadRes);
static DBPutForce __stub_DBPutForce (RPC_DBPUTFORCE, new DBPutForceArg, new DBPutForceRes);
static DBCountryBattleBonus __stub_DBCountryBattleBonus (RPC_DBCOUNTRYBATTLEBONUS, new DBCountryBattleBonusArg, new DBCountryBattleBonusRes);
static FetchPlayerData __stub_FetchPlayerData (RPC_FETCHPLAYERDATA, new FetchPlayerDataArg, new FetchPlayerDataRes);
static ActivatePlayerData __stub_ActivatePlayerData (RPC_ACTIVATEPLAYERDATA, new ActivatePlayerDataArg, new ActivatePlayerDataRes);
static DelPlayerData __stub_DelPlayerData (RPC_DELPLAYERDATA, new DelPlayerDataArg, new DelPlayerDataRes);
static SavePlayerData __stub_SavePlayerData (RPC_SAVEPLAYERDATA, new SavePlayerDataArg, new SavePlayerDataRes);
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
static PreFactionRename __stub_PreFactionRename (RPC_PREFACTIONRENAME, new PreFactionRenameArg, new PreFactionRenameRes);
static AccountAddRole __stub_AccountAddRole (RPC_ACCOUNTADDROLE, new AccountAddRoleArg, new AccountAddRoleRes);
static AccountDelRole __stub_AccountDelRole (RPC_ACCOUNTDELROLE, new AccountDelRoleArg, new AccountDelRoleRes);
static AnnounceFactionRoleDel __stub_AnnounceFactionRoleDel (RPC_ANNOUNCEFACTIONROLEDEL, new AnnounceFactionRoleDelArg, new AnnounceFactionRoleDelRes);
static QueryUserPrivilege __stub_QueryUserPrivilege((void*)0);
static QueryUserForbid __stub_QueryUserForbid((void*)0);
static PlayerKickout __stub_PlayerKickout((void*)0);
static PlayerOffline __stub_PlayerOffline((void*)0);
static GetTaskData_Re __stub_GetTaskData_Re((void*)0);
static SetTaskData_Re __stub_SetTaskData_Re((void*)0);
static PlayerStatusAnnounce __stub_PlayerStatusAnnounce((void*)0);
static OnlineAnnounce __stub_OnlineAnnounce((void*)0);
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
static GMControlGame __stub_GMControlGame((void*)0);
static GMControlGame_Re __stub_GMControlGame_Re((void*)0);
static FaceModify_Re __stub_FaceModify_Re((void*)0);
static AccountingRequest __stub_AccountingRequest((void*)0);
static ChatBroadCast __stub_ChatBroadCast((void*)0);
static WorldChat __stub_WorldChat((void*)0);
static FactionChat __stub_FactionChat((void*)0);
static ChatMultiCast __stub_ChatMultiCast((void*)0);
static ChatSingleCast __stub_ChatSingleCast((void*)0);
static RoleStatusAnnounce __stub_RoleStatusAnnounce((void*)0);
static AddFriend_Re __stub_AddFriend_Re((void*)0);
static AddFriendRemarks_Re __stub_AddFriendRemarks_Re((void*)0);
static GetFriends_Re __stub_GetFriends_Re((void*)0);
static SetGroupName_Re __stub_SetGroupName_Re((void*)0);
static SetFriendGroup_Re __stub_SetFriendGroup_Re((void*)0);
static GetSavedMsg_Re __stub_GetSavedMsg_Re((void*)0);
static ChatRoomCreate_Re __stub_ChatRoomCreate_Re((void*)0);
static ChatRoomInvite_Re __stub_ChatRoomInvite_Re((void*)0);
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
static GTradeStart __stub_GTradeStart((void*)0);
static GTradeEnd __stub_GTradeEnd((void*)0);
static PlayerRename_Re __stub_PlayerRename_Re((void*)0);
static NotifyFactionPlayerRename __stub_NotifyFactionPlayerRename((void*)0);
static PostPlayerRename __stub_PostPlayerRename((void*)0);
static PlayerNameUpdate __stub_PlayerNameUpdate((void*)0);
static PlayerGivePresent_Re __stub_PlayerGivePresent_Re((void*)0);
static PlayerAskForPresent_Re __stub_PlayerAskForPresent_Re((void*)0);
static AnnounceGM __stub_AnnounceGM((void*)0);
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
static VerifyMaster __stub_VerifyMaster((void*)0);
static VerifyMaster_Re __stub_VerifyMaster_Re((void*)0);
static GMSetTimelessAutoLock_Re __stub_GMSetTimelessAutoLock_Re((void*)0);
static IWebAutolockGet_Re __stub_IWebAutolockGet_Re((void*)0);
static IWebAutolockSet_Re __stub_IWebAutolockSet_Re((void*)0);
static ACWhoAmI __stub_ACWhoAmI((void*)0);
static ACStatusAnnounce __stub_ACStatusAnnounce((void*)0);
static ACStatusAnnounce2 __stub_ACStatusAnnounce2((void*)0);
static ACAccuse __stub_ACAccuse((void*)0);
static PlayerAccuse_Re __stub_PlayerAccuse_Re((void*)0);
static AnnounceNewMail __stub_AnnounceNewMail((void*)0);
static GetMailList_Re __stub_GetMailList_Re((void*)0);
static GetMail_Re __stub_GetMail_Re((void*)0);
static GetMailAttachObj_Re __stub_GetMailAttachObj_Re((void*)0);
static DeleteMail_Re __stub_DeleteMail_Re((void*)0);
static PreserveMail_Re __stub_PreserveMail_Re((void*)0);
static PlayerSendMail_Re __stub_PlayerSendMail_Re((void*)0);
static GMailEndSync __stub_GMailEndSync((void*)0);
static SysSendMail_Re __stub_SysSendMail_Re((void*)0);
static SysSendMail3_Re __stub_SysSendMail3_Re((void*)0);
static SysRecoveredObjMail_Re __stub_SysRecoveredObjMail_Re((void*)0);
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
static BattleStart_Re __stub_BattleStart_Re((void*)0);
static BattleEnterNotice __stub_BattleEnterNotice((void*)0);
static BattleMapNotice __stub_BattleMapNotice((void*)0);
static BattleStart __stub_BattleStart((void*)0);
static BattleFactionNotice __stub_BattleFactionNotice((void*)0);
static QueryRewardType __stub_QueryRewardType((void*)0);
static QueryRewardType_Re __stub_QueryRewardType_Re((void*)0);
static QueryGameServerAttr_Re __stub_QueryGameServerAttr_Re((void*)0);
static AnnounceServerAttribute __stub_AnnounceServerAttribute((void*)0);
static AnnounceChallengeAlgo __stub_AnnounceChallengeAlgo((void*)0);
static AnnounceAuthdVersion __stub_AnnounceAuthdVersion((void*)0);
static GetSellList_Re __stub_GetSellList_Re((void*)0);
static FindSellPointInfo_Re __stub_FindSellPointInfo_Re((void*)0);
static AnnounceSellResult __stub_AnnounceSellResult((void*)0);
static SellCancel_Re __stub_SellCancel_Re((void*)0);
static BuyPoint_Re __stub_BuyPoint_Re((void*)0);
static SellPoint_Re __stub_SellPoint_Re((void*)0);
static AnnounceZoneid __stub_AnnounceZoneid((void*)0);
static AnnounceZoneid2 __stub_AnnounceZoneid2((void*)0);
static AnnounceZoneid3 __stub_AnnounceZoneid3((void*)0);
static StockCommission_Re __stub_StockCommission_Re((void*)0);
static StockAccount_Re __stub_StockAccount_Re((void*)0);
static StockTransaction_Re __stub_StockTransaction_Re((void*)0);
static StockBill_Re __stub_StockBill_Re((void*)0);
static StockCancel_Re __stub_StockCancel_Re((void*)0);
static CashLock_Re __stub_CashLock_Re((void*)0);
static CashPasswordSet_Re __stub_CashPasswordSet_Re((void*)0);
static OnDivorce __stub_OnDivorce((void*)0);
static AutolockSet_Re __stub_AutolockSet_Re((void*)0);
static ForwardChat __stub_ForwardChat((void*)0);
static DisableAutolock __stub_DisableAutolock((void*)0);
static ACForbidCheater __stub_ACForbidCheater((void*)0);
static AutolockChanged __stub_AutolockChanged((void*)0);
static SendRefAddBonus __stub_SendRefAddBonus((void*)0);
static SendRewardAddBonus __stub_SendRewardAddBonus((void*)0);
static RefWithdrawBonus_Re __stub_RefWithdrawBonus_Re((void*)0);
static RefListReferrals_Re __stub_RefListReferrals_Re((void*)0);
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
static Post __stub_Post((void*)0);
static GamePostCancel __stub_GamePostCancel((void*)0);
static WebPostCancel_Re __stub_WebPostCancel_Re((void*)0);
static Shelf_Re __stub_Shelf_Re((void*)0);
static ShelfCancel_Re __stub_ShelfCancel_Re((void*)0);
static Sold_Re __stub_Sold_Re((void*)0);
static PostExpire_Re __stub_PostExpire_Re((void*)0);
static WebGetRoleList_Re __stub_WebGetRoleList_Re((void*)0);
static NewKeepAlive __stub_NewKeepAlive((void*)0);
static SysAuctionList_Re __stub_SysAuctionList_Re((void*)0);
static SysAuctionGetItem_Re __stub_SysAuctionGetItem_Re((void*)0);
static SysAuctionAccount_Re __stub_SysAuctionAccount_Re((void*)0);
static SysAuctionBid_Re __stub_SysAuctionBid_Re((void*)0);
static SysAuctionCashTransfer_Re __stub_SysAuctionCashTransfer_Re((void*)0);
static CreateFactionFortress_Re __stub_CreateFactionFortress_Re((void*)0);
static NotifyFactionFortressInfo2 __stub_NotifyFactionFortressInfo2((void*)0);
static FactionFortressEnterNotice __stub_FactionFortressEnterNotice((void*)0);
static FactionFortressList_Re __stub_FactionFortressList_Re((void*)0);
static FactionFortressChallenge_Re __stub_FactionFortressChallenge_Re((void*)0);
static FactionFortressBattleList_Re __stub_FactionFortressBattleList_Re((void*)0);
static FactionFortressGet_Re __stub_FactionFortressGet_Re((void*)0);
static NotifyFactionFortressID __stub_NotifyFactionFortressID((void*)0);
static AnnounceZoneidToIM __stub_AnnounceZoneidToIM((void*)0);
static GameSysMsg __stub_GameSysMsg((void*)0);
static GameDataReq __stub_GameDataReq((void*)0);
static GameDataResp __stub_GameDataResp((void*)0);
static IMKeepAlive __stub_IMKeepAlive((void*)0);
static AnnounceResp __stub_AnnounceResp((void*)0);
static RoleListReq __stub_RoleListReq((void*)0);
static RoleListResp __stub_RoleListResp((void*)0);
static RoleRelationReq __stub_RoleRelationReq((void*)0);
static RoleRelationResp __stub_RoleRelationResp((void*)0);
static RoleStatusReq __stub_RoleStatusReq((void*)0);
static RoleStatusResp __stub_RoleStatusResp((void*)0);
static RoleInfoReq __stub_RoleInfoReq((void*)0);
static RoleInfoResp __stub_RoleInfoResp((void*)0);
static RoleStatusUpdate __stub_RoleStatusUpdate((void*)0);
static RoleGroupUpdate __stub_RoleGroupUpdate((void*)0);
static RoleFriendUpdate __stub_RoleFriendUpdate((void*)0);
static RoleBlacklistUpdate __stub_RoleBlacklistUpdate((void*)0);
static RoleMsg __stub_RoleMsg((void*)0);
static RoleOfflineMessages __stub_RoleOfflineMessages((void*)0);
static RoleActivation __stub_RoleActivation((void*)0);
static RemoveRole __stub_RemoveRole((void*)0);
static RoleInfoUpdate __stub_RoleInfoUpdate((void*)0);
static RoleForbidUpdate __stub_RoleForbidUpdate((void*)0);
static FactionInfoReq __stub_FactionInfoReq((void*)0);
static FactionInfoResp __stub_FactionInfoResp((void*)0);
static FactionMemberUpdate __stub_FactionMemberUpdate((void*)0);
static FactionInfoUpdate __stub_FactionInfoUpdate((void*)0);
static FactionMsg __stub_FactionMsg((void*)0);
static RemoveFaction __stub_RemoveFaction((void*)0);
static FactionForbidUpdate __stub_FactionForbidUpdate((void*)0);
static SyncTeams __stub_SyncTeams((void*)0);
static TeamCreate __stub_TeamCreate((void*)0);
static TeamDismiss __stub_TeamDismiss((void*)0);
static TeamMemberUpdate __stub_TeamMemberUpdate((void*)0);
static RoleEnterVoiceChannel __stub_RoleEnterVoiceChannel((void*)0);
static RoleEnterVoiceChannelAck __stub_RoleEnterVoiceChannelAck((void*)0);
static RoleLeaveVoiceChannel __stub_RoleLeaveVoiceChannel((void*)0);
static RoleLeaveVoiceChannelAck __stub_RoleLeaveVoiceChannelAck((void*)0);
static UserCoupon_Re __stub_UserCoupon_Re((void*)0);
static UserCouponExchange_Re __stub_UserCouponExchange_Re((void*)0);
static AddCashNotify __stub_AddCashNotify((void*)0);
static UserAddCash_Re __stub_UserAddCash_Re((void*)0);
static SSOGetTicket_Re __stub_SSOGetTicket_Re((void*)0);
static SSOGetTicketReq __stub_SSOGetTicketReq((void*)0);
static SyncForceGlobalData __stub_SyncForceGlobalData((void*)0);
static AUMailSended __stub_AUMailSended((void*)0);
static Game2AU __stub_Game2AU((void*)0);
static DBFriendExtList __stub_DBFriendExtList((void*)0);
static CountryBattleApply_Re __stub_CountryBattleApply_Re((void*)0);
static CountryBattleConfigNotify __stub_CountryBattleConfigNotify((void*)0);
static CountryBattleMove_Re __stub_CountryBattleMove_Re((void*)0);
static CountryBattleSyncPlayerLocation __stub_CountryBattleSyncPlayerLocation((void*)0);
static CountryBattleStart __stub_CountryBattleStart((void*)0);
static CountryBattleEnter __stub_CountryBattleEnter((void*)0);
static CountryBattleGetMap_Re __stub_CountryBattleGetMap_Re((void*)0);
static CountryBattleSyncPlayerPos __stub_CountryBattleSyncPlayerPos((void*)0);
static CountryBattleGetConfig_Re __stub_CountryBattleGetConfig_Re((void*)0);
static CountryBattleGetScore_Re __stub_CountryBattleGetScore_Re((void*)0);
static CountryBattlePreEnterNotify __stub_CountryBattlePreEnterNotify((void*)0);
static CountryBattleResult __stub_CountryBattleResult((void*)0);
static CountryBattleSingleBattleResult __stub_CountryBattleSingleBattleResult((void*)0);
static CountryBattleKingAssignAssault_Re __stub_CountryBattleKingAssignAssault_Re((void*)0);
static CountryBattleGetBattleLimit_Re __stub_CountryBattleGetBattleLimit_Re((void*)0);
static CountryBattleGetKingCommandPoint_Re __stub_CountryBattleGetKingCommandPoint_Re((void*)0);
static GetCNetServerConfig_Re __stub_GetCNetServerConfig_Re((void*)0);
static CountryBattleDestroyInstance __stub_CountryBattleDestroyInstance((void*)0);
static QPAnnounceDiscount __stub_QPAnnounceDiscount((void*)0);
static QPGetActivatedServices_Re __stub_QPGetActivatedServices_Re((void*)0);
static QPAddCash_Re __stub_QPAddCash_Re((void*)0);
static PlayerEnterLeaveGT __stub_PlayerEnterLeaveGT((void*)0);
static PlayerChangeDS __stub_PlayerChangeDS((void*)0);
static ChangeDS_Re __stub_ChangeDS_Re((void*)0);
static AnnounceCentralDelivery __stub_AnnounceCentralDelivery((void*)0);
static KickoutUser2 __stub_KickoutUser2((void*)0);
static CashMoneyExchangeNotify __stub_CashMoneyExchangeNotify((void*)0);
static ServerForbidNotify __stub_ServerForbidNotify((void*)0);
static ServerTriggerNotify __stub_ServerTriggerNotify((void*)0);
static KEGetStatus_Re __stub_KEGetStatus_Re((void*)0);
static KECandidateApply_Re __stub_KECandidateApply_Re((void*)0);
static KEVoting_Re __stub_KEVoting_Re((void*)0);
static KEKingNotify __stub_KEKingNotify((void*)0);
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
static TouchPointQuery_Re __stub_TouchPointQuery_Re((void*)0);
static TouchPointCost_Re __stub_TouchPointCost_Re((void*)0);
static AuAddupMoneyQuery_Re __stub_AuAddupMoneyQuery_Re((void*)0);
static GiftCodeRedeem_Re __stub_GiftCodeRedeem_Re((void*)0);
static UniqueDataModifyNotice __stub_UniqueDataModifyNotice((void*)0);
static UniqueDataSynch __stub_UniqueDataSynch((void*)0);
static PlayerProfileGetProfileData_Re __stub_PlayerProfileGetProfileData_Re((void*)0);
static PlayerProfileGetMatchResult_Re __stub_PlayerProfileGetMatchResult_Re((void*)0);
static UniqueDataModifyBroadcast __stub_UniqueDataModifyBroadcast((void*)0);
static TankBattleEnter __stub_TankBattleEnter((void*)0);
static TankBattleStart __stub_TankBattleStart((void*)0);
static TankBattlePlayerApply_Re __stub_TankBattlePlayerApply_Re((void*)0);
static TankBattlePlayerGetRank_Re __stub_TankBattlePlayerGetRank_Re((void*)0);
static AutoTeamSetGoal_Re __stub_AutoTeamSetGoal_Re((void*)0);
static AutoTeamPlayerReady __stub_AutoTeamPlayerReady((void*)0);
static AutoTeamComposeStart __stub_AutoTeamComposeStart((void*)0);
static AutoTeamComposeFailed __stub_AutoTeamComposeFailed((void*)0);
static AutoTeamPlayerLeave __stub_AutoTeamPlayerLeave((void*)0);
static FactionResourceBattleRequestConfig __stub_FactionResourceBattleRequestConfig((void*)0);
static FactionResourceBattleStatusNotice __stub_FactionResourceBattleStatusNotice((void*)0);
static FactionResourceBattlePlayerQueryResult __stub_FactionResourceBattlePlayerQueryResult((void*)0);
static FactionResourceBattleLimitNotice __stub_FactionResourceBattleLimitNotice((void*)0);
static FactionResourceBattleGetMap_Re __stub_FactionResourceBattleGetMap_Re((void*)0);
static FactionResourceBattleGetRecord_Re __stub_FactionResourceBattleGetRecord_Re((void*)0);
static FactionResourceBattleNotifyPlayerEvent __stub_FactionResourceBattleNotifyPlayerEvent((void*)0);
static PlayerLogin __stub_PlayerLogin((void*)0);
static PlayerStatusSync __stub_PlayerStatusSync((void*)0);
static EnterWorld __stub_EnterWorld((void*)0);
static StatusAnnounce __stub_StatusAnnounce((void*)0);
static RoleList __stub_RoleList((void*)0);
static CreateRole __stub_CreateRole((void*)0);
static DeleteRole __stub_DeleteRole((void*)0);
static UndoDeleteRole __stub_UndoDeleteRole((void*)0);
static PlayerBaseInfo __stub_PlayerBaseInfo((void*)0);
static PlayerBaseInfoCRC __stub_PlayerBaseInfoCRC((void*)0);
static GetPlayerIDByName __stub_GetPlayerIDByName((void*)0);
static SetCustomData __stub_SetCustomData((void*)0);
static GetCustomData __stub_GetCustomData((void*)0);
static SetUIConfig __stub_SetUIConfig((void*)0);
static GetUIConfig __stub_GetUIConfig((void*)0);
static SetHelpStates __stub_SetHelpStates((void*)0);
static GetHelpStates __stub_GetHelpStates((void*)0);
static GetPlayerBriefInfo __stub_GetPlayerBriefInfo((void*)0);
static GMGetPlayerConsumeInfo __stub_GMGetPlayerConsumeInfo((void*)0);
static CollectClientMachineInfo __stub_CollectClientMachineInfo((void*)0);
static PublicChat __stub_PublicChat((void*)0);
static PrivateChat __stub_PrivateChat((void*)0);
static AddFriend __stub_AddFriend((void*)0);
static AddFriendRemarks __stub_AddFriendRemarks((void*)0);
static GetFriends __stub_GetFriends((void*)0);
static SetGroupName __stub_SetGroupName((void*)0);
static SetFriendGroup __stub_SetFriendGroup((void*)0);
static DelFriend __stub_DelFriend((void*)0);
static DelFriend_Re __stub_DelFriend_Re((void*)0);
static FriendStatus __stub_FriendStatus((void*)0);
static GetSavedMsg __stub_GetSavedMsg((void*)0);
static ChatRoomCreate __stub_ChatRoomCreate((void*)0);
static ChatRoomInvite __stub_ChatRoomInvite((void*)0);
static ChatRoomJoin __stub_ChatRoomJoin((void*)0);
static ChatRoomLeave __stub_ChatRoomLeave((void*)0);
static ChatRoomExpel __stub_ChatRoomExpel((void*)0);
static ChatRoomSpeak __stub_ChatRoomSpeak((void*)0);
static ChatRoomList __stub_ChatRoomList((void*)0);
static SendAUMail __stub_SendAUMail((void*)0);
static TradeStart __stub_TradeStart((void*)0);
static TradeAddGoods __stub_TradeAddGoods((void*)0);
static TradeRemoveGoods __stub_TradeRemoveGoods((void*)0);
static TradeSubmit __stub_TradeSubmit((void*)0);
static TradeMoveObj __stub_TradeMoveObj((void*)0);
static TradeConfirm __stub_TradeConfirm((void*)0);
static TradeDiscard __stub_TradeDiscard((void*)0);
static SwitchServerCancel __stub_SwitchServerCancel((void*)0);
static GMRestartServer __stub_GMRestartServer((void*)0);
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
static GMPrivilegeChange __stub_GMPrivilegeChange((void*)0);
static Report2GM __stub_Report2GM((void*)0);
static Complain2GM __stub_Complain2GM((void*)0);
static AnnounceLinkType __stub_AnnounceLinkType((void*)0);
static SetMaxOnlineNum __stub_SetMaxOnlineNum((void*)0);
static GMSetTimelessAutoLock __stub_GMSetTimelessAutoLock((void*)0);
static IWebAutolockGet __stub_IWebAutolockGet((void*)0);
static IWebAutolockSet __stub_IWebAutolockSet((void*)0);
static ACReport __stub_ACReport((void*)0);
static ACAnswer __stub_ACAnswer((void*)0);
static ACProtoStat __stub_ACProtoStat((void*)0);
static ReportIP __stub_ReportIP((void*)0);
static CheckNewMail __stub_CheckNewMail((void*)0);
static SysSendMail __stub_SysSendMail((void*)0);
static SysRecoveredObjMail __stub_SysRecoveredObjMail((void*)0);
static GetSellList __stub_GetSellList((void*)0);
static FindSellPointInfo __stub_FindSellPointInfo((void*)0);
static SellCancel __stub_SellCancel((void*)0);
static BattleGetMap __stub_BattleGetMap((void*)0);
static BattleStatus __stub_BattleStatus((void*)0);
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
static CashLock __stub_CashLock((void*)0);
static CashPasswordSet __stub_CashPasswordSet((void*)0);
static MatrixFailure __stub_MatrixFailure((void*)0);
static AutolockSet __stub_AutolockSet((void*)0);
static RefWithdrawBonus __stub_RefWithdrawBonus((void*)0);
static RefListReferrals __stub_RefListReferrals((void*)0);
static RefGetReferenceCode __stub_RefGetReferenceCode((void*)0);
static ExchangeConsumePoints __stub_ExchangeConsumePoints((void*)0);
static GetRewardList __stub_GetRewardList((void*)0);
static WebTradeRolePrePost __stub_WebTradeRolePrePost((void*)0);
static WebTradeRolePreCancelPost __stub_WebTradeRolePreCancelPost((void*)0);
static WebTradeRoleGetDetail __stub_WebTradeRoleGetDetail((void*)0);
static UserCoupon __stub_UserCoupon((void*)0);
static UserCouponExchange __stub_UserCouponExchange((void*)0);
static UserAddCash __stub_UserAddCash((void*)0);
static SSOGetTicket __stub_SSOGetTicket((void*)0);
static QPGetActivatedServices __stub_QPGetActivatedServices((void*)0);
static QPAddCash __stub_QPAddCash((void*)0);
static ReportChat __stub_ReportChat((void*)0);
static PlayerAccuse __stub_PlayerAccuse((void*)0);
static AnnounceLinkVersion __stub_AnnounceLinkVersion((void*)0);
static PShopPlayerGet __stub_PShopPlayerGet((void*)0);
static PShopList __stub_PShopList((void*)0);
static PShopListItem __stub_PShopListItem((void*)0);
static PlayerProfileGetProfileData __stub_PlayerProfileGetProfileData((void*)0);
static PlayerProfileSetProfileData __stub_PlayerProfileSetProfileData((void*)0);
static PlayerProfileGetMatchResult __stub_PlayerProfileGetMatchResult((void*)0);
static TankBattlePlayerGetRank __stub_TankBattlePlayerGetRank((void*)0);
static FactionResourceBattleGetMap __stub_FactionResourceBattleGetMap((void*)0);
static FactionResourceBattleGetRecord __stub_FactionResourceBattleGetRecord((void*)0);
static KeyExchange __stub_KeyExchange((void*)0);
static KickoutUser __stub_KickoutUser((void*)0);
static AccountingResponse __stub_AccountingResponse((void*)0);
static QueryUserPrivilege_Re __stub_QueryUserPrivilege_Re((void*)0);
static QueryUserForbid_Re __stub_QueryUserForbid_Re((void*)0);
static UpdateRemainTime __stub_UpdateRemainTime((void*)0);
static TransBuyPoint_Re __stub_TransBuyPoint_Re((void*)0);
static AddCash __stub_AddCash((void*)0);
static AddCash_Re __stub_AddCash_Re((void*)0);
static SysSendMail3 __stub_SysSendMail3((void*)0);
static AddictionControl __stub_AddictionControl((void*)0);
static BillingRequest __stub_BillingRequest((void*)0);
static BillingBalance __stub_BillingBalance((void*)0);
static BillingBalanceSA_Re __stub_BillingBalanceSA_Re((void*)0);
static BillingConfirm_Re __stub_BillingConfirm_Re((void*)0);
static NetBarAnnounce __stub_NetBarAnnounce((void*)0);
static AuthdVersion __stub_AuthdVersion((void*)0);
static SSOGetTicketRep __stub_SSOGetTicketRep((void*)0);
static AU2Game __stub_AU2Game((void*)0);
static DiscountAnnounce __stub_DiscountAnnounce((void*)0);
static AnnounceProviderID __stub_AnnounceProviderID((void*)0);
static PlayerLogin_Re __stub_PlayerLogin_Re((void*)0);
static PlayerKickout_Re __stub_PlayerKickout_Re((void*)0);
static PlayerLogout __stub_PlayerLogout((void*)0);
static PlayerOffline_Re __stub_PlayerOffline_Re((void*)0);
static QueryPlayerStatus __stub_QueryPlayerStatus((void*)0);
static GetTaskData __stub_GetTaskData((void*)0);
static SetTaskData __stub_SetTaskData((void*)0);
static PlayerInfoUpdate __stub_PlayerInfoUpdate((void*)0);
static PlayerTeamOp __stub_PlayerTeamOp((void*)0);
static PlayerTeamMemberOp __stub_PlayerTeamMemberOp((void*)0);
static GTradeStart_Re __stub_GTradeStart_Re((void*)0);
static GTradeDiscard __stub_GTradeDiscard((void*)0);
static KeepAlive __stub_KeepAlive((void*)0);
static DisconnectPlayer __stub_DisconnectPlayer((void*)0);
static SwitchServerStart __stub_SwitchServerStart((void*)0);
static SwitchServerSuccess __stub_SwitchServerSuccess((void*)0);
static SwitchServerTimeout __stub_SwitchServerTimeout((void*)0);
static SetChatEmotion __stub_SetChatEmotion((void*)0);
static FaceModify __stub_FaceModify((void*)0);
static FaceModifyCancel __stub_FaceModifyCancel((void*)0);
static GMRestartServer_Re __stub_GMRestartServer_Re((void*)0);
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
static QueryGameServerAttr __stub_QueryGameServerAttr((void*)0);
static ACReportCheater __stub_ACReportCheater((void*)0);
static ACTriggerQuestion __stub_ACTriggerQuestion((void*)0);
static BattleChallenge __stub_BattleChallenge((void*)0);
static BattleChallengeMap __stub_BattleChallengeMap((void*)0);
static BattleEnter __stub_BattleEnter((void*)0);
static BattleServerRegister __stub_BattleServerRegister((void*)0);
static DebugCommand __stub_DebugCommand((void*)0);
static StockCommission __stub_StockCommission((void*)0);
static StockAccount __stub_StockAccount((void*)0);
static StockTransaction __stub_StockTransaction((void*)0);
static StockBill __stub_StockBill((void*)0);
static StockCancel __stub_StockCancel((void*)0);
static BillingBalanceSA __stub_BillingBalanceSA((void*)0);
static BillingConfirm __stub_BillingConfirm((void*)0);
static BillingCancel __stub_BillingCancel((void*)0);
static SendRefCashUsed __stub_SendRefCashUsed((void*)0);
static SendTaskReward __stub_SendTaskReward((void*)0);
static RefGetReferenceCode_Re __stub_RefGetReferenceCode_Re((void*)0);
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
static SynMutaData __stub_SynMutaData((void*)0);
static CountryBattleApply __stub_CountryBattleApply((void*)0);
static CountryBattleJoinNotice __stub_CountryBattleJoinNotice((void*)0);
static CountryBattleLeaveNotice __stub_CountryBattleLeaveNotice((void*)0);
static CountryBattleOnlineNotice __stub_CountryBattleOnlineNotice((void*)0);
static CountryBattleOfflineNotice __stub_CountryBattleOfflineNotice((void*)0);
static CountryBattleEnterMapNotice __stub_CountryBattleEnterMapNotice((void*)0);
static CountryBattleServerRegister __stub_CountryBattleServerRegister((void*)0);
static CountryBattleStart_Re __stub_CountryBattleStart_Re((void*)0);
static CountryBattleEnd __stub_CountryBattleEnd((void*)0);
static PlayerChangeDS_Re __stub_PlayerChangeDS_Re((void*)0);
static TryChangeDS __stub_TryChangeDS((void*)0);
static PlayerRename __stub_PlayerRename((void*)0);
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
static FactionResourceBattleRequestConfig_Re __stub_FactionResourceBattleRequestConfig_Re((void*)0);
static FactionResourceBattleServerRegister __stub_FactionResourceBattleServerRegister((void*)0);
static FactionResourceBattleEventNotice __stub_FactionResourceBattleEventNotice((void*)0);
static FactionResourceBattlePlayerQuery __stub_FactionResourceBattlePlayerQuery((void*)0);
static DelRoleAnnounce __stub_DelRoleAnnounce((void*)0);
static DBFriendExtList_Re __stub_DBFriendExtList_Re((void*)0);
static TransBuyPoint __stub_TransBuyPoint((void*)0);
static SyncSellInfo __stub_SyncSellInfo((void*)0);
static DebugAddCash __stub_DebugAddCash((void*)0);
static DelFactionAnnounce __stub_DelFactionAnnounce((void*)0);
static CreateFactionFortress __stub_CreateFactionFortress((void*)0);
static ACRemoteCode __stub_ACRemoteCode((void*)0);
static ACQuestion __stub_ACQuestion((void*)0);
static ACAccuseRe __stub_ACAccuseRe((void*)0);
static Post_Re __stub_Post_Re((void*)0);
static GamePostCancel_Re __stub_GamePostCancel_Re((void*)0);
static WebPostCancel __stub_WebPostCancel((void*)0);
static Shelf __stub_Shelf((void*)0);
static ShelfCancel __stub_ShelfCancel((void*)0);
static Sold __stub_Sold((void*)0);
static PostExpire __stub_PostExpire((void*)0);
static WebGetRoleList __stub_WebGetRoleList((void*)0);
static LoadExchange __stub_LoadExchange((void*)0);
static DSAnnounceIdentity __stub_DSAnnounceIdentity((void*)0);
static SendDataAndIdentity __stub_SendDataAndIdentity((void*)0);
static SendDataAndIdentity_Re __stub_SendDataAndIdentity_Re((void*)0);
static RemoteLoginQuery __stub_RemoteLoginQuery((void*)0);
static RemoteLogout __stub_RemoteLogout((void*)0);
static KickoutRemoteUser_Re __stub_KickoutRemoteUser_Re((void*)0);
static GetRemoteRoleInfo_Re __stub_GetRemoteRoleInfo_Re((void*)0);
static GetRemoteCNetServerConfig_Re __stub_GetRemoteCNetServerConfig_Re((void*)0);
static RemoteLoginQuery_Re __stub_RemoteLoginQuery_Re((void*)0);
static KickoutRemoteUser __stub_KickoutRemoteUser((void*)0);
static GetRemoteRoleInfo __stub_GetRemoteRoleInfo((void*)0);
static GetRemoteCNetServerConfig __stub_GetRemoteCNetServerConfig((void*)0);

};
