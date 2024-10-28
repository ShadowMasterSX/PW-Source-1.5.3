
#ifndef __GNET_DEBUGCOMMAND_HPP
#define __GNET_DEBUGCOMMAND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "battlemanager.h"
#include "postoffice.h"
#include "dbsendmail.hrp"
#include "debugaddcash.hpp"
#include "clearstorehousepasswd.hrp"
//#include "dbsetrefdebug.hrp"
#include "referencemanager.h"
#include "sysauctionmanager.h"
#include "addcash.hpp"
#include "dbcopyrole.hrp"
#include "solochallengerank.h"
#include "cdcmnfbattleman.h"
#include "crosssystem.h"


namespace GNET
{

class DebugCommand : public GNET::Protocol
{
	#include "debugcommand"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("DebugCommand: roleid=%d tag=%d size=%d", roleid, tag, data.size());
		DEBUG_PRINT("DebugCommand: roleid=%d tag=%d size=%d", roleid, tag, data.size());
		Marshal::OctetsStream os;
		switch(tag)
		{
			case 1:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					BattleManager::GetInstance()->SetForgedTime(atoi(buf)); 
				}
				break;
			case 2:
				{
					char name[] = {0xfb,0x7c,0xdf,0x7e,0xa1,0x7b,0x06,0x74,0x58,0x54};
					char buf[256];
					os << (unsigned int)1 << (unsigned int)6 << (unsigned int)1;
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int rid = atoi(buf);
					DBSendMailArg arg;
					arg.mail.header.id        = 0;
					arg.mail.header.sender    = 1;
					arg.mail.header.sndr_type = _MST_AUCTION;
					arg.mail.header.receiver  = rid;
					arg.mail.header.title     = os;
					arg.mail.header.send_time = time(NULL);
					arg.mail.header.attribute = (1<<_MA_UNREAD);
					arg.mail.header.sender_name = Octets(name,10);
					arg.mail.header.attribute |= 1<<_MA_ATTACH_MONEY;
					arg.mail.attach_money = 200000;
					arg.mail.context = os;
					DBSendMail* rpc=(DBSendMail*) Rpc::Call( RPC_DBSENDMAIL, arg);
					GameDBClient::GetInstance()->SendProtocol( rpc );
				}
				break;
			case 3:
				{
					ClearStorehousePasswdArg arg;
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					arg.roleid = atoi(buf);
					ClearStorehousePasswd* rpc=(ClearStorehousePasswd*) Rpc::Call( RPC_CLEARSTOREHOUSEPASSWD,
						arg);
					GameDBClient::GetInstance()->SendProtocol( rpc );
				}
				break;
			case 4:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					int mapid, factionid;
					sscanf( buf, "%d-%d", &mapid, &factionid);
					BattleManager::GetInstance()->TestBattle(mapid, factionid); 
				}
				break;
			case 5:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					int mapid, factionid;
					sscanf( buf, "%d-%d", &mapid, &factionid);
					BattleManager::GetInstance()->SetOwner(mapid, factionid); 
				}
				break;
			case 22:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					
					int gid = atoi(buf);
					CountryBattleMan::OnSetDefaultGroup(gid);
				}
				break;
			case 23:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					
					int id = atoi(buf);
					if(id >= 0 && id <= 4) {
						CountryBattleMan::OnSetCountryIDCtrl(id);
					}
				}
				break;
			case 24:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					CountryBattleMan::OnSetAdjustTime(atoi(buf)); 
				}
				break;
			case 25:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					int roleid = 0;
					int orderid = 0;
					sscanf( buf, "%d-%d", &roleid, &orderid);

					PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);

					SysSendMail3 proto;
					proto.orderid = orderid;
					proto.userid = (pinfo != NULL) ? pinfo->userid : 0;
					proto.roleid = roleid;
					proto.rolename = (pinfo != NULL) ? pinfo->name : Octets();
					proto.mail_title = (pinfo != NULL) ? pinfo->name : Octets();
					proto.mail_context = (pinfo != NULL) ? pinfo->name : Octets();	
					proto.attach_money = 23;
					
					proto.attach_goods.goods_id = 2251;
					proto.attach_goods.count = 3;
					proto.attach_goods.proctype = 16403;
					proto.attach_goods.goods_flag = 1;
					proto.attach_goods.paytype = 1;

					proto.Process(GAuthClient::GetInstance(), 0);
				}
				break;
			case 26:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					int adjust_time = 60*atoi(buf);
					if (adjust_time < 0 || adjust_time > 24*3600)
						break;
					TankBattleManager::GetInstance()->SetAdjustTime(adjust_time); 
					LOG_TRACE("DebugCommand: TankBattleManager SetAdjustTime adjust_time=%d", adjust_time);
				}
				break;
			case 27:
				{
					TankBattleManager::GetInstance()->StartBattle(); 
					LOG_TRACE("DebugCommand: TankBattleManager StartBattle.");
				}
				break;
			case 28:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					int max_player = atoi(buf);
					if (max_player < 1 || max_player > 100)
						break;
					TankBattleManager::GetInstance()->SetMaxPlayer(max_player);
					LOG_TRACE("DebugCommand: TankBattleManager SetMaxPlayer=%d.",max_player);
				}
				break;
			case 29:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					int min_time = 0;
					int max_time = 0;
					sscanf( buf, "%d-%d", &min_time, &max_time);
					if (min_time < 1 || max_time < 1 || max_time > 60 || min_time > max_time)
						break;
					TankBattleManager::GetInstance()->SetMinAndMaxTime(min_time*60,max_time*60);
					LOG_TRACE("DebugCommand: TankBattleManager SetMinAndMaxTime min_time=%d max_time=%d.",min_time,max_time);
				}
				break;
            case 30:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

                    int day = 0;
					int time = 0;
					sscanf(buf, "%d-%d", &day, &time);
					if(day < 0 || day > 6) break;

					FactionResourceBattleMan::GetInstance()->SetAdjustTime(day, time); 
				}
				break;
            case 31:
                {
                     GDeliveryServer* dsm=GDeliveryServer::GetInstance();
                    //remove user baseinfo from cache
                    dsm->rbcache.remove(roleid);
                }
                break;
			case 32:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					CrossGuardClient::GetInstance()->SetDebug(atoi(buf) > 0);		
				}
				break;
			case 33:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

                    int day = 0;
					int time = 0;
					sscanf(buf, "%d-%d", &day, &time);
					if(day < 0 || day > 6) break;

					CrossGuardServer::GetInstance()->SetAdjustTime(day, time); 
				}
				break;
			case 200:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int cash_used = 0;
					int level = 0;
					sscanf( buf, "%d-%d", &cash_used, &level);
					//cash_used *= 34;
					ReferenceManager::GetInstance()->OnReferralUseCash(roleid, cash_used, level);
					RewardManager::GetInstance()->OnUseCash(roleid, cash_used);
					LOG_TRACE("DebugCommand: roleid=%d usecash %d level %d", roleid, cash_used, level);
				}
				break;
			case 201:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int bonus_add = 0;
					sscanf( buf, "%d", &bonus_add);
					int ret = 0;
					ret = ReferenceManager::GetInstance()->WithdrawBonusTest(roleid, bonus_add);
					LOG_TRACE("DebugCommand: roleid=%d bonus_add %d ret %d", roleid, bonus_add, ret);
				}
				break;
/*
			case 202:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int refer_userid = 0;
					int refer_roleid = 0;
					sscanf( buf, "%d-%d", &refer_userid, &refer_roleid);
					DBSetRefDebugArg arg(roleid, refer_userid, refer_roleid);
					DBSetRefDebug *rpc = (DBSetRefDebug *)Rpc::Call(RPC_DBSETREFDEBUG,arg); 
					GameDBClient::GetInstance()->SendProtocol(rpc);
					LOG_TRACE("DebugCommand: roleid=%d set ref_userid %d ref_roleid %d", roleid, refer_userid, refer_roleid);
				}
				break;
*/
			case 203:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int b_min=0, b_mon=0, b_day=0, b_hour=0;
					int e_min=0, e_mon=0, e_day=0, e_hour=0;
					int interval=0, times=0;
					int ret=0;
					ret = sscanf( buf, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &b_mon, &b_day, &b_hour, &b_min, &e_mon, &e_day, &e_hour, &e_min, &interval, &times);
					if (ret == 10)
						ret = RewardManager::GetInstance()->SetRewardTime(b_mon, b_day, b_hour, b_min, e_mon, e_day, e_hour, e_min, interval, times);
					LOG_TRACE("DebugCommand: roleid=%d set rewardtime ret %d", roleid, ret);
				}
				break;
			case 204:
				{
					time_t now = Timer::GetTime();
					time_t future;
					struct tm dt;
					int ret = 0;
					localtime_r(&now, &dt);
					dt.tm_year += 10;
					future = mktime(&dt);
					PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
					if (pinfo == NULL) break;
					ret = RewardManager::GetInstance()->CheckRewardList(pinfo->userid, future);
					LOG_TRACE("DebugCommand: roleid=%d clear rewardlist ret %d", roleid, ret);
				}
				break;
			case 205:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int cash_add = 0;
					sscanf( buf, "%d", &cash_add);
					PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
					if (pinfo == NULL)
						return;
					GameDBClient::GetInstance()->SendProtocol(DebugAddCash(pinfo->userid, cash_add));
					LOG_TRACE("DebugCommand: roleid=%d userid %d cash_add %d", roleid, pinfo->userid, cash_add);
				}
				break;
			case 206:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int bonus = 0;
					sscanf( buf, "%d", &bonus);
					RewardManager::GetInstance()->OnTaskReward(roleid, bonus);
					LOG_TRACE("DebugCommand: roleid=%d add bonus %d", roleid, bonus);
				}

				break;
			case 207:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int userid, locktime;
					if (sscanf(buf, "%d:%d", &userid, &locktime) != 2)
					{
						break;
					}
					IWebAutolockSet proto(0, userid, locktime);
					proto.Process(manager, 0);
					LOG_TRACE("DebugCommand: IWebAutolockSet userid=%d, locktime=%d", userid, locktime);
				}

				break;

			case 208:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					int userid, zoneid, sn, cash;
					if (sscanf(buf, "%d-%d-%d-%d", &userid, &zoneid, &sn, &cash) != 4)
					{
						break;
					}
				
					AddCash proto(userid, zoneid, sn, cash);
					proto.Process(NULL,0);
				}
				break;

			case 209:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					int domainid;
					int64_t wid,lid;
					sscanf( buf, "%d:%lld:%lld", &domainid, &wid, &lid);
					CDS_MNToplistMan::GetInstance()->AnnounceTop();	
					CDS_MNFactionBattleMan::GetInstance()->AnnounceWinner(domainid,wid,lid);
				}
				break;

			case 300:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;

					int hour, min, sec;
					if(3 != sscanf( buf, "%d:%d:%d", &hour, &min, &sec)) break;
					if(hour == -1 && min == -1 && sec == -1)
					{
						SysAuctionManager::GetInstance().SetAdjustTime(0);	
						LOG_TRACE("DebugCommand: SysAuctionManager SetAdjustTime (now)");
						break;
					}
					if(hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) break;
					time_t now = SysAuctionManager::GetInstance().GetTime();
					struct tm tm1;
					localtime_r(&now, &tm1);
					tm1.tm_hour = hour;
					tm1.tm_min = min;
					tm1.tm_sec = sec;
					time_t t1 = mktime(&tm1);
					time_t adjust = t1>now ? t1-now : t1+86400-now;
					SysAuctionManager::GetInstance().SetAdjustTime(adjust);	
					LOG_TRACE("DebugCommand: SysAuctionManager SetAdjustTime (%d:%d:%d)", hour, min, sec);
				}
				break;

			case 301:
				{
					if(FactionFortressMan::GetInstance().DebugDecHealthUpdateTime())	
						LOG_TRACE("DebugCommand: FactionFortressMan::DebugDecHealthUpdateTime");
				}
				break;
			
			case 302:
				{
					char buf[256];
					strncpy(buf, (char*)data.begin(), data.size());
					buf[data.size()] = 0;
					
					int fastmode = 0;
					sscanf( buf, "%d", &fastmode);
					FactionFortressMan::GetInstance().DebugAdjustBattlePeriod(fastmode);	
					LOG_TRACE("DebugCommand: FactionFortressMan::DebugAdjustBattlePeriod fastmode=%d",fastmode);
				}
				break;
			case 303:
				{
					GlobalControl::GetInstance()->SetForbidList(data);	
				}
				break;
			case 304:
				{
					Octets out;
					GlobalControl::GetInstance()->GetForbidList(out);
					char *buf = new char[out.size()+1];
					memcpy(buf,out.begin(),out.size());
					buf[out.size()] = '\0';
					LOG_TRACE("GlobalControl : %s",buf);
				}
				break;
            case 305:
                {
                    // d_delcmd 305 num-type-roleid-level-cls-total_time(s)

                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    int num = 0, type = 0, roleid = 0, level = 0, cls = 0, total_time = 0;
                    sscanf(buf, "%d-%d-%d-%d-%d-%d", &num, &type, &roleid, &level, &cls, &total_time);

                    char str_name[] = {'a', 0, 'l', 0, 'a', 0, 'x', 0};
                    Octets name(str_name, sizeof(str_name));

                    SoloChallengeRank::GetInstance().UpdateRankInfo(roleid, level, cls, name, total_time);
                    LOG_TRACE("DebugCommand: SoloChallengeRank::UpdateRankInfo(), roleid=%d, level=%d, cls=%d, total_time=%d, name_size=%d.",
                        roleid, level, cls, total_time, name.size());

                    for (int i = 1; i < num; ++i)
                    {
                        if (type == 0)
                            SoloChallengeRank::GetInstance().UpdateRankInfo((10000 + i), level, cls, name, total_time);
                        else if (type == 1)
                            SoloChallengeRank::GetInstance().UpdateRankInfo((10000 + i), (rand() % 17 + 89), (rand() % 12), name, (rand() % 20000 + 3600));
                    }
                }
                break;
            case 306:
                {
                    // d_delcmd 306 ranktype-cls

                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    int ranktype = 0, cls = 0, next_sort_time = 0;
                    sscanf(buf, "%d-%d", &ranktype, &cls);

                    typedef std::vector<SoloChallengeRankData> SCRVector;
                    SCRVector userdata;

                    SoloChallengeRank::GetInstance().GetRankInfo(ranktype, cls, next_sort_time, userdata);
                    LOG_TRACE("DebugCommand: SoloChallengeRank::GetRankInfo(), ranktype=%d, cls=%d, next_sort_time=%d, data_size=%d.",
                        ranktype, cls, next_sort_time, userdata.size());

                    SCRVector::const_iterator iter = userdata.begin(), iter_end = userdata.end();
                    for (; iter != iter_end; ++iter)
                    {
                        LOG_TRACE("DebugCommand: SoloChallengeRank::GetRankInfo(), roleid=%d, level=%d, cls=%d, total_time=%d, name_size=%d, zone_id=%d, update_time=%d.",
                            iter->roleid, iter->level, iter->cls, iter->total_time, iter->name.size(), iter->zoneid, iter->update_time);
                    }
                }
                break;
            case 307:
                {
                    // d_delcmd 307 adjust_time(s)

                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    int adjust_time = 0;
                    sscanf(buf, "%d", &adjust_time);

                    SoloChallengeRank::GetInstance().SetAdjustTime(adjust_time);
                    LOG_TRACE("DebugCommand: SoloChallengeRank::SetAdjustTime(), adjust_time=%d.", adjust_time);
                }
                break;

			case 308:
				{
                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    CDC_MNFactionBattleMan::GetInstance()->SetAdjustTime(atoi(buf));
                    LOG_TRACE("DebugCommand: CDC_MNFactionBattleMan::SetAdjustTime(), adjust_time=%d", atoi(buf));
				}
				break;

			case 309:
				{
                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    CDS_MNFactionBattleMan::GetInstance()->SetAdjustTime(atoi(buf));
                    LOG_TRACE("DebugCommand: CDS_MNFactionBattleMan::SetAdjustTime(), adjust_time=%d", atoi(buf));
				}
				break;

			case 310:
				{
                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    CDC_MNFactionBattleMan::GetInstance()->SetState(atoi(buf));
                    LOG_TRACE("DebugCommand: CDC_MNFactionBattleMan::SetState(), state=%d", atoi(buf));
				}
				break;

			case 311:
				{
                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    CDS_MNFactionBattleMan::GetInstance()->SetState(atoi(buf));
                    LOG_TRACE("DebugCommand: CDS_MNFactionBattleMan::SetState(), state=%d", atoi(buf));
				}
				break;
				
			case 312:
				{
                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    int domain_id = 0, attacker_zoneid = 0, defender_zoneid = 0;
					unsigned int attacker_fid = 0, defender_fid = 0;
                    sscanf(buf, "%d-%d-%d-%d-%d", &domain_id, &attacker_fid, &attacker_zoneid, &defender_fid, &defender_zoneid);
					CDS_MNFactionBattleMan::GetInstance()->SetDomainInfo(domain_id, attacker_fid, attacker_zoneid, defender_fid, defender_zoneid);
					DEBUG_PRINT("DebugCommand: CDS_MNFactionBattleMan::SetDomainInfo() domain_id=%d, attacker_fid=%d,attacker_zoneid=%d,defender_fid=%d,defender_zoneid=%d\n",
							domain_id, attacker_fid, attacker_zoneid, defender_fid, defender_zoneid);
						
				}
				break;

			case 313:
				{
					//开启跨服帮派战战场
					CDS_MNFactionBattleMan::GetInstance()->SetInitDomainInstance();
					DEBUG_PRINT("DebugCommand: CDS_MNFactionBattleMan::SetInitDomainInstance()"); 

				}
				break;

			case 314:
				{
                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    int domain_id = 0, attacker_or_defender = 0;
                    sscanf(buf, "%d-%d", &domain_id, &attacker_or_defender); 
                    CDS_MNFactionBattleMan::GetInstance()->SetDomainWinner(domain_id, (bool)attacker_or_defender);
                    LOG_TRACE("DebugCommand: CDS_MNFactionBattleMan::SetDomainWinner(), domain_%d winner is attacker or defender =%d", domain_id, (bool)attacker_or_defender);
				}
				break;

			case 315:
				{
                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    int zoneid = 0, roleid = 0, domain_id;
					unsigned int fid = 0;
                    sscanf(buf, "%d-%d-%d-%d", &roleid, &fid, &zoneid, &domain_id); 
                    CDS_MNFactionBattleMan::GetInstance()->SetPlayerBattleEnterSuccess(roleid, fid, zoneid, domain_id);
                    LOG_TRACE("DebugCommand: CDS_MNFactionBattleMan::SetPlayerBattleEnterSuccess(), roleid=%d fid=%d zoneid=%d enter battle domain_[%d] success", 
							roleid, fid, zoneid, domain_id);
				}
				break;

			case 316:
				{
					//测试计算奖励和发奖逻辑
					CDS_MNFactionBattleMan::GetInstance()->TestBonus();
					DEBUG_PRINT("DebugCommand: CDS_MNFactionBattleMan::TestBonus()"); 
				}
				break;

			case 317:
				{
					CDS_MNFactionBattleMan::GetInstance()->TestUpdateDomainNum();
					DEBUG_PRINT("DebugCommand: CDS_MNFactionBattleMan::TestUpdateDomainNum()"); 
				}
				break;

			case 318:
				{
					CDS_MNFactionBattleMan::GetInstance()->ClearDomainMap();
					DEBUG_PRINT("DebugCommand: CDS_MNFactionBattleMan::ClearDomainMap()"); 
				}
				break;

            case 319:
                {
                    char buf[256] = {0};
                    strncpy(buf, (char*)data.begin(), data.size());
                    buf[data.size()] = 0;

                    int old_state = 0, new_state = 0;
                    sscanf(buf, "%d", &new_state);

                    old_state = CrossSoloRankClient::GetInstance()->SetState(new_state);
                    LOG_TRACE("DebugCommand: CrossSoloRankClient::SetState(), old_state=%d, new_state=%d.",
                        old_state, new_state);
                }
                break;

            default:
                break;
		}
	}
};

};

#endif
