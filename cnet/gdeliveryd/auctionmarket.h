/* @file: auctionmarket.h
 * @description: Define a auction manager, which manages all auctions' timeout, add, delete.
 *
 * @date: 2005-11-1
 * @last-modified: 2005-11-1
 * @author: liping
 */ 

#ifndef __GNET_AUCTIONHOUSE_H
#define __GNET_AUCTIONHOUSE_H
#include "itimer.h"
#include "mutex.h"
#include <map>
#include <set>
#include <utility>  //for std::pair
#include "gauctiondetail"

#define  AUCTIONID_MIN 1
#define  AUCTIONID_MAX 10000000
#define  MAX_ATTEND_AUCTION 32  //MAX number of auction one player can attend
#define  MAX_TIMEOUT_PERTICK 1
namespace GNET
{
class AuctionObj
{
public:	
	enum EraseReason
	{
		_ER_TIMEOUT,
		_ER_CANCEL,
		_ER_BID_SUCC,
	};
	typedef std::set<int/*roleid*/> AttenderSet;
public:	
	AuctionObj(const GAuctionDetail& auction)
	{
		m_auction_=auction;
		m_setAttender_.insert(m_auction_.info.seller);
		m_setAttender_.insert(m_auction_.info.bidder);
	}	
	~AuctionObj() { }
	void OnErase(EraseReason reason); //send protocols
	
	int  GetEndTime() { return m_auction_.info.end_time;   }
	int  GetSeller()  { return m_auction_.info.seller; }
	int  GetBidder()  { return m_auction_.info.bidder; }
	int  GetPrice()   { return m_auction_.info.bidprice; }
	int  GetBinprice(){ return m_auction_.info.binprice; }
	void SetEndTime( int time ) { m_auction_.info.end_time=time; }
	void SetNewPrice( int price,int new_bidder ) { m_auction_.info.bidprice=price; m_auction_.info.bidder=new_bidder; }

	operator const GAuctionDetail& () const { return m_auction_; }
	operator       GAuctionDetail& ()       { return m_auction_; }
	const GAuctionItem& GetAuctionInfo() const { return m_auction_.info; }
		  GAuctionItem& GetAuctionInfo()       { return m_auction_.info; }

	AttenderSet&   GetAttenderSet() { return m_setAttender_; }	  
	void           PutNewAttender(int roleid) { m_setAttender_.insert(roleid); }
	
	bool CanClose()
	{
		struct timeval now;
		IntervalTimer::GetTime(&now);
		return (m_auction_.info.end_time-now.tv_sec > 7200);
	}
private:	
	AttenderSet     m_setAttender_;
	GAuctionDetail  m_auction_;
};	
class AuctionMarket : public IntervalTimer::Observer
{
public:
	typedef unsigned short   category_t;
	typedef unsigned int     auctionid_t;
	struct  Compare
	{
		int         m_time;
		auctionid_t m_aid;
		Compare(int _t,auctionid_t _aid) : m_time(_t),m_aid(_aid) { }
		bool operator()( const std::pair<int,auctionid_t>& rhs )
		{ return rhs.first==m_time && rhs.second==m_aid; }
	};
	typedef std::set<auctionid_t>                         IDSet;
	typedef std::multimap<int,auctionid_t>                TimeMap;
	typedef std::multimap<category_t,AuctionObj>          CategoryMap;
	typedef std::map<auctionid_t,CategoryMap::iterator>   AuctionIDMap;
	typedef std::map<int,IDSet>                           RoleAuctionMap; //map roleid to all auctions this role intend
	typedef std::map<int,unsigned short>                  ClassifyMap;
	struct find_param_t{
		category_t category;
		unsigned int item_id;
		unsigned int handle;
		bool blForward;
		find_param_t( category_t c,unsigned int i,unsigned int h,bool f ) :
			category(c),item_id(i),handle(h),blForward(f) { }
	};
	// additional typedef
	typedef std::vector<GAuctionItem>   GAuctionItemVector;
	typedef std::vector<GAuctionDetail> GAuctionDetailVector;
private:
	AuctionMarket() : m_blInit_(false),m_lockermap_("AuctionMarket::lockermap")
	{
		ResetMap();
	}
	void ResetMap()
	{
		m_mapAuctionID_.clear();
		m_mapTime_.clear();
		m_mapCategory_.clear();
		m_mapRoleAuction_.clear();
		m_maxAuctionID_=0;
		/*
		m_setValidID_.clear();
	   	for ( auctionid_t id=AUCTIONID_MIN;id<AUCTIONID_MAX;++id )
			m_setValidID_.insert( id );
		*/	
	}	
public:
	static AuctionMarket& GetInstance() { static AuctionMarket instance; return instance; }
	~AuctionMarket() { }
	// initial
	bool IsMarketOpen() { return m_blInit_; }
	void InitAuctionObjs(GAuctionDetailVector& list,bool blFinish=false);
    // manager AuctionObjs
	bool AddAuction( category_t category,const GAuctionDetail& auction );
	bool UpdateAuction( category_t category,const GAuctionDetail& auction );
	bool RmvAuction( auctionid_t aid );
	bool ExtendTime( auctionid_t aid,int extend_time );
	bool UpdatePrice(auctionid_t aid,int new_price,int new_bidder); 
	bool GetAuction( auctionid_t aid,GAuctionDetail& item );
	int  ValidPrice(auctionid_t aid, unsigned int price, char bin, unsigned int& newprice);
			
	int  GetAuctionID(); //generate a new auctionid
	void GetAuctionList( const find_param_t& param,GAuctionItemVector& result,unsigned int& end ); //search auction list by category and item id
	bool ExitAuction( int roleid,auctionid_t aid ); //exit a auction, but roleid MUST NOT a seller or bidder
	int  GetAttendAuctionNum( int roleid );
	bool GetAttendAuctionList( int roleid,GAuctionItemVector& auc_list); //get all auctions a player attending
	bool CanAuctionClose(auctionid_t aid);//测试拍卖是否可以取消
	// timer update
	bool Update();
	bool Initialize();
	unsigned short GetCategory(int item) { return m_mapClassify[item]; }
	bool ValidCategory(int item, unsigned short category);

private:
	auctionid_t     m_maxAuctionID_;
	bool            m_blInit_;	
	Thread::Mutex   m_lockermap_;
	TimeMap         m_mapTime_;
	CategoryMap     m_mapCategory_;
	AuctionIDMap    m_mapAuctionID_;
	//IDSet           m_setValidID_;
	RoleAuctionMap  m_mapRoleAuction_;

	ClassifyMap     m_mapClassify;
};

}
#endif
