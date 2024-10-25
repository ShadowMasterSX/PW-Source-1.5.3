#include "sellid"
#include "sellpointinfo"
#include "selldataimage.h"
#include "log.h"
namespace GNET
{
bool SellInfoQuery::Update(StorageEnv::Transaction& txn, Octets& key, Octets& value)
{
	Marshal::OctetsStream key_os;
    key_os = key;
	SellID sellid;
	try
	{
		key_os>>sellid;
		if ( sellid.sellid>m_maxSellID ) m_maxSellID=sellid.sellid;
		m_mapSeller[sellid.roleid].attendlist.insert(sellid.sellid);
	}
	catch ( Marshal::Exception & )
	{
		Log::log(LOG_ERR,"SellInfoQuery: marshal err.\n");
	}
	return true;
}
SellDataImage::SellDataImage() : m_maxSellID_(0) 
{
	m_blInit_=false;
}
bool SellDataImage::Init()
{
	SellInfoQuery q;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( "sellpoint" );
		StorageEnv::AtomTransaction txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
			cursor.walk( q );
			m_maxSellID_=q.m_maxSellID;
			m_mapSeller_.swap( q.m_mapSeller );
			DEBUG_PRINT("init SellDataImage. maxSellID=%d,rolenum=%d\n",m_maxSellID_,m_mapSeller_.size());
			return m_blInit_=true;
		}
		catch ( DbException e ) { throw; }
		catch ( ... )
		{
			DbException ee( DB_OLD_VERSION );
			txn.abort( ee );
			throw ee;
		}
	}
	catch ( DbException e )
	{
		Log::log( LOG_ERR, "init SellDataImage, error when walk, what=%s\n", e.what() );
	}
	return false;
}

void SellDataImage::OnAddSell( roleid_t roleid,sellid_t sellid )
{
	Thread::Mutex::Scoped l(m_lock_);
	_addsell( roleid,sellid );
}
void SellDataImage::OnRmvSell( roleid_t roleid,sellid_t sellid )
{
	Thread::Mutex::Scoped l(m_lock_);
	_rmvsell( roleid,sellid );
}
bool SellDataImage::GetAttendList( roleid_t roleid,AttendList& list )
{
	Thread::Mutex::Scoped l(m_lock_);
	if ( !m_blInit_ ) return false;
	list=m_mapSeller_[roleid].attendlist;
	return true;
}
///////////////////////////////////////
///    BELOW IS PRIVATE FUNCTION    ///
///////////////////////////////////////
void SellDataImage::_addsell( roleid_t roleid,sellid_t sellid )
{
	if (!m_blInit_) return;
	m_mapSeller_[roleid].attendlist.insert(sellid);
}
void SellDataImage::_rmvsell( roleid_t roleid,sellid_t sellid )
{
	if (!m_blInit_) return;
	m_mapSeller_[roleid].attendlist.erase(sellid);
}

}// end of namespace
