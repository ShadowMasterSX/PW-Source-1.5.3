#ifndef _REFERENCEMANAGER_H_o
#define _REFERENCEMANAGER_H_o

#include <map>
#include <set>
#include <vector>
#include "itimer.h"

/*
线上推广系统因平台存在bug外服未启用
Update函数中写盘与WithdrawBonusTrans写盘可能存在时序问题 liuguichen 20121210
*/
namespace GNET
{
class GReferral;
class GReferrer;
class ReferralBrief;
class RefTrans;
class UserInfo;

class CountedDirty
{
	int dirty;
public:
	CountedDirty():dirty(0) {}
	void SetDirty(bool _dirty)
	{
		if (_dirty) 
			++dirty;
		else if (dirty > 0)
			--dirty;
	}
	bool IsDirty() { return dirty>0; }
};

class Referral					//下线信息
{
	int userid;
	int referrer;				//上线userid
	int bonus_total1; 		//由于本人消耗元宝对上线的历史总贡献鸿利值
	int bonus_total2; 		//由于本人下线消耗元宝对本人上线的历史总贡献鸿利值
	int bonus_withdraw; 		//上线从该玩家已经提取的鸿利值
	int bonus_withdraw_today; 		//上线今日从该玩家已经提取的鸿利值
	int max_role_level;             //贡献过红利的角色中等级最高的角色的等级数
	std::vector<Octets> namelist;   //贡献过红利的角色的名称构成的列表
	CountedDirty dirty;
	bool logout;
	bool loaded;
	friend class ReferenceManager;
	friend class WithdrawBonusTrans;

public:
	Referral():userid(0),referrer(0),bonus_total1(0),bonus_total2(0),bonus_withdraw(0),
				bonus_withdraw_today(0),max_role_level(0),logout(false),
				loaded(false)
	{
	}
	void OnLoad(const GReferral &referral, int _referrer);
	void SetDirty(bool _dirty) { dirty.SetDirty(_dirty); }
	bool IsDirty() { return dirty.IsDirty(); }
	void SetLogout(bool _logout) { logout = _logout; }
	bool IsLogout() { return logout; }
	void OnUseCash(int cashused, int level, Octets rolename);
	void SyncDB();
	void OnLogin();
	void SetLoaded(bool _loaded) { loaded = _loaded; }
	bool IsLoaded() { return loaded; }
	void ToGReferral(GReferral &gref);
private:
	void CheckBonusWithdrawToday();
};

class Referrer					//上线信息
{
	typedef std::set<int> ReferralSet;
	ReferralSet referrals;
	int userid;
	int bonus_withdraw;			//上线历史上从所有下线提取的总的鸿利值
	bool loaded;				//上线信息未加载
	CountedDirty dirty;
	friend class ReferenceManager;
	friend class WithdrawBonusTrans;
	
public:
	Referrer():userid(0), bonus_withdraw(0), loaded(false)
	{
	}
	void OnLoad(const GReferrer &referrer);
	void AddReferral(int _userid) { referrals.insert(_userid); }
	void DelReferral(int _userid) { referrals.erase(_userid); }
	bool IsEmpty() { return referrals.empty(); }
	void SetLoaded(bool _loaded) { loaded = _loaded; }
	bool IsLoaded() { return loaded; }
	void SetDirty(bool _dirty) { dirty.SetDirty(_dirty); }
	bool IsDirty() { return dirty.IsDirty(); }
	void SyncDB();
	//void OnLogin();
	void ToGReferrer(GReferrer &gref);
private:
//	void CheckExpWithdrawToday();
};

class ReferenceManager : public IntervalTimer::Observer
{
	enum {
		ST_OPEN = 0x000001,
	};
	enum{ CHECKNUM_ONUPDATE = 10 };
	typedef std::map<int, Referrer> ReferrerMap;
	ReferrerMap referrers;
	typedef std::map<int, Referral> ReferralMap;
	ReferralMap referrals;
	typedef std::map<int, RefTrans *> WithdrawTransMap;		//以上线userid∈挛駇ap
	WithdrawTransMap transmap;
	typedef std::map<int, Octets> RefCodeMap;
	RefCodeMap refcodemap;
	int referrer_cursor;
	int referral_cursor;
	int trans_cursor;
	int status;
	static ReferenceManager instance;

	ReferenceManager():referrer_cursor(0),referral_cursor(0),trans_cursor(0),status(0) { }

public:
	static ReferenceManager *GetInstance() { return &instance; }

	bool Initialize();
	void OnLogin(UserInfo * userinfo);
	void OnLogout(UserInfo * userinfo);
	void OnLoadReferrer(const GReferrer &referrer);
	void OnLoadReferral(const GReferral &referral);
	void OnReferralUseCash(int roleid, int cashused, int level);
	int  ListReferrals(int roleid, int start_index, int &total, int &bonus_avail_today, std::vector<ReferralBrief> &referrals);
	void OnDBUpdateReferrer(int userid);
	void OnDBUpdateReferral(int userid);
	void OnDBWithdrawConfirm(int userid);
	void OnDBWithdrawRollback(int userid);
	bool Update();
	bool IsInTransaction(int referrer) { return transmap.find(referrer)!=transmap.end(); }
	void InsertRefCode(int roleid, const Octets & code);
	bool GetRefCode(int roleid, Octets & code);
	Referrer *GetReferrer(int userid)
	{
		ReferrerMap::iterator it = referrers.find(userid);
		if (it != referrers.end())
			return &it->second;
		else 
			return NULL;
	}
	Referral *GetReferral(int userid)
	{
		ReferralMap::iterator it = referrals.find(userid);
		if (it != referrals.end())
			return &it->second;
		else
			return NULL;
	}
	int WithdrawBonus(int roleid);
	int WithdrawBonusTest(int roleid, int bonus_add);
private:
	int BonusWithdrawLimit(int level);
};

};
#endif
