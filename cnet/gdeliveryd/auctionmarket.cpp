#include "macros.h"
#include "auctionmarket.h"
#include <algorithm>
#include "dbauctiontimeout.hrp"
#include "gamedbclient.hpp"
namespace GNET
{

void AuctionObj::OnErase(EraseReason reason) //send protocols	
{
	switch ( reason )
	{
	case AuctionObj::_ER_BID_SUCC:
		break;
	case AuctionObj::_ER_CANCEL:
		break;
	case AuctionObj::_ER_TIMEOUT:
		{
			GameDBClient::GetInstance()->SendProtocol(Rpc::Call( RPC_DBAUCTIONTIMEOUT,
				AuctionId(m_auction_.info.auctionid)));
		}
		break;
	}
}
// define AuctionMarket functions
void AuctionMarket::InitAuctionObjs(GAuctionDetailVector& list,bool blFinish)
{
	Thread::Mutex::Scoped l(m_lockermap_);

	if ( m_blInit_ )
	{
		ResetMap();
		m_blInit_=false;
	}
	for ( size_t i=0;i<list.size();++i )
	{
		GAuctionDetail& item = list[i];
		unsigned short category = GetCategory(item.info.itemid);
		if(!category)
		{
			Log::formatlog("auctioninit","put unknown item %d to category 1.", item.info.itemid); 
			item.category = 1;
		}else if(category!=item.category)
		{
			Log::formatlog("auctioninit","move item %d from category %d to %d.", 
				item.info.itemid, item.category, category); 
			item.category = category;
		}
		m_mapTime_.insert( std::make_pair(item.info.end_time,item.info.auctionid) );
		m_mapAuctionID_[item.info.auctionid]=m_mapCategory_.insert(
				std::make_pair( item.category,AuctionObj(item) ) 
			);
		m_mapRoleAuction_[item.info.seller].insert( item.info.auctionid );
		m_mapRoleAuction_[item.info.bidder].insert( item.info.auctionid );
		//m_setValidID_.erase(item.info.auctionid);
		if (item.info.auctionid>m_maxAuctionID_) m_maxAuctionID_=item.info.auctionid; 
	}
	if ( blFinish ) 
	{ 
		Log::formatlog("auction","initauction: total=%d maprole=%d", m_mapAuctionID_.size(), 
			m_mapRoleAuction_.size()); 
		m_blInit_=true; 
	}
}

bool AuctionMarket::AddAuction( category_t category,const GAuctionDetail& auction )
{
	if ( !m_blInit_ ) return false;
	Thread::Mutex::Scoped l(m_lockermap_);
	m_mapTime_.insert( std::make_pair(auction.info.end_time,auction.info.auctionid) );
	m_mapAuctionID_[auction.info.auctionid]=m_mapCategory_.insert(
			std::make_pair( category,AuctionObj(auction) ) 
		);
	m_mapRoleAuction_[auction.info.seller].insert( auction.info.auctionid );
	//m_setValidID_.erase(auction.info.auctionid);
	if (auction.info.auctionid>m_maxAuctionID_) m_maxAuctionID_=auction.info.auctionid; 
	return true;
}

bool AuctionMarket::UpdateAuction( category_t category,const GAuctionDetail& auction )
{
	if ( !m_blInit_ ) return false;

	Thread::Mutex::Scoped l(m_lockermap_);

	category = GetCategory(auction.info.itemid);

	// erase old in category map
	CategoryMap::iterator itc=m_mapCategory_.lower_bound(category),itce=m_mapCategory_.upper_bound(category);
	for ( ;itc!=itce;++itc )
		if ( (*itc).second.GetAuctionInfo().auctionid==auction.info.auctionid )
		{
			m_mapCategory_.erase( itc );
			break;
		}
	m_mapAuctionID_[auction.info.auctionid]=m_mapCategory_.insert(
			std::make_pair( category,AuctionObj(auction) ) 
		);
	m_mapRoleAuction_[auction.info.seller].insert( auction.info.auctionid );
	if ( auction.info.bidder!=0 )
		m_mapRoleAuction_[auction.info.bidder].insert( auction.info.auctionid );
	if (auction.info.auctionid>m_maxAuctionID_) m_maxAuctionID_=auction.info.auctionid; 
	return true;

}
bool AuctionMarket::RmvAuction( auctionid_t aid )
{
	if ( !m_blInit_ ) return false;
	Thread::Mutex::Scoped l(m_lockermap_);
	AuctionIDMap::iterator it_aid=m_mapAuctionID_.find( aid );
	if ( it_aid==m_mapAuctionID_.end() )
	{  
		Log::log(LOG_ERR,"auction %d not found when removing", aid);
		return false;
	}
	//erase from RoleAuction map
	AuctionObj::AttenderSet& attenders=(*it_aid).second->second.GetAttenderSet();
	AuctionObj::AttenderSet::iterator it_as=attenders.begin(),ite_as=attenders.end();
	for ( ;it_as!=ite_as;++it_as )
		m_mapRoleAuction_[ *it_as ].erase( aid );
	/*
	int bidder=(*it_aid).second->second.GetBidder();
	int seller=(*it_aid).second->second.GetSeller();
	m_mapRoleAuction_[bidder].erase( aid );
	m_mapRoleAuction_[seller].erase( aid );
	*/
	//erase from TimeMap
	int expire_time=(*it_aid).second->second.GetEndTime();
	TimeMap::iterator it_time,ite_time=m_mapTime_.upper_bound(expire_time);
	it_time=std::find_if(
				m_mapTime_.lower_bound(expire_time),
				ite_time,
				Compare(expire_time,aid)
			);
	if ( it_time!=ite_time ) m_mapTime_.erase(it_time);
	//erase from category map
	(*it_aid).second->second.OnErase(AuctionObj::_ER_BID_SUCC);
	m_mapCategory_.erase( (*it_aid).second );
	//erase from AuctionIDMap
	//m_setValidID_.insert( (*it_aid).first );
	m_mapAuctionID_.erase( it_aid );
	return true;
}
bool AuctionMarket::ExtendTime( auctionid_t aid,int extend_time )
{
	if ( !m_blInit_ ) return false;
	if ( extend_time==0 ) return true;
	Thread::Mutex::Scoped l(m_lockermap_);
	//erase old
	AuctionIDMap::iterator it_aid=m_mapAuctionID_.find( aid );
	if ( it_aid==m_mapAuctionID_.end() )
	{ 
		Log::log(LOG_ERR,"auction %d not found when extending", aid);
		return false;
	}
	int expire_time=(*it_aid).second->second.GetEndTime();
	TimeMap::iterator it_time,ite_time=m_mapTime_.upper_bound(expire_time);
	it_time=std::find_if(
				m_mapTime_.lower_bound(expire_time),
				ite_time,
				Compare(expire_time,aid)
			);
	if ( it_time!=ite_time ) m_mapTime_.erase(it_time);
	//insert new
	int new_time=expire_time+extend_time;
	m_mapTime_.insert( std::make_pair(new_time,aid) );
	(*it_aid).second->second.SetEndTime( new_time );
	return true;
}
bool AuctionMarket::UpdatePrice(auctionid_t aid,int new_price,int new_bidder)
{
	if ( !m_blInit_ ) return false;
	Thread::Mutex::Scoped l(m_lockermap_);
	AuctionIDMap::iterator it_aid=m_mapAuctionID_.find( aid );
	if ( it_aid==m_mapAuctionID_.end() )
	{
		Log::log(LOG_ERR,"auction %d not found when updating price", aid);
	   	return false;
	}
	(*it_aid).second->second.SetNewPrice( new_price,new_bidder );
	(*it_aid).second->second.PutNewAttender( new_bidder );
	m_mapRoleAuction_[new_bidder].insert( aid );
	return true;
}
bool AuctionMarket::GetAuction( auctionid_t aid,GAuctionDetail& item )
{	
	if ( !m_blInit_ ) return false;
	Thread::Mutex::Scoped l(m_lockermap_);
	AuctionIDMap::iterator it_aid=m_mapAuctionID_.find( aid );
	if ( it_aid==m_mapAuctionID_.end() ) return false;
	item=(*it_aid).second->second;
	return true;
}

int AuctionMarket::ValidPrice(auctionid_t aid, unsigned int price, char bin, unsigned int& newprice)
{
	if ( !m_blInit_ ) 
		return ERR_AS_MARKET_UNOPEN;
	Thread::Mutex::Scoped l(m_lockermap_);
	AuctionIDMap::iterator it_aid=m_mapAuctionID_.find( aid );
	if ( it_aid==m_mapAuctionID_.end() ) 
		return ERR_AS_BID_NOTFOUND;

	AuctionObj& obj = (*it_aid).second->second;
	if(bin)
	{
		newprice = obj.GetBinprice();
		if(price<newprice)
			return ERR_AS_BID_LOWBID;
		else if(price>newprice)
			return ERR_AS_BID_INVALIDPRICE;
	}
	else
	{	
		newprice = obj.GetPrice();
		if(newprice<100000)
		{
			if(price>200000)
				return ERR_AS_BID_INVALIDPRICE;
		}
		else if(price>2*newprice)
			return ERR_AS_BID_INVALIDPRICE;

		if(obj.GetBidder())
		{
			if(price < (unsigned int)(1.05*newprice))
				return ERR_AS_BID_LOWBID;
		}
		else if(price<newprice)
			return ERR_AS_BID_LOWBID;
	}
	return 0;
}

void AuctionMarket::GetAuctionList( const find_param_t& param,GAuctionItemVector& result,unsigned int& end )
{
	if ( !m_blInit_ ) return;
	Thread::Mutex::Scoped l(m_lockermap_);
	if(param.category==0)	//所有物品依次显示
	{
		size_t szFoundNum = m_mapAuctionID_.size();
		if(szFoundNum == 0)	return;
		AuctionIDMap::reverse_iterator it=m_mapAuctionID_.rbegin(), ie=m_mapAuctionID_.rend();
		size_t handle = param.handle;
		size_t szBegin=0,szLen=0;
		if(param.blForward && handle<szFoundNum)
		{
			szBegin = handle;
			szLen = (handle+_HANDLE_PAGESIZE)>szFoundNum ? szFoundNum-szBegin : _HANDLE_PAGESIZE;
			end = szBegin + szLen;
		}
		if(!param.blForward)
		{
			if ( handle>=szFoundNum ) handle=szFoundNum-1;
			szBegin = handle+1>_HANDLE_PAGESIZE ? handle-_HANDLE_PAGESIZE+1 : 0;
			szLen   = handle+1>_HANDLE_PAGESIZE ? _HANDLE_PAGESIZE : handle+1;
			end = szBegin-1;
		}
		std::advance(it,szBegin);
		for(size_t i=0;i<szLen;++it,++i)
		{
			result.push_back( it->second->second.GetAuctionInfo() );
		}
		return;
	/*	size_t begin = param.handle;
		AuctionIDMap::reverse_iterator it=m_mapAuctionID_.rbegin(), ie=m_mapAuctionID_.rend();

		if(begin>=m_mapAuctionID_.size())
			return;
		else if(begin)
			std::advance(it,begin);
		size_t count = std::distance(it,ie);
		count = count>_HANDLE_PAGESIZE ? _HANDLE_PAGESIZE : count;
		for(size_t i=0;i<count;++it,++i)
		{
			result.push_back( it->second->second.GetAuctionInfo() );
		}
		end = begin + count;
		return;*/
	}
	if(param.item_id != 0)	//指定的目录树内查找某一物品
	{
		if(!ValidCategory(param.item_id, param.category)) return;	
		CategoryMap::iterator it=m_mapCategory_.lower_bound(param.category),
							  ite=m_mapCategory_.upper_bound(param.category);
		size_t szFoundNum= ( it!=m_mapCategory_.end() && (*it).first==param.category ) ? std::distance(it,ite) : 0;
		if ( szFoundNum==0 ) return;
		size_t handle=param.handle;
		if(param.blForward && handle<szFoundNum)
		{
			std::advance(it,handle);
			size_t i,count;
			for (i=0,count=0;i<szFoundNum-handle && count<_HANDLE_PAGESIZE;++it,++i )
			{
				if((*it).second.GetAuctionInfo().itemid == param.item_id)
				{
					result.push_back( (*it).second.GetAuctionInfo() );
					++count; 
				}
			}
			end = handle + i;			
		}
		else if(!param.blForward)
		{
			if(handle >= szFoundNum) handle = szFoundNum-1;
			std::advance(it,handle);
			size_t i,count;
			for(i=0,count=0; i<handle+1 && count<_HANDLE_PAGESIZE; --it,++i )
			{
				if((*it).second.GetAuctionInfo().itemid == param.item_id)
				{
					result.insert(result.begin(), (*it).second.GetAuctionInfo());
					++count; 
				}
			}
			end = handle - i;			
		}		
		return;	
	}
	//指定目录树内依次显示
	{
	CategoryMap::iterator it=m_mapCategory_.lower_bound(param.category),
						  ite=m_mapCategory_.upper_bound(param.category);
	size_t szFoundNum= ( it!=m_mapCategory_.end() && (*it).first==param.category ) ? std::distance(it,ite) : 0;
	if ( szFoundNum==0 ) return;
	size_t handle=param.handle;
	size_t szBegin=0,szLen=0;
	if ( param.blForward && handle<szFoundNum )
	{
		szBegin=handle;
		szLen=(handle+_HANDLE_PAGESIZE)>szFoundNum ? szFoundNum-szBegin : _HANDLE_PAGESIZE;
		end = szBegin + szLen; 
	}
	if ( !param.blForward)
	{
		if ( handle>=szFoundNum ) handle=szFoundNum-1;
		szBegin = handle+1>_HANDLE_PAGESIZE ? handle-_HANDLE_PAGESIZE+1 : 0;
	    szLen   = handle+1>_HANDLE_PAGESIZE ? _HANDLE_PAGESIZE : handle+1;
		end = szBegin-1; 
	}
	std::advance(it,szBegin);
	for ( size_t i=0;i<szLen;++it,++i )
	{
		result.push_back( (*it).second.GetAuctionInfo() );
	}
	}
}
int AuctionMarket::GetAuctionID()
{
	if ( !m_blInit_ ) return _AUCTIONID_INVALID;
	/*
	Thread::Mutex::Scoped l(m_lockermap_);
	if ( m_setValidID_.size()==0 ) return _AUCTIONID_INVALID;
	int ret=*m_setValidID_.begin();
	m_setValidID_.erase( m_setValidID_.begin() );
	return ret;
	*/
	// ?? it is impossible AUCTIONID_MIN exist, when m_maxAuctionID_ reaches AUCTIONID_MAX
	// for the longest auction is 24 hours
	++m_maxAuctionID_;
	if ( m_maxAuctionID_>AUCTIONID_MAX ) m_maxAuctionID_=AUCTIONID_MIN; 
	return m_maxAuctionID_;
} 
int  AuctionMarket::GetAttendAuctionNum( int roleid )
{
	Thread::Mutex::Scoped l(m_lockermap_);
	RoleAuctionMap::iterator it=m_mapRoleAuction_.find( roleid );
	return it!=m_mapRoleAuction_.end() ? (*it).second.size() : 0;
}

bool AuctionMarket::ExitAuction( int roleid,auctionid_t aid ) //exit a auction, but roleid MUST NOT a seller or bidder
{
	if ( !m_blInit_ ) return false;
	Thread::Mutex::Scoped l(m_lockermap_);
	AuctionIDMap::iterator it_aid=m_mapAuctionID_.find( aid );
	if ( it_aid==m_mapAuctionID_.end() ) {
	   	return true; //no auction exist, return true
	}
	AuctionObj& auction=(*it_aid).second->second;
	if ( roleid==auction.GetSeller() || roleid==auction.GetBidder() ) return false;
	m_mapRoleAuction_[roleid].erase( aid );
	return true;
}
bool AuctionMarket::GetAttendAuctionList( int roleid,GAuctionItemVector& auc_list)
{
	Thread::Mutex::Scoped l(m_lockermap_);
	RoleAuctionMap::iterator it=m_mapRoleAuction_.find( roleid );
	if ( it!=m_mapRoleAuction_.end() )
	{
		IDSet& aid_list=(*it).second;
		IDSet::iterator s_it=aid_list.begin(),s_ite=aid_list.end();
		for ( ;s_it!=s_ite;++s_it )
		{
			AuctionIDMap::iterator ac_it=m_mapAuctionID_.find( *s_it );
			if ( ac_it!=m_mapAuctionID_.end() ) auc_list.push_back( ac_it->second->second.GetAuctionInfo() );
		}
		return true;
	}
	return false;
}
bool AuctionMarket::CanAuctionClose(auctionid_t aid)
{
	if ( !m_blInit_ ) return false;
	Thread::Mutex::Scoped l(m_lockermap_);
	AuctionIDMap::iterator it_aid=m_mapAuctionID_.find( aid );
	if ( it_aid==m_mapAuctionID_.end() ) {
	   	return false; 
	}
	AuctionObj& auction=(*it_aid).second->second;
	return auction.CanClose();
}
bool AuctionMarket::Update()
{
	Thread::Mutex::Scoped l(m_lockermap_);
	
	struct timeval now;
	IntervalTimer::GetTime(&now);
	AuctionIDMap::iterator it_aid;
	TimeMap::iterator it=m_mapTime_.begin(),it_erase;
	int counter=0;
	while ( it!=m_mapTime_.end() && counter<MAX_TIMEOUT_PERTICK )
	{
		it_erase=it;
		if ( (*it).first<=now.tv_sec )
		{
			++it;
			counter++;
			it_aid=m_mapAuctionID_.find( (*it_erase).second );
			if ( it_aid!=m_mapAuctionID_.end() )
			{
				AuctionObj::AttenderSet& attenders = (*it_aid).second->second.GetAttenderSet();
				AuctionObj::AttenderSet::iterator it_as=attenders.begin(),ite_as=attenders.end();
				for ( ;it_as!=ite_as;++it_as )
					m_mapRoleAuction_[ *it_as ].erase(it_aid->first);

				(*it_aid).second->second.OnErase(AuctionObj::_ER_TIMEOUT);
				m_mapCategory_.erase( (*it_aid).second );
				m_mapAuctionID_.erase( it_aid );
			}
			m_mapTime_.erase( it_erase );
		}
		else
			break;
	}
	return true;
}

bool AuctionMarket::Initialize()
{
	char buf[200];
	FILE *f = fopen("auctionid.txt", "r");

	if(!f)
		return false;
	m_mapClassify.clear();
	while( fgets(buf, 200, f) )
	{
		int idItem, idCategory;
		sscanf(buf, "%d%d", &idItem, &idCategory);
		m_mapClassify[idItem] = (unsigned short)idCategory;
	}
	fclose(f);
	IntervalTimer::Attach( this,500000/IntervalTimer::Resolution() ); 
	Log::formatlog("auction","initialize: total category=%d", m_mapClassify.size()); 
	return true;
}

bool AuctionMarket::ValidCategory(int item, unsigned short category) 
{ 
	ClassifyMap::iterator it = m_mapClassify.find(item);
	if(it!=m_mapClassify.end())
	{
		return it->second==category;
	}
	return false;
}

}
