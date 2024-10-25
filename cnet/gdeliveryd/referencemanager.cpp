#include "referencemanager.h"
#include "greferral"
#include "referralbrief"
#include "greferrer"
#include "mapuser.h"
#include "timer.h"
#include <time.h>
#include "dbrefgetreferral.hrp"
#include "dbrefupdatereferral.hrp"
#include "dbrefgetreferrer.hrp"
#include "dbrefupdatereferrer.hrp"
#include "gamedbclient.hpp"
#include "gproviderserver.hpp"
#include "sendrefaddbonus.hpp"
#include <limits.h>
#include "localmacro.h"
#include "reftrans.h"


namespace GNET
{
ReferenceManager ReferenceManager::instance;

void Referral::OnLoad(const GReferral &referral, int _referrer)
{
	SetLoaded(true);
	userid = referral.userid;
	referrer = _referrer;
	bonus_total1 = referral.bonus_total1;
	bonus_total2 = referral.bonus_total2;
	bonus_withdraw = referral.bonus_withdraw;
	bonus_withdraw_today = referral.bonus_withdraw_today;
	max_role_level = referral.max_role_level;
	namelist = referral.rolenames;
	CheckBonusWithdrawToday();
}

void Referral::SyncDB()
{
	DBRefUpdateReferralArg arg;
	ToGReferral(arg.referral);
	DBRefUpdateReferral *rpc = (DBRefUpdateReferral *)Rpc::Call(RPC_DBREFUPDATEREFERRAL,arg); 
	GameDBClient::GetInstance()->SendProtocol(rpc);
}

void Referral::OnLogin()
{
	if (IsLoaded())
		CheckBonusWithdrawToday();
}

void Referral::ToGReferral(GReferral &gref)
{
	gref.userid = userid;
	gref.bonus_total1 = bonus_total1;
	gref.bonus_total2 = bonus_total2;
	gref.bonus_withdraw = bonus_withdraw;
	gref.bonus_withdraw_today = bonus_withdraw_today;
	gref.max_role_level = max_role_level;
	gref.rolenames = namelist;
}

void Referral::CheckBonusWithdrawToday()
{
	LOG_TRACE("CheckBonusWithDrasToday() userid %d", userid);
//	if (bonus_withdraw_today > 0)
	{
		UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
		if (pinfo==NULL)
		{
			LOG_TRACE("CheckBonusWithDrasTOday() userid %d UserInfo is NULL", userid);
			return;
		}
		struct tm tmnow;
		time_t now = Timer::GetTime();
		localtime_r(&now, &tmnow);
		tmnow.tm_sec = 0;
		tmnow.tm_min = 0;
		tmnow.tm_hour = 0;
		time_t midnight = mktime(&tmnow);

		if (pinfo->lastlogin < midnight)
		{
			LOG_TRACE("clear bonus_withdraw_today userid %d", userid);
			bonus_withdraw_today = 0;
			SetDirty(true);
			return;
		}
	}
}

void Referrer::ToGReferrer(GReferrer &gref)
{
	gref.userid = userid;
	gref.bonus_add = bonus_withdraw;
}

void Referrer::OnLoad(const GReferrer &referrer)
{
	SetLoaded(true);
	userid = referrer.userid;
	bonus_withdraw = referrer.bonus_add;
//	CheckExpWithdrawToday();
}
/*
void Referrer::OnLogin()
{
	if (IsLoaded())
		CheckExpWithdrawToday();
}
*/
/*
void Referrer::CheckExpWithdrawToday()
{
	if (exp_withdraw_today > 0)
	{
		GRoleInfo *role = RoleInfoCache::Instance().Get(roleid);
		if (role == NULL) return;

		struct tm tmnow;
		time_t now = Timer::GetTime();
		localtime_r(&now, &tmnow);
		tmnow.tm_sec = 0;
		tmnow.tm_min = 0;
		tmnow.tm_hour = 0;
		time_t midnight = mktime(&tmnow);

		if (role->lastlogin_time < midnight)
		{
			exp_withdraw_today = 0;
			SetDirty(true);
			return;
		}
	}
}
*/
void Referrer::SyncDB()
{
	DBRefUpdateReferrerArg arg;
	ToGReferrer(arg.referrer);
	DBRefUpdateReferrer *rpc = (DBRefUpdateReferrer *)Rpc::Call(RPC_DBREFUPDATEREFERRER, arg);
	GameDBClient::GetInstance()->SendProtocol(rpc);
}

bool ReferenceManager::Initialize()
{
	IntervalTimer::Attach(this,10000000/IntervalTimer::Resolution());
	status |= ST_OPEN;
	return true;
}
//userlogin时候调用
//需要在playerlogin时才调用，否则不会记录上次登录时间
void ReferenceManager::OnLogin(UserInfo * userinfo)
{
/*
	ReferralMap::iterator itr = referrals.find(userinfo.userid);
	if (itr != referrals.end())
	{
		itr->second.OnLogin();
	}
*/
/*	
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo==NULL || pinfo->user==NULL) return;
*/
	if (!(status & ST_OPEN)) return;
	ReferralMap::iterator it = referrals.find(userinfo->userid);
	if (it == referrals.end())
	{
		if (userinfo->real_referrer > 0)
		{
			Referrer &referrer = referrers[userinfo->real_referrer];
			if (!referrer.IsLoaded())
			{
				DBRefGetReferrer *rpc = (DBRefGetReferrer *)Rpc::Call(RPC_DBREFGETREFERRER, Integer(userinfo->real_referrer));
				GameDBClient::GetInstance()->SendProtocol(rpc);
			}
			DBRefGetReferral *rpc = (DBRefGetReferral *)Rpc::Call(RPC_DBREFGETREFERRAL, Integer(userinfo->userid));
			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
	}
	else
	{
		it->second.SetLogout(false);
		it->second.OnLogin();
		if (userinfo->real_referrer > 0)
		{
			Referrer &referrer = referrers[userinfo->real_referrer];
			referrer.AddReferral(userinfo->userid);
			if (!referrer.IsLoaded())
			{
				DBRefGetReferrer *rpc = (DBRefGetReferrer *)Rpc::Call(RPC_DBREFGETREFERRER, Integer(userinfo->real_referrer));
				GameDBClient::GetInstance()->SendProtocol(rpc);
			}
		}
	}
//below is for test
/*
	if (!referrers[userinfo->userid].IsLoaded())
	{
		LOG_TRACE("get referrer userid %d", userinfo->userid);
		DBRefGetReferrer *rpc = (DBRefGetReferrer *)Rpc::Call(RPC_DBREFGETREFERRER, Integer(userinfo->userid));
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
*/
}

void ReferenceManager::OnLogout(UserInfo * userinfo)
{
	if (!(status & ST_OPEN)) return;
	ReferralMap::iterator itr = referrals.find(userinfo->userid);
	if (itr != referrals.end())
	{
		ReferrerMap::iterator it = referrers.find(itr->second.referrer);
		if (it != referrers.end()) it->second.DelReferral(userinfo->userid);
		if (itr->second.IsDirty())
			itr->second.SetLogout(true);
		else
			referrals.erase(itr);
	}
}

void ReferenceManager::OnLoadReferral(const GReferral &gref)
{
	UserInfo * pinfo = UserContainer::GetInstance().FindUser(gref.userid);
	if (pinfo && pinfo->status == _STATUS_ONGAME)
	{
		int referrer = pinfo->real_referrer;
		if (referrer > 0)
		{
			referrals[gref.userid].OnLoad(gref, referrer);
			referrers[referrer].AddReferral(gref.userid);
		}
	}
}

void ReferenceManager::OnLoadReferrer(const GReferrer &referrer)
{
	LOG_TRACE("load referrer userid %d", referrer.userid);
	if (!(status & ST_OPEN)) return;
	ReferrerMap::iterator it = referrers.find(referrer.userid);
	if (it != referrers.end())
	{
		LOG_TRACE("load 2 referrer userid %d", referrer.userid);
		it->second.OnLoad(referrer);
	}
}

bool ReferenceManager::Update()
{
	if (!referrals.empty())
	{
		ReferralMap::iterator it = referrals.lower_bound(referral_cursor);
		ReferralMap::iterator ie = referrals.end();
		int i = 0; 

		while (it!=ie && i < CHECKNUM_ONUPDATE)
		{
			if (it->second.IsDirty())
			{
				it->second.SyncDB();
				++it;
			}
			else if (it->second.IsLogout())
			{
				ReferrerMap::iterator itr = referrers.find(it->second.referrer);
				if (itr != referrers.end())
					itr->second.DelReferral(it->first);
				referrals.erase(it++);
			}
			else 
				++it;
			++i;
		}

		if (it != ie)
			referral_cursor = it->first;
		else
			referral_cursor = 0;
	}

	if (!referrers.empty())
	{
		ReferrerMap::iterator it = referrers.lower_bound(referrer_cursor);
		ReferrerMap::iterator ie = referrers.end();

		int i = 0;
		while (it!=ie && i < CHECKNUM_ONUPDATE)
		{
			if (it->second.IsDirty())
			{
				it->second.SyncDB();
				++it;
			}
			else if (it->second.IsEmpty())
			{
			//below is for test
				referrers.erase(it++);
			}
			else
				++it;
			++i;
		}

		if (it != ie)
			referrer_cursor = it->first;
		else
			referrer_cursor = 0;
	}

	if (!transmap.empty())
	{
		WithdrawTransMap::iterator it = transmap.lower_bound(trans_cursor);
		WithdrawTransMap::iterator ie = transmap.end();

		int i = 0; 
		while (it!=ie && i < CHECKNUM_ONUPDATE)
		{
			if (referrers.find(it->first) == referrers.end())
			{
				if (it->second)
				{
					it->second->Rollback();
					delete it->second;
				}
				transmap.erase(it++);
			}
			else
				++it;
			++i;
		}

		if (it != ie)
			trans_cursor = it->first;
		else
			trans_cursor = 0;
	}
	return true;
}

void Referral::OnUseCash(int cashused, int level, Octets rolename)
{
	if (level > max_role_level)
		max_role_level = level;
	std::vector<Octets>::iterator it, ite;
	if (rolename.size())
	{
		for (it=namelist.begin(),ite=namelist.end(); it!=ite; ++it)
		{
			if (*it == rolename)
				break;
		}
		if (it == ite)
			namelist.push_back(rolename);
	}
	int bonus_cont = cashused * 3/100;
	if (bonus_total1 > INT_MAX-bonus_cont)
		bonus_total1 = INT_MAX;
	else
		bonus_total1 += bonus_cont;
	SetDirty(true);
}

void ReferenceManager::OnReferralUseCash(int roleid, int cashused, int level)
{
	if (!(status & ST_OPEN)) return;
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo == NULL)
		return;
	ReferralMap::iterator it = referrals.find(pinfo->userid);
	if (it != referrals.end()) it->second.OnUseCash(cashused, level, pinfo->name);
}

void ReferenceManager::InsertRefCode(int roleid, const Octets & code)
{
	refcodemap[roleid] = code;
}

bool ReferenceManager::GetRefCode(int roleid, Octets & code)
{
	if (!(status & ST_OPEN)) return true;
	RefCodeMap::const_iterator it;
	it = refcodemap.find(roleid);
	if (it == refcodemap.end())
		return false;
	else
	{
		code = it->second;
		return true;
	}
}

int ReferenceManager::BonusWithdrawLimit(int level)
{
	if (level <= 20)
		return level*6;
	else if (level <= 40)
		return level*8;
	else if (level <= 60)
		return level*20;
	else if (level <= 70)
		return level*50;
	else if (level <=80)
		return level*90;
	else if (level <= 90)
		return level*170;
	else
		return level*340;
}

/*
#define BONUS_WITHDRAW_LIMIT(LEVEL) ((int)((LEVEL)<=90?6*(LEVEL):0.00002*(LEVEL)*(LEVEL)*(LEVEL)*(LEVEL)+0.523*(LEVEL)*(LEVEL)-79.8*(LEVEL)+3225))
#define EXP_WITHDRAW_LIMIT_NOREBORN(LEVEL) ((int64_t)((LEVEL)<75?(0.45*(LEVEL)*(LEVEL)+12)*(int64_t)10000:(8*(LEVEL)*(LEVEL)-1156*(LEVEL)+44300)*(int64_t)10000))
#define EXP_WITHDRAW_LIMIT_REBORN(LEVEL) ((int64_t)((LEVEL)<75?(0.45*(LEVEL)*(LEVEL)+12)*(int64_t)100000:(8*(LEVEL)*(LEVEL)-1156*(LEVEL)+44300)*(int64_t)100000))
#define EXP_WITHDRAW_LIMIT(LEVEL, REBORN_CNT) ((REBORN_CNT)<=0?EXP_WITHDRAW_LIMIT_NOREBORN(LEVEL):EXP_WITHDRAW_LIMIT_REBORN(LEVEL))
*/
int ReferenceManager:: ListReferrals(int roleid, int start_index, int &total, int &bonus_avail_today, std::vector<ReferralBrief> &referral_briefs)
{
/*
	RoleInfoCache &cache = RoleInfoCache::Instance();
	GRoleInfo *referrer_role = cache.Get(roleid);
	if (referrer_role == NULL) return REF_ERR_REFERRERLOGOUT;
*/
	if (!(status & ST_OPEN)) return REF_ERR_NOREFERRALFOUND;
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo == NULL)
		return REF_ERR_NOREFERRALFOUND;
	ReferrerMap::iterator it = referrers.find(pinfo->userid);
	if (it == referrers.end())
		return REF_ERR_NOREFERRALFOUND;	
	else if (!it->second.IsLoaded())
		return REF_ERR_REFERRERNOTLOADED;
	else if (IsInTransaction(pinfo->userid))
		return REF_ERR_REFERRERINTRANSACTION;
	else
	{
		Referrer &ref = it->second;
		if (ref.IsEmpty()) return REF_ERR_NOREFERRALFOUND;
		Referrer::ReferralSet &referral_list = ref.referrals;
		total = (int)referral_list.size();
		int list_num = 0;
		if (total > start_index)
		{
			list_num = REF_LIMIT_REFERRALPERPAGE;
			if (total-start_index < REF_LIMIT_REFERRALPERPAGE)
				list_num = total-start_index;
			referral_briefs.resize(list_num);
		}

		Referrer::ReferralSet::iterator itr = referral_list.begin(), ite = referral_list.end();
		int index = 0;
		//int64_t total_exp_left = 0;
		bonus_avail_today = 0;
		for (int i = 0; itr != ite; ++i, ++itr)
		{
//			GRoleInfo *referral_info = cache.Get(*itr);
			ReferralMap::iterator itr2 = referrals.find(*itr);
			if (/*referral_info!=NULL &&*/ itr2!=referrals.end())
			{
				//int referral_level = referral_info->level+referral_info->reborn_cnt*200;
				int bonus_withdraw_limit = BonusWithdrawLimit(itr2->second.max_role_level);
				bonus_withdraw_limit -= itr2->second.bonus_withdraw_today;
				if (bonus_withdraw_limit < 0) bonus_withdraw_limit = 0;
				int bonus_left_total = itr2->second.bonus_total1+itr2->second.bonus_total2-itr2->second.bonus_withdraw;
				int referral_bonus_avail = bonus_withdraw_limit < bonus_left_total ? bonus_withdraw_limit : bonus_left_total;
				if (referral_bonus_avail > 0 && itr2->second.bonus_withdraw > INT_MAX-referral_bonus_avail)
					referral_bonus_avail = INT_MAX-itr2->second.bonus_withdraw;

/*
				if (bonus_withdraw_limit > itr2->second.bonus_total1+itr2->second.bonus_total2)
					bonus_withdraw_limit = itr2->second.bonus_total1+itr2->second.bonus_total2;
				int referral_bonus_avail = bonus_withdraw_limit-itr2->second.bonus_withdraw; 
*/
				bonus_avail_today += referral_bonus_avail;
/*
				int64_t referral_exp_left = itr2->second.exp_total1+itr2->second.exp_total2-itr2->second.exp_withdraw; 
				total_exp_left += referral_exp_left;
*/
				if (index<list_num && i>=start_index)
				{
					ReferralBrief &brief = referral_briefs[index++];
					brief.rolenames = itr2->second.namelist;
					brief.max_level = itr2->second.max_role_level;
					brief.bonus_total1 = itr2->second.bonus_total1;
					brief.bonus_total2 = itr2->second.bonus_total2;
					//brief.bonus_left = itr2->second.bonus_total1+itr2->second.bonus_total2-itr2->second.bonus_withdraw;
					brief.bonus_left = bonus_left_total;
					brief.bonus_avail = referral_bonus_avail;
				}
			}
		}
/*
		exp_withdraw_limit -= ref.exp_withdraw_today;
		exp_avail_today = exp_withdraw_limit>total_exp_left ? total_exp_left : exp_withdraw_limit;
*/
		return ERR_SUCCESS;
	}
}

void ReferenceManager::OnDBUpdateReferrer(int userid)
{
	ReferrerMap::iterator it = referrers.find(userid);
	if (it != referrers.end()) it->second.SetDirty(false);
}

void ReferenceManager::OnDBUpdateReferral(int userid)
{
	ReferralMap::iterator it = referrals.find(userid);
	if (it != referrals.end()) it->second.SetDirty(false);
}

int ReferenceManager::WithdrawBonus(int roleid)
{
/*
	GRoleInfo *referrer_role = RoleInfoCache::Instance().Get(roleid);
	if (referrer_role == NULL) return REF_ERR_REFERRERLOGOUT;
*/
	if (!(status & ST_OPEN)) return REF_ERR_NOREFERRALFOUND;
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo == NULL)
		return REF_ERR_NOREFERRALFOUND;
	ReferrerMap::iterator it = referrers.find(pinfo->userid);
	if (it == referrers.end())
		return REF_ERR_NOREFERRALFOUND;	
	else if (!it->second.IsLoaded())
		return REF_ERR_REFERRERNOTLOADED;
	else if (IsInTransaction(pinfo->userid))
		return REF_ERR_REFERRERINTRANSACTION;
	else
	{
		Referrer &ref = it->second;
		if (ref.IsEmpty()) return REF_ERR_NOREFERRALFOUND;
//		int64_t exp_withdraw_limit = EXP_WITHDRAW_LIMIT(referrer_role->level, referrer_role->reborn_cnt);
//		exp_withdraw_limit -= ref.exp_withdraw_today;

		Referrer::ReferralSet &referral_list = ref.referrals;
		Referrer::ReferralSet::iterator itr, ite = referral_list.end();
		WithdrawBonusTrans *ptrans = new WithdrawBonusTrans();
		int total_bonus_withdraw = 0;
		for (itr = referral_list.begin(); itr!=ite/* && total_bonus_withdraw<exp_withdraw_limit*/; ++itr)
		{
			ReferralMap::iterator itr2 = referrals.find(*itr);
			if (itr2 != referrals.end())
			{
				int bonus_withdraw_limit = BonusWithdrawLimit(itr2->second.max_role_level);
				bonus_withdraw_limit -= itr2->second.bonus_withdraw_today;
				if (bonus_withdraw_limit < 0) bonus_withdraw_limit = 0;
				int bonus_left_total = itr2->second.bonus_total1+itr2->second.bonus_total2-itr2->second.bonus_withdraw;
				int referral_bonus_avail = bonus_withdraw_limit < bonus_left_total ? bonus_withdraw_limit : bonus_left_total;
				if (referral_bonus_avail > 0)
				{
					if (itr2->second.bonus_withdraw > INT_MAX-referral_bonus_avail)
						referral_bonus_avail = INT_MAX-itr2->second.bonus_withdraw;
					ptrans->referral_withdraws[*itr] = referral_bonus_avail;
					itr2->second.bonus_withdraw += referral_bonus_avail;
					itr2->second.bonus_withdraw_today += referral_bonus_avail;
					itr2->second.SetDirty(true);
					total_bonus_withdraw += referral_bonus_avail;
				}
/*
				int64_t exp_to_withdraw = itr2->second.exp_total1+itr2->second.exp_total2-itr2->second.exp_withdraw;
				if (exp_to_withdraw > 0)
				{
					if (total_exp_withdraw+exp_to_withdraw > exp_withdraw_limit) 
						exp_to_withdraw = exp_withdraw_limit-total_exp_withdraw;

					total_exp_withdraw += exp_to_withdraw;
					ptrans->referral_withdraws[*itr] = itr2->second.exp_withdraw;
					itr2->second.exp_withdraw += exp_to_withdraw;
					itr2->second.SetDirty(true);
				}
*/
			}
		}
		if (ref.bonus_withdraw > INT_MAX-total_bonus_withdraw)
			total_bonus_withdraw = INT_MAX-ref.bonus_withdraw;
		if (total_bonus_withdraw > 0)
		{
			ptrans->roleid = roleid;
			ptrans->referrer = pinfo->userid;
//			ptrans->total_withdraw = ref.exp_withdraw_today;
			ptrans->to_withdraw = total_bonus_withdraw;
			ref.bonus_withdraw += total_bonus_withdraw;
			ref.SetDirty(true);

			ReferralMap::iterator itr2 = referrals.find(pinfo->userid);
			if (itr2 != referrals.end())
			{
				int total2_delt = total_bonus_withdraw/3;
				if (itr2->second.bonus_total2 > INT_MAX-total2_delt)
					total2_delt = INT_MAX-itr2->second.bonus_total2;
				ptrans->total2 = total2_delt;
				itr2->second.bonus_total2 += total2_delt;
				itr2->second.SetDirty(true);
			}

			if (ptrans->TrySubmit())
			{
				Log::formatlog("reference", "role(id=%d) withdraw bonus %d\n", roleid, total_bonus_withdraw);
				transmap[pinfo->userid] = ptrans;
				return ERR_SUCCESS;
			}
			else
			{
				ptrans->Rollback();
				delete ptrans;
				return REF_ERR_SUBMITTODB;
			}
		}
		else
		{
			delete ptrans;
			return REF_ERR_NOBONUSAVAILABLE;
		}
	}
}

int ReferenceManager::WithdrawBonusTest(int roleid, int bonus_add)
{
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if (pinfo == NULL)
		return REF_ERR_NOREFERRALFOUND;
	ReferrerMap::iterator it = referrers.find(pinfo->userid);
	if (it == referrers.end())
		return REF_ERR_NOREFERRALFOUND;	
	else if (!it->second.IsLoaded())
		return REF_ERR_REFERRERNOTLOADED;
	else if (IsInTransaction(pinfo->userid))
		return REF_ERR_REFERRERINTRANSACTION;
	else
	{
		Referrer &ref = it->second;
		//if (ref.IsEmpty()) return REF_ERR_NOREFERRALFOUND;
//		int64_t exp_withdraw_limit = EXP_WITHDRAW_LIMIT(referrer_role->level, referrer_role->reborn_cnt);
//		exp_withdraw_limit -= ref.exp_withdraw_today;

		WithdrawBonusTrans *ptrans = new WithdrawBonusTrans();
		int total_bonus_withdraw = bonus_add;
		if (ref.bonus_withdraw > INT_MAX-total_bonus_withdraw)
			total_bonus_withdraw = INT_MAX-ref.bonus_withdraw;
		if (total_bonus_withdraw > 0)
		{
			ptrans->roleid = roleid;
			ptrans->referrer = pinfo->userid;
//			ptrans->total_withdraw = ref.exp_withdraw_today;
			ptrans->to_withdraw = total_bonus_withdraw;
			ref.bonus_withdraw += total_bonus_withdraw;
			ref.SetDirty(true);

			ReferralMap::iterator itr2 = referrals.find(pinfo->userid);
			if (itr2 != referrals.end())
			{
				int total2_delt = total_bonus_withdraw/3;
				if (itr2->second.bonus_total2 > INT_MAX-total2_delt)
					total2_delt = INT_MAX-itr2->second.bonus_total2;
				ptrans->total2 = total2_delt;
				itr2->second.bonus_total2 += total2_delt;
				itr2->second.SetDirty(true);
			}

			if (ptrans->TrySubmit())
			{
				Log::formatlog("reference", "role(id=%d) withdraw bonus %d\n", roleid, total_bonus_withdraw);
				transmap[pinfo->userid] = ptrans;
				return ERR_SUCCESS;
			}
			else
			{
				ptrans->Rollback();
				delete ptrans;
				return REF_ERR_SUBMITTODB;
			}
		}
		else
		{
			delete ptrans;
			return REF_ERR_NOBONUSAVAILABLE;
		}
	}
}

/*
int ReferenceManager::WithdrawBonus(int roleid)
{
	RoleInfoCache &cache = RoleInfoCache::Instance();
	GRoleInfo *referrer_role = cache.Get(roleid);
	if (referrer_role == NULL) return REF_ERR_REFERRERLOGOUT;

	ReferrerMap::iterator it = referrers.find(roleid);
	if (it == referrers.end())
		return REF_ERR_NOREFERRALFOUND;	
	else if (!it->second.IsLoaded())
		return REF_ERR_REFERRERNOTLOADED;
	else if (IsInTransaction(roleid))
		return REF_ERR_REFERRERINTRANSACTION;
	else
	{
		Referrer &ref = it->second;
		if (ref.IsEmpty()) return REF_ERR_NOREFERRALFOUND;
		Referrer::ReferralSet &referral_list = ref.referrals;
		Referrer::ReferralSet::iterator itr, ite = referral_list.end();
		WithdrawBonusTrans *ptrans = new WithdrawBonusTrans();
		int total_bonus_withdraw = 0;
		for (itr = referral_list.begin(); itr != ite; ++itr)
		{
			GRoleInfo *referral_info = cache.Get(*itr);
			ReferralMap::iterator itr2 = referrals.find(*itr);
			if (referral_info!=NULL && itr2!=referrals.end())
			{
				int referral_level = referral_info->level+referral_info->reborn_cnt*200;
				int bonus_withdraw_limit = BONUS_WITHDRAW_LIMIT(referral_level);
				if (bonus_withdraw_limit > itr2->second.bonus_total1+itr2->second.bonus_total2)
					bonus_withdraw_limit = itr2->second.bonus_total1+itr2->second.bonus_total2;
				int referral_bonus_avail = bonus_withdraw_limit-itr2->second.bonus_withdraw; 

				if (referral_bonus_avail > 0)
				{
					total_bonus_withdraw += referral_bonus_avail;
					ptrans->referral_withdraws[*itr] = itr2->second.bonus_withdraw;
					itr2->second.bonus_withdraw += referral_bonus_avail;
					itr2->second.SetDirty(true);
				}
			}
		}

		if (total_bonus_withdraw > 0)
		{
			ptrans->referrer = roleid;
			ptrans->total_withdraw = ref.bonus_withdraw;
			ptrans->to_withdraw = total_bonus_withdraw;
			ref.bonus_withdraw += total_bonus_withdraw;
			ref.SetDirty(true);

			ReferralMap::iterator itr2 = referrals.find(roleid);
			if (itr2 != referrals.end())
			{
				ptrans->total2 = itr2->second.bonus_total2;
				itr2->second.bonus_total2 += total_bonus_withdraw/3;
				itr2->second.SetDirty(true);
			}

			if (ptrans->TrySubmit())
			{
				Log::formatlog("reference", "role(id=%d) withdraw bonus %d\n", roleid, total_bonus_withdraw);
				transmap[roleid] = ptrans;
				return REF_ERR_SUCCESS;
			}
			else
			{
				ptrans->Rollback();
				delete ptrans;
				return REF_ERR_SUBMITTODB;
			}
		}
		else
		{
			delete ptrans;
			return REF_ERR_NOBONUSAVAILABLE;
		}
	}
}
*/
void ReferenceManager::OnDBWithdrawConfirm(int userid)
{
	WithdrawTransMap::iterator it = transmap.find(userid);
	if (it != transmap.end())
	{
		if (it->second) 
		{
			it->second->Confirm();
			delete it->second;
		}
		transmap.erase(it);
	}
}

void ReferenceManager::OnDBWithdrawRollback(int userid)
{
	WithdrawTransMap::iterator it = transmap.find(userid);
	if (it != transmap.end())
	{
		if (it->second) 
		{
			it->second->Rollback();
			delete it->second;
		}
		transmap.erase(it);
	}
}
/*
void WithdrawExpTrans::Rollback()
{
	ReferenceManager *refman = ReferenceManager::GetInstance();
	Referrer *pref = refman->GetReferrer(referrer);
	if (pref != NULL) 
	{
		pref->exp_withdraw_today = total_withdraw;
		pref->SetDirty(false);
	}

	Referral *pfal = refman->GetReferral(referrer);
	if (pfal != NULL) 
	{
		pfal->exp_total2 = total2;
		pfal->SetDirty(false);
	}

	ReferralOldWithdrawMap::iterator it, ie = referral_withdraws.end();
	for (it = referral_withdraws.begin(); it != ie; ++it)
	{
		pfal = refman->GetReferral(it->first);
		if (pfal != NULL) 
		{
			pfal->exp_withdraw = it->second;
			pfal->SetDirty(false);
		}
	}
}
*/
void WithdrawBonusTrans::Rollback()
{
	ReferenceManager *refman = ReferenceManager::GetInstance();
	Referrer *pref = refman->GetReferrer(referrer);
	if (pref != NULL) 
	{
		//pref->bonus_withdraw = total_withdraw;
		pref->bonus_withdraw -= to_withdraw;
		pref->SetDirty(false);
	}

	Referral *pfal = refman->GetReferral(referrer);
	if (pfal != NULL) 
	{
		//pfal->bonus_total2 = total2;
		pfal->bonus_total2 -= total2;
		pfal->SetDirty(false);
	}

	ReferralWithdrawdMap::iterator it, ie = referral_withdraws.end();
	for (it = referral_withdraws.begin(); it != ie; ++it)
	{
		pfal = refman->GetReferral(it->first);
		if (pfal != NULL) 
		{
			//pfal->bonus_withdraw = it->second;
			pfal->bonus_withdraw -= it->second;
			pfal->bonus_withdraw_today -= it->second;
			pfal->SetDirty(false);
		}
	}
}
/*
void WithdrawExpTrans::OnConfirm()
{
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(referrer);
	if (pinfo != NULL)
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, SendRefAddExp(referrer, to_withdraw));
}
*/
void WithdrawBonusTrans::OnConfirm()
{
	LOG_TRACE("roleid add bonus %d to GS   1", to_withdraw);
	PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	{
		LOG_TRACE("roleid %d add bonus %d to GS   2", roleid, to_withdraw);
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, SendRefAddBonus(roleid, to_withdraw));
	}
}

};
