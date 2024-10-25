/* @file: selldataimage.h
 * @description: record information of point selling. such as max_sellid, participating list of a role, status of a sell  
 *
 */
#ifndef __GNET_SELLDATAIMAGE_H
#define __GNET_SELLDATAIMAGE_H
#include <set>
#include <map>
#include "storage.h"
#include "mutex.h"
namespace GNET
{
	typedef std::set<int> AttendList;
	struct seller_t
	{
		AttendList attendlist;
	};
	struct sellinfo_t
	{
		int sellid;
		int roleid;
		int status;
	};
	class SellInfoQuery : public StorageEnv::IQuery
	{
		typedef int roleid_t;
	public:	
		SellInfoQuery() : m_maxSellID(0) { }
		bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value );
	public:
		int m_maxSellID;
		std::map<roleid_t,seller_t> m_mapSeller;	
	};
	class SellDataImage
	{
		typedef int sellid_t;
		typedef int roleid_t;
		typedef std::map<roleid_t,seller_t> SellerMap;
		typedef std::map<sellid_t,sellinfo_t> SellInfoMap;
	private:
		SellDataImage();
	public:
		static SellDataImage& GetInstance()
		{
			static SellDataImage instance;
			return instance;
		}
		static int GetSellID() {
			if ( !GetInstance().m_blInit_ ) return -1; //-1 means invalid sellid
			return ++GetInstance().m_maxSellID_;
		}
		bool Init();
		void OnAddSell( roleid_t roleid,sellid_t sellid );
		void OnRmvSell( roleid_t roleid,sellid_t sellid );

		bool GetAttendList( roleid_t roleid,AttendList& list );
	public:
		void _addsell( roleid_t roleid,sellid_t sellid );
		void _rmvsell( roleid_t roleid,sellid_t sellid );	
	private:
		Thread::Mutex m_lock_;
		SellerMap m_mapSeller_;

		bool m_blInit_;
		int  m_maxSellID_;
	};
}
#endif
