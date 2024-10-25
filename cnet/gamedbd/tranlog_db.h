/* @file: tranlog_db.h
 * @description: write log into db4, interface is compatible with tranlog.h
 */ 

#ifndef __GNET_TRANLOG_DB_H
#define __GNET_TRANLOG_DB_H
// DB version of record and tranlog
#include <string>
#include "octets.h"
#include "marshal.h"
#include "storage.h"
#include "errcode.h"

#include "tranlog_i.h"

namespace GNET
{
template<typename Key, typename Value>
class Record
{
	#pragma pack(1)
	struct Layout
	{
		Key	key;
		Value	value;
	};
	#pragma pack()
	Layout	layout;
public:
	typedef Key   key_type;
	typedef Value value_type;
	Record() { }
	const Key& GetKey() const { return layout.key; }
	const Value& GetValue() const { return layout.value; }
	operator Value& () { return layout.value; }
	void SetDirty() { return; }
	void CheckPoint( ) {
		return;
	}
	bool Load( Octets& key, Octets& value )
	{
		try {
			Marshal::OctetsStream(key) >> layout.key;
			Marshal::OctetsStream(value) >> layout.value;
			layout.key.UpdateSerial();
			return true;
		}
		catch ( Marshal::Exception& )
		{
			return false;
		}
	}
	static Record Alloc( )
	{
		Record record;
		record.layout.key = Key::Alloc();
		return record;
	}
};

template<typename Record>
class TransLog
{
public:
	typedef typename Record::key_type key_type;
	typedef std::map< key_type, Record > Map;
protected:
	Map map;
	std::string table_name;
	void _LoadRecord() { }
public:
	~TransLog() { }
	TransLog( std::string name ) : table_name(name) { }

	void CheckPoint() { }

	typename Map::iterator Alloc()
	{
		Record record;
		while (map.find((record = Record::Alloc()).GetKey()) != map.end());
		return map.insert( map.end(), std::make_pair( record.GetKey(), record ) );
	}
	Map& GetMap() { return map; }
};

typedef Record<LogKey, LogValue> SellRecord;
class SellInventory : public TransLog<SellRecord>
{
	class LoadRecordQuery : public StorageEnv::IQuery
	{
		public:	
			LoadRecordQuery( SellInventory::Map& map ) : m_map(map) { }
			bool Update(StorageEnv::Transaction& txn, Octets& key, Octets& value)
			{
				SellRecord record;
				if ( record.Load(key,value) )
					if ( ((SellRecord::value_type&)record).status!=SellRecord::value_type::_ST_ABORT_END &&
							((SellRecord::value_type&)record).status!=SellRecord::value_type::_ST_COMMIT_END
					   )
						m_map.insert( std::make_pair( record.GetKey(), record ) );
				return true;
			}
		public:
			SellInventory::Map& m_map;		
	};
	protected:
		void _LoadRecord() //derive from parent
		{
			try {
				LoadRecordQuery q( GetMap() );
				StorageEnv::Storage * ptrans = StorageEnv::GetStorage( table_name.c_str() );
				StorageEnv::AtomTransaction txn;
				StorageEnv::Storage::Cursor cursor = ptrans->cursor( txn );
				cursor.walk( q );
			}
			catch ( DbException e ) { throw; }	
		}
	public:
		SellInventory(std::string table_name) : TransLog<SellRecord>(table_name) { m_blInit_=false; }	
		bool Init() {
			try {
				_LoadRecord();
			}
			catch ( ... ) {
				return m_blInit_=false;
			}	
			return m_blInit_=true;
		}
		int Flush(Map::iterator it,StorageEnv::Storage * ptrans,StorageEnv::Transaction& txn) {
			try {
				//StorageEnv::Storage * ptrans = StorageEnv::GetStorage( table_name.c_str() );
				SellRecord &record=(*it).second;
				ptrans->insert( 
						Marshal::OctetsStream()<<record.GetKey(),
						Marshal::OctetsStream()<<record.GetValue(),
						txn
					);	
				return ERR_SUCCESS;
			}
			catch ( DbException e ) {
				return e.get_errno();
			}
		}
	private:
		bool m_blInit_;		
};

} // end of namespace

#endif
