#ifndef __GNET_TRANSMAN_H
#define __GNET_TRANSMAN_H
/* @file: transman.h
 * @description: define a interface to manager transactions,such as search,update,fault-restore,etc.
 */ 
#include "mutex.h"
//#include "itimer.h"
#include "timer.h"
#include <map>
#include <set>
//#include "tranlog.h"
#include "tranlog_db.h"
namespace GNET
{
	class TransMan : public Timer::Observer
	{
	public:	
		typedef SellRecord::key_type                     TransKey;
		struct Memento {
			typedef std::map<TransKey,int> Map;
			int roleid;
			Map trans_status_map;
		};
	private:
		typedef SellInventory::Map::iterator             InvIterator;
		typedef int                                      ExpireTime;
		typedef std::set<TransKey>                       TransList;
		typedef std::map<int,TransList>                  AttendList; /* player's attending list. roleid->TransKeyList */ 
		typedef std::multimap<ExpireTime,InvIterator>             PendingTransMap; 
		typedef std::map<TransKey,PendingTransMap::iterator>      PendingKeyMap;
	private:	
		TransMan(std::string storage_name) : m_lock_("TransMan::m_lock_"),m_sellInv_( storage_name ) { 
			m_zoneid_=0;
			m_aid_=0;
			// Attach to Timer Observer list in Init()
		}
		~TransMan() {
			Thread::Mutex::Scoped l(m_lock_);
			m_sellInv_.CheckPoint();
		}
	public:	
		static TransMan& GetInstance(std::string storage_name="") {
			static TransMan instance(storage_name);
			return instance;
		}
		/* Initialize m_sellInv_ and handle un-done transactions */  
		bool Init(int zoneid,int aid);
		/* Add new transaction */
		int AddTrans(int buyer,int seller,int price,int point, int sellid,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn);
		int AddTrans_NoLock(int buyer,int seller,int price,int point, int sellid,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn);
		/* Update transaction status */
		int UpdateTrans( const TransKey& key,int newstatus,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn );
		int UpdateTrans_NoLock( const TransKey& key,int newstatus,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn );
		/* CheckPoint SellInventory,only valid in file-translog mode */
		bool CheckPoint();
		bool CheckPoint_NoLock();
		/* Get SellRecord */
		bool GetSellRecord( const SellRecord::key_type& key,SellRecord& record );
		bool GetSellRecord_NoLock( const SellRecord::key_type& key,SellRecord& record );
		/* Handle incomplete transaction of a role */
		int OnRoleLogin(int roleid,int* money_get,std::vector<int>& delsell,StorageEnv::Transaction& txn,StorageEnv::Storage * ptrans,Memento* mem=NULL);	
		int OnRoleLogin_NoLock(int roleid,int* money_get,std::vector<int>& delsell,StorageEnv::Transaction& txn,StorageEnv::Storage * ptrans,Memento* mem=NULL);	


		//memento related functions
		Memento* CreateMemento() { return new Memento(); }
		void ReleaseMemento(Memento* mem) { delete mem; }
		void RestoreMemento(Memento* mem);
		void RestoreMemento_NoLock(Memento* mem);
		
		/* itimer::Update. Internal locked */
		void Update();

		Thread::Mutex& GetLock() { return m_lock_; }
	private:
		// transaction related function
		bool  _send2au( const SellRecord& record );
		bool  _add_pending_task( const InvIterator& it );
		bool  _remove_pending_task( const InvIterator& it );
		void  _dump_record( const SellRecord& record );
	private:
		Thread::Mutex m_lock_;
		SellInventory m_sellInv_;
		PendingTransMap  m_mapPendingTrans_;
		PendingKeyMap    m_mapPendingKeys_;
		AttendList       m_mapAttendList_;

		int m_zoneid_;
		int m_aid_;
	};
}
#endif

