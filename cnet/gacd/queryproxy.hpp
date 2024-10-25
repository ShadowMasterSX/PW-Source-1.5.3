#ifndef __GNET_QUERYPROXY_HPP
#define __GNET_QUERYPROXY_HPP

#include <vector>
#include <map>

#include "thread.h"
#include "acq"
#include "codereschecker.hpp"

namespace GNET
{

struct CodeResCheckerFromClient : public CodeResChecker
{
    int m_iSessionID;
    CodeResCheckerFromClient(int iSessionID) : m_iSessionID(iSessionID) { }
    virtual void DoCheck(WaitingCode *pWC, UserSessionData *pUser, int iRes, bool &bCommit);
	virtual CodeResChecker* Clone() { return new CodeResCheckerFromClient(*this); }
    virtual ~CodeResCheckerFromClient() { }
};

class QueryProxy 
{
	static QueryProxy instance;
	int xid;
	Thread::Mutex xid_locker;
	int GetXID();
	Thread::Mutex locker;
	struct Res
	{
		std::vector<std::pair<short, int> > res;
		bool operator==(const Res &r) const
		{
			size_t n = res.size();
			if( n != r.res.size() ) return false;
			for(size_t i=0; i<n; ++i)
				if( res[i].first != r.res[i].first || res[i].second != r.res[i].second ) return false;
			return true;
		}
		bool operator<(const Res &r) const
		{
			size_t n = res.size();
			if( n < r.res.size() ) return true;
			if( n > r.res.size() ) return false;
			for(size_t i=0; i<n; ++i)
			{
				if( res[i].first < r.res[i].first ) return true;
				if( res[i].first > r.res[i].first ) return false;
				if( res[i].second < r.res[i].second ) return true;
				if( res[i].second > r.res[i].second ) return false;
			}
			return false;
		}
		void Add(short code, int _res)
		{
			res.push_back(std::make_pair(code, _res));
		}
	};
	struct Waiting
	{
		int timeout;
		unsigned int sid;
		ACQ acq;
		std::map<int, Res> waiting;
	};
	typedef std::map<unsigned int, Waiting> WaitingMap;
	WaitingMap waiting_map;
public:
	void OnTimer();
	void CheckRes(int xid, int uid, short code_id, int res);
	void SendQuery(ACQ acq, unsigned int sid, const IntOctetsVector &params);
	static QueryProxy *GetInstance() { return &instance; }
private:
	QueryProxy() : xid(1), xid_locker("xid_locker"), locker("proxy_code_locker") { }
};

};
#endif
