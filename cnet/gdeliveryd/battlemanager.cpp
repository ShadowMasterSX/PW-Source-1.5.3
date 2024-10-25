#include <list>
#include "battlemanager.h"
#include "hashstring.h"
#include "groleforbid"
#include "groleinventory"
#include "battlegetmap_re.hpp"
#include "battlechallengemap_re.hpp"
#include "battlestart.hpp"
#include "sendbattlechallenge.hpp"
#include "battlechallenge_re.hpp"
#include "battlechallengemap.hpp"
#include "domaindataman.h"
#include "mapuser.h"
#include "gfactionclient.hpp"
#include "dbbattlechallenge.hrp"
#include "gamedbclient.hpp"
#include "battlestatus_re.hpp"
#include "gdeliveryserver.hpp"
#include "maplinkserver.h"
#include "chatbroadcast.hpp"
#include "factionchat.hpp"
#include "dbbattlebonus.hrp"
#include "dbbattleend.hrp"
#include "dbbattleload.hrp"
#include "dbbattleset.hrp"
#include "battlefactionnotice.hpp"
#include "gfactionclient.hpp"
#include "localmacro.h"
#include "factionresourcebattleman.h"

namespace GNET
{

inline int Level2Bonus(int level)
{
	switch (level)
	{
	case DOMAIN_TYPE_3RD_CLASS: return 10000 * 1000;
	case DOMAIN_TYPE_2ND_CLASS: return 10000 * 2000;
	case DOMAIN_TYPE_1ST_CLASS: return 10000 * 3000;
	case DOMAIN_TYPE_NULL:
	default: return 0;
	}
}

bool BattleManager::SendMap(int roleid, unsigned int sid, unsigned int localsid)
{
	BattleGetMap_Re re;
	re.retcode = 0;
	re.maxbid = 1;
	re.status = (status&(ST_OPEN|ST_BIDDING));
	re.bonus_id = bonusid;
	re.bonus_count1 = countoflevel1;
	re.bonus_count2 = countoflevel2;
	re.bonus_count3 = countoflevel3;
	re.localsid = localsid;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(status&ST_BIDDING)
			re.cities.push_back(GTerritory(it->id, it->level, it->color, it->owner, 0, 0, 0, 0));
		else
			re.cities.push_back(GTerritory(it->id, it->level, it->color, it->owner, it->challenger, it->battle_time,
					0, 0));
	}
	GDeliveryServer::GetInstance()->Send(sid, re);
	return true;
}

bool BattleManager::GetMapNotice(BattleMapNotice& pro)
{
	if(!(status&ST_DATAREADY))
		return false;
	pro.status = 0;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		pro.cities.push_back(GCity(it->id, it->level, it->owner));
	}
	return true;
}

bool BattleManager::SyncMapNotice(short id)
{
	LOG_TRACE( "BattleManager: Sync map notice to GS, cityid=%d.", id);
	BattleMapNotice pro;
	pro.status = 0;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->id==id)
		{
			pro.cities.push_back(GCity(it->id, it->level, it->owner));
			break;
		}
	}
	GProviderServer::GetInstance()->BroadcastProtocol(pro);
	return true;
}

bool BattleManager::SyncMapNotice()
{
	LOG_TRACE( "BattleManager: Sync map notice to GS.");
	BattleMapNotice pro;
	pro.status = 0;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		pro.cities.push_back(GCity(it->id, it->level, it->owner));
	}
	GProviderServer::GetInstance()->BroadcastProtocol(pro);
	return true;
}

bool BattleManager::SyncBattleFaction()
{
	LOG_TRACE( "BattleManager: Sync battle faction to gfactiond.");
	BattleFactionNotice pro;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->owner > 0) pro.factionids.insert(it->owner);
		if(it->challenger > 0) pro.factionids.insert(it->challenger);
	}
	if(pro.factionids.size())
		GFactionClient::GetInstance()->SendProtocol(pro);
	return true;
}

bool BattleManager::SendStatus(int roleid,unsigned int sid, unsigned int linkid)
{
	BattleStatus_Re re;
	re.retcode = 0;
	re.localsid = linkid;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		re.cities.push_back((char)(it->status&CS_FIGHTING));
	}
	GDeliveryServer::GetInstance()->Send(sid, re);
	return true;
}

/**
 * 判断该帮派所拥有的领土是否与该城市相邻
 * @param cityid 城市ID
 * @param fid 帮派ID
 * @return true: 该帮派拥有和该城市相邻的领土 false: 该帮派没有和该城市相邻的领土
 */
bool BattleManager::IsAdjacent(short cityid, unsigned int fid)
{
	DOMAIN_INFO_SERV * pcity = domain_data_getbyid(cityid);

	if(!pcity)
		return false;
	TVector::iterator ie=cities.end();
	for(std::vector<int>::iterator i=pcity->neighbours.begin();i!=pcity->neighbours.end();++i)
	{
		for(TVector::iterator it=cities.begin();it!=ie;++it)
		{
			if(it->id==*i)
			{
				if(fid==it->owner)
					return true;
				break;
			}
		}
	}
	return false;
}

int BattleManager::Challenge(const Protocol& arg, Protocol& res)
{
	const SendBattleChallenge& proto = (const SendBattleChallenge&)arg;
	BattleChallenge_Re& result = (BattleChallenge_Re&)res;
	if(!(status&ST_BIDDING))
	{
		return ERR_BS_FAILED;
	}
	if(proto.authentication != (rand_num ^ ((t_base >> 8) & 0xFFFF) ^ proto.id) )
	{
		return ERR_BS_FAILED;
	}
	int sum = 0;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		GChallengerInfoList gci;
		if(it->challengerdetails.size())
			Marshal::OctetsStream(it->challengerdetails)>>gci;
		for(std::vector<GChallengerInfo>::iterator gciter=gci.challengerlist.begin();gciter!=gci.challengerlist.end();++gciter)
		{
			if(gciter->faction==proto.factionid)
			{
				return ERR_BS_FAILED;
			}
		}
		if(it->owner==proto.factionid)
			sum++;
	}
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->id==proto.id)
		{
			//result.deposit = it->deposit;
			//result.challenger = it->challenger;
			result.cutoff_time = it->cutoff_time;
			if(it->owner==proto.factionid)
				return ERR_BS_FAILED;
			//if(it->deposit==0 && proto.deposit<100000)
			//	return ERR_BS_FAILED;
			if(proto.deposit < 1) //至少需要一张银票竞拍
				return ERR_BS_FAILED;
			//if(it->deposit+100000>proto.deposit)
			//	return ERR_BS_FAILED;
			if(proto.maxbonus<=0)
				return ERR_BS_FAILED;
			if(it->status&CS_BIDDING)
				return ERR_BS_FAILED;
			//if(proto.deposit > proto.syncdata.inventory.money)
			//	return ERR_BS_FAILED; //在之前已经判断过，不再需要判断
			if(it->cutoff_time==0)
				return ERR_BS_FAILED;
			//if(it->level==DOMAIN_TYPE_2ND_CLASS && (sum < 2 || proto.deposit<1000000))
			//	return ERR_BS_FAILED;
			//if(it->level==DOMAIN_TYPE_1ST_CLASS && (sum < 3 || proto.deposit<2000000))
			//	return ERR_BS_FAILED;
			if(it->level==DOMAIN_TYPE_2ND_CLASS && sum < 2)
				return ERR_BS_FAILED;
			if(it->level==DOMAIN_TYPE_1ST_CLASS && sum < 3)
				return ERR_BS_FAILED;
			if(sum && !IsAdjacent(it->id, proto.factionid))
				return ERR_BS_FAILED;
			DBBattleChallengeArg arg(proto.roleid,it->id,proto.factionid,proto.deposit,proto.maxbonus,GetTime(),proto.syncdata);
			
			if(it->owner==0)
				arg.maxbonus = (int)(MAX_PACKAGE_MONEY*0.9);
			DBBattleChallenge* rpc = (DBBattleChallenge*)Rpc::Call(RPC_DBBATTLECHALLENGE,arg);
			if(GameDBClient::GetInstance()->SendProtocol(rpc))
			{
				time_t now = GetTime();
				it->status |= CS_BIDDING;
				it->timeout = now+180;
				if(it->cutoff_time-now<1200)
					it->cutoff_time += 1200;
				if(it->cutoff_time > BidMaxDuration())
					it->cutoff_time = BidMaxDuration();
				return ERR_SUCCESS;
			}
			else
			{
				LOG_TRACE( "BattleManager: Send to gamedbd failed.\n");
				return ERR_BS_OUTOFSERVICE;
			}
		}
	}
	return ERR_DATAERROR;
}

bool BattleManager::ChallengeMap(int roleid,int factionid)
{
	BattleChallengeMap_Re re;
	re.roleid = roleid;
	re.retcode = 0;
	re.status = (status&ST_BIDDING)?1:0;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		GChallengerInfoList gci;
		if(it->challengerdetails.size())
			Marshal::OctetsStream(it->challengerdetails)>>gci;
		else
		{
			re.cities.push_back(GBattleChallenge(it->id,0,0,it->maxbonus,it->cutoff_time));
			continue;
		}
		std::vector<GChallengerInfo>::iterator gciter;
		for(gciter=gci.challengerlist.begin();gciter!=gci.challengerlist.end();++gciter)
		{
			if((int)gciter->faction==factionid)
			{
				re.cities.push_back(GBattleChallenge(it->id,factionid,0,it->maxbonus,it->cutoff_time));
				break;
			}
		}
		if(gciter==gci.challengerlist.end())
		{
			re.cities.push_back(GBattleChallenge(it->id,(unsigned int)-1,0,it->maxbonus,it->cutoff_time));
		}
	}
	re.rand_num = rand_num;
	GFactionClient::GetInstance()->SendProtocol(re);
	return true;
}

bool BattleManager::Initialize()
{
	if(domain_data_load())
		return false;
	UpdateTime();
	IntervalTimer::Attach( this,60000000/IntervalTimer::Resolution());
	status |= ST_OPEN;
	Conf* conf = Conf::GetInstance();
	bonusid = atoi(conf->find("BattleBonus","id").c_str());
	countoflevel1 = atoi(conf->find("BattleBonus","countoflevel1").c_str());
	countoflevel2 = atoi(conf->find("BattleBonus","countoflevel2").c_str());
	countoflevel3 = atoi(conf->find("BattleBonus","countoflevel3").c_str());
	max_bonus_count = atoi(conf->find("BattleBonus","maxcount").c_str()); 
	proctype = atoi(conf->find("BattleBonus","proctype").c_str());
	specialid = atoi(conf->find("BattleBonus","specialid").c_str());
	countofspecial = atoi(conf->find("BattleBonus","countofspecial").c_str());
	specialproctype = atoi(conf->find("BattleBonus","specialproctype").c_str());
	max_count_special = atoi(conf->find("BattleBonus","maxcountofspecial").c_str());
	return true;
}

bool BattleManager::RegisterServer(int server, int world)
{
	LOG_TRACE( "Register Server: server=%d map=%d.\n", server, world );
	servers.insert(std::make_pair(world, server));
	return true;
}

bool BattleManager::LoadMap(std::vector<GTerritoryDetail>& v)
{
	size_t count = domain_data_getcount();
	if(v.size()!=count)
	{
		Log::log(LOG_ERR,"Map load error. load size%d, config size %d", v.size(), count);
		return false;
	}

	if((status&ST_DATAREADY)&&(cities.size()==count))
	{
		Log::log(LOG_ERR,"Map already loaded. status=%d", status);
	}
	else
	{
		cities = v;
		time_t now = Timer::GetTime();
		size_t i = 0;
		//int week=now/604800;
		for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it,++i)
		{
			DOMAIN_INFO_SERV * p = domain_data_getbyindex(i);
			if(it->level!=p->type)
				return false;
			if(it->owner)
			{
				if(it->color==0 || it->color>TERRITORY_NUMBER)
					it->color = SelectColor(it->owner);
			}
			else
				it->color = 0;

			if(it->battle_time && it->challenger!=0 && it->battle_time<now+600)
			{
				it->status |= CS_BATTLECANCEL;
				Log::log(LOG_ERR,"Battle time missed, cancel cityid=%d defender=%d attacker=%d deposit=%d time=%d.", 
						it->id, it->owner, it->challenger, it->deposit, it->battle_time);
			}
			if(it->challenger && !it->battle_time)
			{
				status |= ST_SETTIME;
			}
			if(it->challenger==0)
			{
				it->deposit = 0;
				it->maxbonus = 0;
				it->cutoff_time = 0;
			}
		//	if((it->bonus_time/604800)==week)
		//	{
		//		status |= ST_SPECIALOK;
		//	}
		}
		status |= ST_DATAREADY;
		SyncMapNotice();
		//通知gfaction加载城战相关帮派信息
		SyncBattleFaction();	
	}

    FactionResourceBattleMan::GetInstance()->OnDomainLoadComplete();
	return true;
}

bool BattleManager::SyncChallenge(std::vector<GTerritoryDetail>& v)
{
	if(v.size()!=cities.size())
	{
		Log::log(LOG_ERR,"SyncChallenge: fatal, size not equal %d != %d.", v.size(), cities.size());
		return false;
	}

	TVector::iterator i=v.begin(),it=cities.begin(),ie=cities.end();

	for(;it!=ie;++it,++i)
	{
		if(it->owner!=i->owner)
		{
			Log::log(LOG_ERR,"SyncChallenge: fatal, owner not equal %d != %d.", it->owner, i->owner);
			it->owner = i->owner;
		}
		if(it->challenger!=i->challenger)
		{
			Log::log(LOG_ERR,"SyncChallenge: challenger not equal %d != %d.", it->challenger, i->challenger);
			it->challenger = i->challenger;
		}
	}
	EndChallenge();
	return true;
}

bool BattleManager::OnChallenge(short result,int challenge_res,short cityid,unsigned int deposit,unsigned int maxbonus,int fid,int ctime,Protocol& pro)
{
	BattleChallenge_Re&  reply = (BattleChallenge_Re&)pro;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->id==cityid)
		{
			GChallengerInfoList gci;
			if(it->challengerdetails.size())
				Marshal::OctetsStream(it->challengerdetails)>>gci;
			it->status &= (~CS_BIDDING);
			if(result==ERR_SUCCESS)
			{
				if(challenge_res==1)
				{
					it->challenge_time=ctime;
					it->deposit = deposit;
					it->challenger=fid;
					it->maxbonus = maxbonus;
				}
				
				GChallengerInfo cinfo;
				cinfo.faction=fid;
				cinfo.time=ctime;
				cinfo.deposit=deposit;
				gci.challengerlist.push_back(cinfo);
				Marshal::OctetsStream challengedata;
				challengedata<<gci;
				it->challengerdetails=challengedata;
				{
					ChatBroadCast cbc;
					cbc.channel = CMSG_BATTLEBID;
					cbc.srcroleid = fid; 
					Marshal::OctetsStream data;
					//data<<(unsigned char)it->id<<(unsigned int)fid<<(unsigned int)deposit
					//	<<(unsigned int)it->cutoff_time;
					data<<(unsigned char)it->id<<(unsigned int)0<<(unsigned int)0<<(unsigned int)0
					<<(unsigned int)it->cutoff_time;
					cbc.msg = data;
					GFactionClient::GetInstance()->SendProtocol(cbc);
				}
				Log::formatlog("battle_onchallenge","zoneid=%d:cityid=%d:owner=%d:challenger=%d:deposit=%d", 
					GDeliveryServer::GetInstance()->zoneid,cityid,it->owner,it->challenger,it->deposit);
			}
			//reply.deposit = it->deposit;
			reply.cutoff_time = it->cutoff_time;
			//reply.challenger = it->challenger;
			break;
		}
	}
	return true;
}

bool BattleManager::SendPlayer(int roleid, const Protocol& proto, unsigned int& localsid, unsigned int& linkid)
{
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if (pinfo)
		{
			linkid = pinfo->linksid;
			localsid = pinfo->localsid;
		}
		else
			return false;
	}
	return GDeliveryServer::GetInstance()->Send(linkid, proto);
}

bool BattleManager::FindSid(int roleid, unsigned int& localsid, unsigned int& linkid, unsigned int& gsid)
{
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if ( pinfo )
		{
			linkid = pinfo->linksid;
			localsid = pinfo->localsid;
			gsid = pinfo->gameid;
		}
		else
			return false;
	}
	return true;
}

int BattleManager::GetMapType(int id)
{
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->id==id)
		{
			int map = 228 + it->level*2;
			if(!it->owner)
				map++;
			return map;
		}
	}
	return  0;
}

void BattleManager::BeginSendBonus()
{
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
		it->status &= (~CS_BONUSOK);
		SendSpecialBonus();
}

time_t BattleManager::GetTime()
{
	if(t_forged)
		return t_base+t_forged; //伪造时间，用于debug调试
	else
		return Timer::GetTime();
}

void BattleManager::SetForgedTime(time_t forge)
{
	t_forged = forge;
}

void BattleManager::BeginChallenge()
{
	ChatBroadCast cbc;
	cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
	cbc.srcroleid = CMSG_BIDSTART; 
	LinkServer::GetInstance().BroadcastProtocol(cbc);
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->challenger!=0)
		{
			Log::log(LOG_ERR,"Warning: challenger %d of cityid=%d is not 0, deposit=%d.",
				it->challenger,it->id,it->deposit);
		}
		it->status = 0;
		it->cutoff_time = BidDefDuration();
		it->battle_time = 0;
		it->timeout = 0;
	}
	status &= (~ST_SETTIME);
	status |= ST_BIDDING;

	rand_num = rand() & 0xFFFF;
}

void BattleManager::EndChallenge()
{
	status &= (~ST_BIDDING);

	ChatBroadCast cbc;
	Marshal::OctetsStream data;
	cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
	cbc.srcroleid = CMSG_BIDEND;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->challenger != 0)
		{
			data<<(short)it->id<<(unsigned int)it->owner<<(unsigned int)it->challenger;
		}
	}
	cbc.msg=data;
	LinkServer::GetInstance().BroadcastProtocol(cbc);

	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
		it->cutoff_time = 0;

	ArrangeBattle();
	DBBattleSetArg arg;
	arg.reason = _BATTLE_SETTIME;
	arg.cities = cities;
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBBATTLESET,arg));

	rand_num = 0;
}

bool BattleManager::OnSendBonus(short ret, unsigned int fid,GRoleInventory& item, unsigned int money)
{
	if(ret==ERR_SUCCESS)
	{
		FactionChat msg(GN_CHAT_CHANNEL_SYSTEM);
		Marshal::OctetsStream data;
		data << (unsigned int)money;
		data << item.id;
		data << item.count;
		msg.src_roleid = CMSG_BONUSSEND;
		msg.msg = data;
		msg.dst_localsid = fid;
		GFactionClient::GetInstance()->SendProtocol(msg);
	}
	SendBonus();
	return true;
}

bool BattleManager::SendBonus()
{
	unsigned int fid=0;
	int bonus = 0;
	GRoleInventory inv;
	inv.id=bonusid;
	inv.proctype=proctype;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(!it->owner || (it->status&CS_BONUSOK))
			continue;
		if(!fid)
			fid = it->owner;
		if(fid==it->owner)
		{
			/*
			DOMAIN_INFO_SERV* p = domain_data_getbyid(it->id);
			if(p)
				bonus += p->reward;
			*/
			//bonus += Level2Bonus(it->level);
			switch(it->level)
			{
				case 1:
					inv.count+=countoflevel1;
					break;
				case 2:
					inv.count+=countoflevel2;
					break;
				case 3:
					inv.count+=countoflevel3;
					break;
				default:
					break;
			}
			inv.max_count = max_bonus_count;
			it->status |= CS_BONUSOK;
		}
	}
	if(fid && (bonus<0 || bonus>MAX_BONUS))
	{
		Log::log(LOG_ERR,"Invalid bonus amount %d for faction=%d.", bonus, fid);
		return false;
	}
	if(fid && (inv.id<=0 || inv.count<=0))
	{
		Log::log(LOG_ERR,"Invalid bonus:inv error,num = %d for faction = %d.",inv.count,fid);
		return false;
	}
	if(fid)
	{
		DBBattleBonusArg arg;
		arg.factionid = fid;
		arg.money = 0;
		arg.item = inv;
		arg.isspecialbonus=0;
		Log::formatlog("sendbonus","zoneid=%d:factionid=%d:amount=%d", GDeliveryServer::GetInstance()->zoneid, fid, bonus);
		DBBattleBonus* rpc = (DBBattleBonus*) Rpc::Call( RPC_DBBATTLEBONUS, arg);
		GameDBClient::GetInstance()->SendProtocol( rpc );
		return true;
	}
	else
		return false;
}

bool BattleManager::SendSpecialBonus()
{
	unsigned int fid=0;
	int bonus = 0;
	GRoleInventory inv;
	inv.id=specialid;
	inv.proctype=specialproctype;
	inv.count=countofspecial;
	inv.max_count=max_count_special;
	TVector tempforandom;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(!it->owner)
			continue;
		tempforandom.push_back(*it);
	}
	if(!tempforandom.size())
	{
		return false;
	}
	int randomcity=rand() % tempforandom.size();
	short cid = tempforandom[randomcity].id;
	fid = tempforandom[randomcity].owner;
	if(fid && (bonus<0 || bonus>MAX_BONUS))
	{
		Log::log(LOG_ERR,"Invalid bonus amount %d for faction=%d.", bonus, fid);
		return false;
	}
	if(fid && (inv.id<=0 || inv.count<=0))
	{
		Log::log(LOG_ERR,"Invalid special bonus:inv error,num = %d for faction = %d.",inv.count,fid);
		return false;
	}
	if(fid)
	{
		//ChatBroadCast cbc;
		//Marshal::OctetsStream data;
		//data<<(short)cid ;
		//cbc.msg = data;
		//cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
		//cbc.srcroleid = CMSG_SPECIAL;
		//LinkServer::GetInstance().BroadcastProtocol(cbc);
		DBBattleBonusArg arg;
		arg.factionid = fid;
		arg.cityid=cid;
		arg.money = 0;
		arg.item = inv;
		arg.isspecialbonus=1;
		Log::formatlog("sendspecialbonus","zoneid=%d:factionid=%d:amount=%d", GDeliveryServer::GetInstance()->zoneid, fid, bonus);
		DBBattleBonus* rpc = (DBBattleBonus*) Rpc::Call( RPC_DBBATTLEBONUS, arg);
		GameDBClient::GetInstance()->SendProtocol( rpc );
		return true;
	}
	else
		return false;
}

time_t BattleManager::UpdateTime()
{
	time_t now = GetTime();
	LOG_TRACE("Timer update: (%d) %s", t_forged, ctime(&now));
	if(now-t_base>604800) //已经过去一周时间
	{
		struct tm dt;
		localtime_r(&now, &dt);
		dt.tm_sec = 0;
		dt.tm_min = 0;
		dt.tm_hour = 0;
		t_base = mktime(&dt)-86400*dt.tm_wday; //t_base重置为本周周日的零点
		status &= (~(ST_BIDDING|ST_BONUSOK));
	}
	return now;
}

void BattleManager::UpdateBid(time_t now)
{
	bool done = true;
	if(BidBeginTime() < now && now < BidMaxDuration() + 120)
	{
		if((status&ST_BIDDING)==0)
		{
		   	if(now < BidDefDuration())
			{
				Log::formatlog("beginbid","zoneid=%d:now=%d", GDeliveryServer::GetInstance()->zoneid, now);
				BeginChallenge();
			}
			return;
		}
		for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
		{
			if(it->cutoff_time && now>it->cutoff_time)
				it->cutoff_time = 0;
			if(it->cutoff_time)
				done = false;
			if(it->status&CS_BIDDING)
			{
				if(it->timeout && it->timeout<now)
				{
					it->status &= (~CS_BIDDING);
					it->timeout = 0;
				}
				done = false;
			}
		}
	}
	else if(status&ST_SETTIME)
	{
		Log::formatlog("settime","zoneid=%d:now=%d", GDeliveryServer::GetInstance()->zoneid, now);
		status &= (~ST_SETTIME);
		ArrangeBattle();
		DBBattleSetArg arg;
		arg.reason = _BATTLE_SETTIME;
		arg.cities = cities;
		GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBBATTLESET,arg));
	}

	if((status&ST_BIDDING) && done)
	{
		Log::formatlog("endbid","zoneid=%d:now=%d", GDeliveryServer::GetInstance()->zoneid, now);
		status &= (~ST_BIDDING);
		GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBBATTLELOAD,DBBattleLoadArg(1)));
	}
}

void BattleManager::UpdateBattle(time_t now)
{
	bool cancel_sent = false;
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->status&CS_BATTLECANCEL)
		{
			if(cancel_sent)
				continue;
			Log::formatlog("battleend","zoneid=%d:cityid=%d:time=%d:result=4:defender=%d:attacker=%d", 
					GDeliveryServer::GetInstance()->zoneid, it->id, now, it->owner, it->challenger);
			DBBattleEndArg rpcarg(it->id, _BATTLE_CANCEL, 0, it->owner, it->challenger); 
			GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBBATTLEEND,rpcarg));
			it->status = 0;
			it->challenger = 0;
			it->battle_time = 0;
			it->deposit = 0;
			it->maxbonus = 0;
			it->challengerdetails.clear();
			cancel_sent = true;
			
		}else if(it->status&CS_SENDSTART)
		{
			if(it->timeout && it->timeout <=now)
			{
				Log::log(LOG_ERR,"Start battle timeout, cancel cityid=%d defender=%d attacker=%d deposit=%d.", 
						it->id, it->owner, it->challenger, it->deposit);
				it->status &= (~CS_SENDSTART);
				it->status |= CS_BATTLECANCEL;
				it->timeout = 0;
			}
		}else if(it->status&CS_FIGHTING)
		{
			if(it->timeout && it->timeout <=now)
			{
				Log::log(LOG_ERR,"Battle end timeout, cancel cityid=%d defender=%d attacker=%d, deposit=%d.", 
						it->id, it->owner, it->challenger, it->deposit);
				it->status &= (~CS_FIGHTING);
				it->status |= CS_BATTLECANCEL;
				it->timeout = 0;
			}
		}else if(it->battle_time && it->challenger!=0 && it->battle_time<now)
		{
			it->status |= CS_SENDSTART;
			it->timeout = now + 600;
			BattleStart start;
			start.battle_id = it->id;
			start.map_type = 228 + it->level*2;
			if(!it->owner)
				start.map_type++;
			start.defender = it->owner;
			start.attacker = it->challenger;
			start.end_time = now+3600*3;

			int server = FindServer(start.map_type);
			if(server)
			{
				LOG_TRACE("BattleManager: start battle on server %d, battle=%d defender=%d attacker=%d\n",
					server, start.battle_id, start.defender, start.attacker);
				GProviderServer::GetInstance()->DispatchProtocol(server,start);
			}
			else
				LOG_TRACE("BattleManager: battle server for map=%d not found\n", start.map_type);
		}
	}
}

bool BattleManager::Update()
{
	time_t now = UpdateTime();

	UpdateBid(now);

	if((status&ST_BONUSOK)==0 && RewardBeginTime() < now)
	{
		status |= ST_BONUSOK;
		BeginSendBonus();
	}

	if((status&ST_BIDDING)==0)
		UpdateBattle(now);

	return true;
}

char BattleManager::SelectColor(unsigned int factionid)
{
	char colors[TERRITORY_NUMBER+1] = {0};
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->owner==factionid && it->color)
		{
			return it->color;
		}
		if(it->color<TERRITORY_NUMBER+1 && it->color>0)
			colors[it->color] = 1;
	}
	for(int i=1;i<TERRITORY_NUMBER+1;i++)
	{
		if(colors[i]==0)
			return i;
	}
	return 0;
}

bool BattleManager::OnBattleEnd(int id, int result, int defender, int attacker)
{
	int color = 0;
	unsigned int deposit=0;
	Log::formatlog("battleend","zoneid=%d:cityid=%d:time=%d:result=%d:defender=%d:attacker=%d", 
			GDeliveryServer::GetInstance()->zoneid, id, GetTime(), result, defender, attacker);
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->id==id)
		{
			if((it->status&CS_FIGHTING)==0)
			{
				Log::log(LOG_ERR,"BattleManager: cityid=%d is not in fighting state, result=%d defender=%d attacker=%d.", 
					id, result, defender, attacker);
				return false;
			}
			if(result==1)
			{
				it->color = SelectColor(attacker);
				it->owner = attacker;
				color = it->color;
			}
			deposit = it->deposit;
			it->challenger = 0;
			it->status = 0;
			it->deposit = 0;
			it->maxbonus = 0;
			it->battle_time = 0;
			it->challengerdetails.clear();
			break;
		}
	}
	
	ChatBroadCast cbc;
	Marshal::OctetsStream data;
	//data<<(unsigned char)id<<(unsigned char)result<<deposit<<(unsigned int)attacker<<(unsigned char)color;
	data<<(unsigned char)id<<(unsigned char)result<<(unsigned int)0<<(unsigned int)attacker<<(unsigned char)color;
	cbc.msg = data;
	cbc.channel = CMSG_BATTLEEND;
	cbc.srcroleid = attacker;
	GFactionClient::GetInstance()->SendProtocol(cbc);

	SyncMapNotice(id);

	DBBattleEndArg rpcarg(id, result, color, defender, attacker); 
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBBATTLEEND,rpcarg));
	/*
	TVector::iterator itv=cities.begin();
	for(;itv!=cities.end();++itv)
	{
		if(itv->status!=0 && itv->status!=CS_FIGHTED)
			break;
	}
	if(itv==cities.end())
	{
		TVector tempforrandom;
		for(TVector::iterator itt=cities.begin();itt!=cities!=cities.end();++itt)
		{
			if(0==it->owner)
				continue;
			tempforrandom.push_back(*itt);
		}
		int randomcity=rand()%tempforrandom.size();
		unsigned int cid=tempforrandom[rand()%tempforrandom.size()].id;
		
		Conf* conf=Conf::GetInstance();
		GRoleInventory inv;
		inv.id=atoi(conf->find("BattleBonus","id").c_str());
		inv.count=inv.max_count=atoi(conf->find("BattleBonus","countofspecial").c_str());
		inv.proctype=atoi(conf->find("BattleBonus","proctype").c_str());
		
		DBBattleSpecialBonusArg dbsba(cid,0,inv);
		GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBBATTLESPECIALBONUS,dbsba);
	}
	*/	
	return true;
}

bool BattleManager::OnBattleStart(int id, int retcode)
{
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->id==id)
		{
			if(retcode)
			{
				it->status &= (~CS_SENDSTART);
				it->status |= CS_BATTLECANCEL;
				Log::log(LOG_ERR,"Battle start failed (%d), cancel cityid=%d defender=%d attacker=%d"
					     " deposit=%d.", retcode, it->id, it->owner, it->challenger, it->deposit);
				return false;
			}
			it->status |= CS_FIGHTING;
			it->status &= (~CS_SENDSTART);
			it->timeout = GetTime() + 4*3600;

			ChatBroadCast cbc;
			cbc.channel = CMSG_BATTLESTART;
			cbc.srcroleid = it->challenger; 
			DEBUG_PRINT("start battle :challenger = %d",it->challenger);
			Marshal::OctetsStream data;
			data << (unsigned char)it->id << (unsigned int)it->challenger;
			cbc.msg = data;
			GFactionClient::GetInstance()->SendProtocol(cbc);
			Log::formatlog("battlestart","zoneid=%d:cityid=%d:time=%d:defender=%d:attacker=%d:deposit=%d", 
				GDeliveryServer::GetInstance()->zoneid,id,GetTime(),it->owner,it->challenger,
				it->deposit);
			break;
		}
	}
	return true;
}

typedef std::vector<BattleManager::TVector::iterator>  ItVector;

/**
 * 在同一时间段进行城战的列表
 */
class TimingSet
{
	unsigned int max; //同一时间段可进行的最大的城战场次
public:
	ItVector list; //同一时间段进行城战的城市列表，里面有攻防双方的信息
	int priority;
	TimingSet(int m) : max(m),priority(0){}
	bool Insert(BattleManager::TVector::iterator& it, bool force=false)
	{
		if(max<=list.size())
			return false; //场次已满
		if(!force)
		{
			for(ItVector::iterator i=list.begin();i!=list.end();++i)
				if(it->owner==(*i)->challenger || it->challenger==(*i)->owner) //同一时间段的领土战，一帮派不能同时进攻和防守
					return false;
		}
		list.push_back(it);
		priority += 2*it->level;
		return true;
	}
	bool TryInsert(unsigned int owner, ItVector& v, int size)
	{
		if(max-list.size()<(unsigned int)size) //没有足够多的场次
			return false;
		for(ItVector::iterator it=list.begin();it!=list.end();++it)
			if((*it)->challenger==owner) //同一时间段的领土战，一帮派不能同时进攻和防守
				return false;
		for(ItVector::iterator il=v.begin();il!=v.end()&&size;++il)
		{
			ItVector::iterator it;
			for(it=list.begin();it!=list.end();++it)
				if((*il)->challenger==(*it)->owner) //同一时间段的领土战，一帮派不能同时进攻和防守
					break;
			if(it==list.end())
				size--;
		}
		return (size==0);
	}
};
struct compare_Priority
{
	bool operator() (const TimingSet& c1,const TimingSet& c2) const { return c1.priority > c2.priority; }
};

struct compare_Time
{
	bool operator() (const BattleManager::TVector::iterator& g1,const BattleManager::TVector::iterator& g2) const
	{
		return g1->challenge_time < g2->challenge_time;
	}
};
	
/**
 * 某帮派要防御的领土战的列表
 */
class ItSet
{
public:
	ItVector list; //帮派要防御的领土战列表
	unsigned int owner; //帮派ID
	int priority;
	ItSet(int o,BattleManager::TVector::iterator& it) : owner(o)
	{
		priority = it->level;
		list.push_back(it);
	}
	bool Add(BattleManager::TVector::iterator& it)
	{
		priority += it->level;
		list.push_back(it);
		return true;
	}
	std::vector<TimingSet>::iterator FindSlot(std::vector<TimingSet>& set,std::vector<TimingSet>::iterator it,int n)
	{
		for(;it!=set.end() && !it->TryInsert(owner, list, n);++it);
		return it;
	}
};
struct compare_Size
{
	bool operator() (const ItSet& i1,const ItSet& i2) const
	{
		return (i1.list.size()>i2.list.size()) || (i1.list.size()==i2.list.size() && i1.priority>i2.priority);
	}
};

#define ARRANGE_BATTLE_GROUP_SIZE 7

/**
 * 找到列表中，挑战时间间隔最小的N场领土战(N目前为7)，作为一组
 * @list 领土战列表
 * @return 间隔最小的N场领土战的首地址的iterator
 */
ItVector::iterator GetCombination(ItVector& list)
{
	int interval=INT_MAX;
	ItVector::iterator res=list.begin();
	ItVector::iterator iti=list.begin();
	if(list.size() >= ARRANGE_BATTLE_GROUP_SIZE)
	{
		for(; (iti + (ARRANGE_BATTLE_GROUP_SIZE - 1)) != list.end(); iti++)
		{
			if( interval > ((*(iti + ARRANGE_BATTLE_GROUP_SIZE - 1))->challenge_time - (*iti)->challenge_time) )
			{
				interval = (*(iti + ARRANGE_BATTLE_GROUP_SIZE - 1))->challenge_time - (*iti)->challenge_time;
				res=iti;
			}
		}
	}
	return res;
}

bool BattleManager::ArrangeBattle()
{
	std::vector<ItSet>     sorter; //各个帮派要防守的领土战列表，其实本质是一个二维数组，每个元素都是某一帮派要防御的领土战列表
	std::vector<TimingSet> scheduler; //在不同时间段进行城战的列表，其实本质是一个二维数组，每个元素是某一时间段要进行的领土战列表
	ItVector left;
	ItVector::iterator triple;
	
	//times表示可以打城战的时间段list，tmax表示一个时间段最多可进行的场次
	const std::vector<BATTLETIME_SERV>& times = getbattletimelist();
	unsigned int i,tsize = times.size(),tmax = getbattletimemax();
	
	//初始化不同时间段进行的战场列表
	for(i=0;i<tsize;i++)
		scheduler.push_back(TimingSet(tmax));
		
	//初始化各个帮派要防守的领土战列表
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->challenger) //有人向该领土挑战
		{
			if(it->owner==0) //该领土无主
				sorter.push_back(ItSet(0,it));
			else //该领土已经被某帮派占领
			{
				std::vector<ItSet>::iterator its = sorter.begin();
				for(;its!=sorter.end();++its)
				{
					if(its->owner==it->owner) //这个帮派已经有领土被挑战
					{
						its->Add(it); //将本次被挑战的领土加入该帮派被挑战的列表
						break;
					}
				}
				if(its==sorter.end()) //这个帮派尚未有被挑战的领土
					sorter.push_back(ItSet(it->owner,it)); //将本次被挑战的领土加入该帮派被挑战的列表
			}
		}
	}
	
	for(std::vector<ItSet>::iterator its=sorter.begin();its!=sorter.end();++its)
	{
		//将每个帮派要防御的领土战按出价时间排序
		std::sort(its->list.begin(),its->list.end(),compare_Time());	
	}
	//将不同帮派要防御的领土战列表，按每个帮派要防御的场次多少排序
	std::sort(sorter.begin(),sorter.end(),compare_Size());
	
	std::vector<TimingSet>::iterator ic, ice=scheduler.end();
	for(std::vector<ItSet>::iterator its=sorter.begin(),ite=sorter.end();its!=ite;++its)
	{
		unsigned int size = its->list.size();
		ic = scheduler.begin();
		while(size)
		{
			
			//size = its->list.size()>3?3:its->list.size();
			size=its->list.size();
			if(size > ARRANGE_BATTLE_GROUP_SIZE)
			{
				size = ARRANGE_BATTLE_GROUP_SIZE;
				triple=GetCombination(its->list); //找到列表中，挑战时间间隔最小的N场领土战(N目前为7)，作为一组
				ic = its->FindSlot(scheduler, ic, size); //尝试着在领土战的不同时间段中，找到本组的合适位置
				if(ic==ice)
					ic = its->FindSlot(scheduler, scheduler.begin(), size); //没有合适的位置，从头重新尝试一遍
				if(ic==ice)
					break; //重新尝试后，依然找不到，留待最后一并处理
				i=0;
				for(ItVector::iterator il=triple;il!=its->list.end()&&i<size;)
				{
					if(ic->Insert(*il)) //添加到合适的时间段
					{
						i++;
						il=its->list.erase(il); //将已经安排好的场次从原列表中删除
					}
					else
					{
						il++;
					}
				}
			}
			else
			{
				ic = its->FindSlot(scheduler, ic, size); //尝试着在领土战的不同时间段中，找到本组的合适位置
				if(ic==ice)
					ic = its->FindSlot(scheduler, scheduler.begin(), size); //没有合适的位置，从头重新尝试一遍
				if(ic==ice) 
					break; //重新尝试后，依然找不到，留待最后一并处理
				i = 0;
				for(ItVector::iterator il=its->list.begin();il!=its->list.end()&&i<size;)
				{
					if(ic->Insert(*il)) //添加到合适的时间段
					{
						i++;
						il = its->list.erase(il); //将已经安排好的场次从原列表中删除
					}
					else
						++il;
				}
			}
			if(++ic==ice)
				ic = scheduler.begin();
		}
		
		//将剩余找不到合适时段的场次都存入该列表中
		left.insert(left.end(),its->list.begin(),its->list.end());
	}
	//处理剩余没有安排的领土战
	for(ItVector::iterator it=left.begin();it!=left.end();++it)
	{
		std::vector<TimingSet>::iterator is=scheduler.begin(),ise=scheduler.end();
		for(;is!=ise&&!is->Insert(*it);++is);
		if(is==ise)
			for(is=scheduler.begin();is!=ise&&!is->Insert(*it,true);++is);
	}
	std::sort(scheduler.begin(),scheduler.end(),compare_Priority());
	i = 0;
	for(std::vector<TimingSet>::iterator it=scheduler.begin(),ie=scheduler.end();it!=ie;++it,++i)
	{
		int offset =0;
		const BATTLETIME_SERV& t = times[i];
		for(ItVector::iterator il=it->list.begin();il!=it->list.end();++il)
		{
			(*il)->battle_time = t_base + 86400*(t.nDay?t.nDay:7) + 3600*t.nHour + 60*t.nMinute + offset;
			if((*il)->battle_time < BidBeginTime())
			{
				(*il)->battle_time += 604800;
			}
			offset += 180;
		}
	}
	return true;
}

bool BattleManager::OnDBConnect(Protocol::Manager *manager, int sid)
{
	if(status&ST_OPEN)
		manager->Send(sid,Rpc::Call(RPC_DBBATTLELOAD,DBBattleLoadArg(0)));
	return true;
}

bool BattleManager::OnDelFaction(unsigned int factionid)
{
	for(TVector::iterator it=cities.begin();it!=cities.end();++it)
	{
		if(it->owner==factionid)
		{
			it->owner = 0;
			it->color = 0;
		}
		if(it->challenger==factionid)
		{
			it->challenger = 0;
			it->battle_time = 0;
			it->deposit = 0;
			it->maxbonus = 0;
		}
	}
	return true;
}

void BattleManager::SetOwner(int id, int factionid)
{
	DBBattleSetArg arg;
	TVector::iterator it=cities.begin(),ie=cities.end();
	for(;it!=ie;++it)
	{
		if(it->id==id)
		{
			it->owner = factionid;
			it->color = 40;
			arg.cities.push_back(*it);
			break;
		}
	}
	arg.reason = _BATTLE_DEBUG;
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBBATTLESET,arg));
}

void BattleManager::TestBattle(int id, int challenger)
{
	SetForgedTime(0);
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		if(it->id!=id)
			continue;

		time_t now = Timer::GetTime();
		it->status |= CS_SENDSTART;
		it->timeout = now + 600;
		BattleStart start;
		start.battle_id = it->id;
		start.map_type = 228 + it->level*2;
		if(!it->owner)
			start.map_type++;
		start.defender = it->owner;
		start.attacker = challenger;
		it->challenger = challenger;
		start.end_time = now+1800;

		int server = FindServer(start.map_type);
		if(server)
		{
			LOG_TRACE("BattleManager: start battle on server %d, battle=%d defender=%d attacker=%d\n",
				server, start.battle_id, start.defender, start.attacker);
			GProviderServer::GetInstance()->DispatchProtocol(server,start);
		}
		else
			LOG_TRACE("BattleManager: battle server for map=%d not found\n", start.map_type);
	}
}

bool BattleManager::GetCityInfo(CityWar & cw)
{
	for(TVector::iterator it=cities.begin(),ie=cities.end();it!=ie;++it)
	{
		GCity city;
		city.id = it->id;
		city.level = it->level;
		city.owner = it->owner;
		cw.citylist.push_back(city);
	}
	return true;
}
};
