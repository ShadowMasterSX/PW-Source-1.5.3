#ifndef __GNET_TRANLOG_H
#define __GNET_TRANLOG_H
#include <string>
#include <unistd.h>
#include <map>
#include <algorithm>
#include <fcntl.h>
#include "octets.h"
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
	off_t	position;
	bool	dirty;
public:
	typedef Key   key_type;
	typedef Value value_type;
	Record() { }
	const Key& GetKey() const { return layout.key; }
	const Value& GetValue() const { return layout.value; }
	operator Value& () { return layout.value; }
	void SetDirty() { dirty = true; }
	void CheckPoint( int fd )
	{
		if ( dirty )
			pwrite( fd, (const void *)&layout, sizeof(layout), position );
		dirty = false;
	}
	bool Load( int fd )
	{
		position = lseek( fd, 0, SEEK_CUR );
		dirty = false;
		if ( read( fd, (void *)&layout, sizeof(layout)) != sizeof(layout) )
			return false;
		layout.key.UpdateSerial();
		return true;
	}
	static Record Alloc( int fd )
	{
		Record record;
		record.layout.key = Key::Alloc();
		record.position = lseek(fd, 0, SEEK_END);
		record.dirty = true;
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
	int fd;
	void _LoadRecord()
	{
		Record record;
		while ( record.Load( fd ) )
			map.insert( std::make_pair( record.GetKey(), record ) );
	}
public:
	~TransLog() { close(fd); }
	TransLog( std::string path ) : fd( open(path.c_str(), O_CREAT|O_RDWR, 0600) )
	{
		//this->_LoadRecord();
	}

	void CheckPoint()
	{
		for ( typename Map::iterator it = map.begin(), ie = map.end(); it != ie; ++it )
			(*it).second.CheckPoint( fd );
		fsync( fd );
	}

	typename Map::iterator Alloc()
	{
		Record record;
		while (map.find((record = Record::Alloc( fd )).GetKey()) != map.end());
		record.CheckPoint( fd );
		return map.insert( map.end(), std::make_pair( record.GetKey(), record ) );
	}

	Map& GetMap() { return map; }
};
typedef Record<LogKey, LogValue> SellRecord;
class SellInventory : public TransLog<SellRecord>
{
	protected:
		void _LoadRecord() //derive from parent
		{
			SellRecord record;
			while ( record.Load( fd )  )
				if ( ((SellRecord::value_type&)record).status!=SellRecord::value_type::_ST_ABORT_END &&
						((SellRecord::value_type&)record).status!=SellRecord::value_type::_ST_COMMIT_END
				   )
					map.insert( std::make_pair( record.GetKey(), record ) );
		}
	public:
		SellInventory(std::string filename) : TransLog<SellRecord>(filename) { 
		}	
		bool Init() {
			_LoadRecord();
			return true;
		}
		int Flush(Map::iterator it,StorageEnv::Transaction& txn) {
			return ERR_SUCCESS;
		}
};

} // end of namespace

#endif

