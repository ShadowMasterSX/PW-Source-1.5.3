#include "gamedbserver.hpp"
#include "transman.h"
#include "transbuypoint.hpp"
#include <utility>
#include "log.h"
#define UPDATE_INTERVAL 10
#define REQUEST_TIMEOUT 120

#define _findrecord(key) m_sellInv_.GetMap().find(key)
#define _record_end()    m_sellInv_.GetMap().end()
namespace GNET
{
/* Initialize m_sellInv_ and handle un-done transactions */  
bool TransMan::Init(int zoneid,int aid)
{
	m_zoneid_=zoneid;
	m_aid_=aid;
	SellRecord::key_type::Init( zoneid );
	//restore incomplete transactions
	if ( !m_sellInv_.Init() )
	{
		Log::log(LOG_ERR,"TransMan::init transaction data failed.\n");
		return false;
	}
	InvIterator it=m_sellInv_.GetMap().begin(),ite=m_sellInv_.GetMap().end();
	Log::formatlog("sellpoint","TransMan::Init: size(%d)\n",m_sellInv_.GetMap().size());
	for ( ;it!=ite;++it )
	{
		_dump_record( it->second );
		SellRecord::value_type& value=(*it).second;
		if ( value.status==SellRecord::value_type::_ST_BEGIN  ||
			 value.status==SellRecord::value_type::_ST_ABORT  ||
		 	 value.status==SellRecord::value_type::_ST_COMMIT )
		{
			//printf("\t===>add to attend list.\n");
			m_mapAttendList_[value.buyer ].insert( it->first );
			m_mapAttendList_[value.seller].insert( it->first );
		}
		if ( value.status==SellRecord::value_type::_ST_BEGIN )
			_add_pending_task( it );
	}
	//IntervalTimer::Attach(this,UPDATE_INTERVAL*1000000/IntervalTimer::Resolution()); 
	Timer::Attach(this); 
	return true;
}
/* Add new transaction */
int TransMan::AddTrans_NoLock(int buyer,int seller,int price,int point, int sellid,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn)
{
	int retcode;
	InvIterator it=m_sellInv_.Alloc();
	if ( it!=_record_end() )
	{
		(SellRecord::value_type&)it->second=SellRecord::value_type(buyer,seller,price,point,sellid);
		if ( (retcode=m_sellInv_.Flush(it,ptrans,txn))!=ERR_SUCCESS ) {
			m_sellInv_.GetMap().erase(it);
			return retcode;
		}	
		it->second.SetDirty();
		_add_pending_task( it );
		_send2au( (*it).second );
		Log::formatlog("sellpoint","AddTrans: transid(z:%d,s:%d),sellid(r:%d,s:%d),buyer=%d,price=%d,point=%d,aid=%d\n",
				(*it).second.GetKey().zoneid,(*it).second.GetKey().serial,
				seller,sellid,
				buyer,price,point,m_aid_);
		_dump_record( it->second );
		//update attend list
		m_mapAttendList_[buyer ].insert( it->first );
		m_mapAttendList_[seller].insert( it->first );
		return ERR_SUCCESS;
	}
	else
	{
		txn.abort( DbException(0) );
		return ERR_NOFREESPACE;
	}
}
int TransMan::AddTrans(int buyer,int seller,int price,int point, int sellid,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn)
{
	Thread::Mutex::Scoped l(m_lock_);
	return AddTrans_NoLock(buyer,seller,price,point,sellid,ptrans,txn);
}
/* Update transaction status */
int TransMan::UpdateTrans_NoLock( const TransKey& key,int newstatus,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn )
{
	SellRecord* record=NULL;
	int old_status,retcode=ERR_SUCCESS;

	if ( newstatus>=SellRecord::value_type::_ST_MAX )
	{
		txn.abort(DbException(0));
		return ERR_SP_INVA_STATUS; //invalid status
	}
	InvIterator it=_findrecord(key);
	record= (it==_record_end()) ? NULL : &it->second;
	if ( record==NULL || 
		 !((SellRecord::value_type&)*record).ValidNewStatus(newstatus) ) 
	{
		txn.abort(DbException(0));
		return ERR_SP_INVA_STATUS;
	}
	//write to storage
	old_status=((SellRecord::value_type&)*record).status;
	((SellRecord::value_type&)*record).status=newstatus;
	if ( (retcode=m_sellInv_.Flush(it,ptrans,txn))!=ERR_SUCCESS )
	{
		((SellRecord::value_type&)*record).status=old_status;
		return retcode;
	}
	record->SetDirty();
	_remove_pending_task(it);
	Log::formatlog("sellpoint","UpdateTrans(%d->%d): transid(z:%d,s:%d),sellid(r:%d,s:%d),buyer=%d,price=%d,point=%d,aid=%d\n",
				old_status,newstatus,
				(*it).second.GetKey().zoneid,(*it).second.GetKey().serial,
				(*it).second.GetValue().seller,(*it).second.GetValue().sellid,
				(*it).second.GetValue().buyer,(*it).second.GetValue().price,
				(*it).second.GetValue().point,
				m_aid_);
	_dump_record( it->second );
	return ERR_SUCCESS;
}
int TransMan::UpdateTrans( const TransKey& key,int newstatus,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn )
{
	Thread::Mutex::Scoped l(m_lock_);
	return UpdateTrans_NoLock( key,newstatus,ptrans,txn );
}
/* CheckPoint SellInventory */
bool TransMan::CheckPoint_NoLock()
{
	m_sellInv_.CheckPoint();
	return true;
}
bool TransMan::CheckPoint()
{
	Thread::Mutex::Scoped l(m_lock_);
	return CheckPoint_NoLock();
}
/* Get SellRecord */
bool TransMan::GetSellRecord_NoLock( const SellRecord::key_type& key,SellRecord& record )
{
	InvIterator it=_findrecord( key );
	if (it!=_record_end()) record=it->second;
	return (it!=_record_end());
}
bool TransMan::GetSellRecord( const SellRecord::key_type& key,SellRecord& record )
{
	Thread::Mutex::Scoped l(m_lock_);
	return GetSellRecord_NoLock( key,record );
}
/* Handle incomplete transaction of a role */   
int TransMan::OnRoleLogin( int roleid,int* money_get,std::vector<int>& delsell,StorageEnv::Transaction& txn,StorageEnv::Storage * ptrans,Memento* mem )
{
	Thread::Mutex::Scoped l(m_lock_);
	return OnRoleLogin_NoLock( roleid,money_get,delsell,txn,ptrans,mem );
}
int TransMan::OnRoleLogin_NoLock( int roleid,int* money_get,std::vector<int>& delsell,StorageEnv::Transaction& txn,StorageEnv::Storage * ptrans,Memento* mem )
{
	int retcode=ERR_SUCCESS;
	std::vector<TransKey> del_attend_key;
	AttendList::iterator it;
	TransList::iterator itlst,itelst;

	*money_get=0;
	it=m_mapAttendList_.find( roleid );
	if ( it==m_mapAttendList_.end() ) return ERR_SUCCESS;
	DEBUG_PRINT("TransMan::OnRoleLogin: role=%d,attend.size=%d\n",roleid,(*it).second.size());
	//traverse all attend transaction
	if (mem) mem->roleid=roleid;
	for ( itlst=(*it).second.begin(),itelst=(*it).second.end();
		  itlst!=itelst;
		  ++itlst )
	{
		TransKey key=*itlst;
		InvIterator itsell=m_sellInv_.GetMap().find( key );
		if ( itsell!=m_sellInv_.GetMap().end() )
		{
			SellRecord &record=itsell->second;
			switch ( record.GetValue().status )
			{
			case SellRecord::value_type::_ST_ABORT:	
				if ( roleid==record.GetValue().buyer )
				{
	DEBUG_PRINT("TransMan::OnRoleLogin: role=%d, as buyer, getmoney=%d,tid(z:%d,s:%d),sellid(r:%d,s:%d)\n",
					roleid,record.GetValue().price,
					record.GetKey().zoneid,record.GetKey().serial,
					record.GetValue().seller,record.GetValue().sellid);
					
					if ( *money_get+record.GetValue().price>MAX_PACKAGE_MONEY )
						break;
					*money_get+=record.GetValue().price; //return money to buyer
					if ( mem ) mem->trans_status_map[key]=record.GetValue().status;
					((SellRecord::value_type&)record).status=SellRecord::value_type::_ST_ABORT_END;
					if ( (retcode=m_sellInv_.Flush( itsell,ptrans,txn ))!=ERR_SUCCESS )
					{
	Log::log(LOG_ERR,"TransMan::OnRoleLogin. flush record failed. retcode=%d,roleid=%d, tid(z:%d,s:%d), sellid(r:%d,s:%d),buyer=%d,price=%d,point=%d,status=%d\n",
								retcode,roleid,
								record.GetKey().zoneid,record.GetKey().serial,
								record.GetValue().seller,record.GetValue().sellid,
								record.GetValue().buyer,record.GetValue().price,
								record.GetValue().point,record.GetValue().status);
						return retcode;
					}
					del_attend_key.push_back(key);
				}
				break;
			case SellRecord::value_type::_ST_COMMIT:
				if ( roleid==record.GetValue().seller )
				{
	DEBUG_PRINT("TransMan::OnRoleLogin: role=%d, as seller, getmoney=%d,tid(z:%d,s:%d),sellid(r:%d,s:%d)\n",
					roleid,record.GetValue().price,
					record.GetKey().zoneid,record.GetKey().serial,
					record.GetValue().seller,record.GetValue().sellid);
	
					if ( *money_get+record.GetValue().price>MAX_PACKAGE_MONEY )
						break;
					*money_get+=record.GetValue().price; //give money to seller
					if ( mem ) mem->trans_status_map[key]=record.GetValue().status;
					((SellRecord::value_type&)record).status=SellRecord::value_type::_ST_COMMIT_END;
					if ( (retcode=m_sellInv_.Flush( itsell,ptrans,txn ))!=ERR_SUCCESS )
					{
						Log::log(LOG_ERR,"TransMan::OnRoleLogin. flush record failed. retcode=%d,roleid=%d, tid(z:%d,s:%d), sellid(r:%d,s:%d),buyer=%d,price=%d,point=%d,status=%d\n",
								retcode,roleid,
								record.GetKey().zoneid,record.GetKey().serial,
								record.GetValue().seller,record.GetValue().sellid,
								record.GetValue().buyer,record.GetValue().price,
								record.GetValue().point,record.GetValue().status);
						return retcode;
					}
					del_attend_key.push_back(key);
					//if transaction is in _ST_COMMIT,
					//sellinfo is already del from "sell" table, so announce it to delivery to synchronize
					delsell.push_back( record.GetValue().sellid ); 
				}
				break;	
			case SellRecord::value_type::_ST_ABORT_END:
			case SellRecord::value_type::_ST_COMMIT_END:
				del_attend_key.push_back(key);
				break;	
			}
		}
		else
			del_attend_key.push_back(key);
	}
	//remove complete transaction from attendlist
	for ( size_t i=0;i<del_attend_key.size();++i )
	{
		(*it).second.erase( del_attend_key[i] );	
	}
	return retcode;
}
void TransMan::RestoreMemento(Memento* mem)
{
	if ( !mem ) return;
	Thread::Mutex::Scoped l(m_lock_);
	RestoreMemento_NoLock(mem);
}
void TransMan::RestoreMemento_NoLock(Memento* mem)
{
	Memento::Map::iterator it,ite;
	InvIterator itsell;
	for ( it=mem->trans_status_map.begin(),ite=mem->trans_status_map.end();
		  it!=ite; ++it )
	{	
		itsell=m_sellInv_.GetMap().find( (*it).first );
		if ( itsell!=m_sellInv_.GetMap().end() )
		{
			((SellRecord::value_type&)(*itsell).second).status=(*it).second;
			m_mapAttendList_[mem->roleid].insert( it->first );
		}
	} 
}
/* Timer::Observer::Update */
void TransMan::Update()
{
	//struct timeval now;
	time_t now;
	std::vector<PendingTransMap::iterator> expire_set;

	Thread::Mutex::Scoped l(m_lock_);
	//IntervalTimer::GetTime(&now);
	now=Timer::GetTime();
	PendingTransMap::iterator it=m_mapPendingTrans_.begin(),ite=m_mapPendingTrans_.end();
	for ( ;it!=ite && (*it).first<=now;++it )
		expire_set.push_back( it );
	//printf("Expired request(%d,%d):\n",expire_set.size(),m_mapPendingTrans_.size());
	for ( size_t i=0;i<expire_set.size();++i )
	{
		InvIterator itinv=expire_set[i]->second;
		_dump_record( itinv->second );
		_remove_pending_task( itinv );
		_send2au( itinv->second );
		_add_pending_task( itinv );
	}
	expire_set.clear();
#if 0
	//check point every interval, only for test purpose
	m_sellInv_.CheckPoint();
	printf("\n\n");
#endif
}
//////////////////////////////////////////
///  BELOW IS PRIVATE FUNCTIONS        /// 
//////////////////////////////////////////
bool TransMan::_send2au( const SellRecord& record )
{
	//send to AU ??
	printf(" send to AU ..... \n");
	return GameDBServer::GetInstance()->Send2Delivery(
			TransBuyPoint(
				TransID(record.GetKey().zoneid,record.GetKey().serial),
				SellID(record.GetValue().seller,record.GetValue().sellid),
				record.GetValue().buyer,
				record.GetValue().price,
				record.GetValue().point,
				m_aid_
			)
		);
}
bool TransMan::_add_pending_task( const InvIterator& it )
{
	time_t now;
	now=Timer::GetTime();
	int expire_time=now+REQUEST_TIMEOUT;
	m_mapPendingKeys_[it->first]=m_mapPendingTrans_.insert( std::make_pair(expire_time,it) );
	printf("_add_pending_task:(%d,%d),expire_time=%d,now=%d\n",
			it->first.zoneid,it->first.serial,(int)expire_time,(int)now);
	return true;
}
bool TransMan::_remove_pending_task( const InvIterator& it )
{
	PendingKeyMap::iterator itpk=m_mapPendingKeys_.find( it->first );
	if ( itpk!=m_mapPendingKeys_.end() )
	{
		m_mapPendingTrans_.erase( (*itpk).second );
		m_mapPendingKeys_.erase(itpk);
		printf("_remove_pending_task:(%d,%d)\n",it->first.zoneid,it->first.serial);
		return true;
	}
	return false;
}
void TransMan::_dump_record( const SellRecord& record )
{
	printf("Dump record(%d,%d):\n",record.GetKey().zoneid,record.GetKey().serial);
	printf("\tBuyer:%d,Seller:%d,Price:%d,Point:%d,SellId:%d,Status:%d\n",
			record.GetValue().buyer,record.GetValue().seller,record.GetValue().price,
			record.GetValue().point,record.GetValue().sellid, record.GetValue().status
		  );
}

}

