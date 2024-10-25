/* @file: tranlog_i.h
 * @description: define tranlog key and value types
 */ 
#ifndef __GNET_TRANLOG_INLINE_H
#define __GNET_TRANLOG_INLINE_H
#include "marshal.h"
namespace GNET
{
class LogKey : public Marshal
{
	static unsigned int zid;
	static unsigned int max_serial;
public:
	unsigned int zoneid;
	unsigned int serial;

	static void Init( unsigned zoneid ) { zid = zoneid; }
	LogKey() : zoneid(zid) { }
	LogKey( unsigned int z, unsigned s ) : zoneid(z), serial(s) { }
	void UpdateSerial() const { max_serial = std::max(max_serial, serial); }
	bool operator < ( const LogKey& rhs ) const { 
		return zoneid<rhs.zoneid ? true : (zoneid==rhs.zoneid ? serial<rhs.serial:false); 
	}
	unsigned int GetSerial() const { return serial; }
	static LogKey Alloc() { return LogKey( zid, ++max_serial ); }
	OctetsStream& marshal(OctetsStream & os) const
	{
		os<<zoneid;
		os<<serial;
		return os;
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		os>>zoneid;
		os>>serial;
		return os;
	}
};

struct LogValue : public Marshal
{
	enum {
		_ST_BEGIN=0,
		_ST_COMMIT,
		_ST_ABORT,
		_ST_COMMIT_END,
		_ST_ABORT_END,
		_ST_MAX,
	};
	int     buyer;
	int     seller;
	int     price;   /* virtual Currency */
	int     point;   /* point num for sale */
	int     sellid;  /* size of cardnumber */
	int     status;
	LogValue( int _buyer=0,int _seller=0,int _price=0,int _point=0,int _sellid=0,int _status=_ST_BEGIN) :
		buyer(_buyer),seller(_seller),price(_price),point(_point),sellid(_sellid),status(_status) { 
	}
	bool ValidNewStatus( int newst )
	{
		return ((status==_ST_BEGIN && newst==_ST_COMMIT) ||
				(status==_ST_BEGIN && newst==_ST_ABORT)  ||
				(status==_ST_COMMIT && newst==_ST_COMMIT_END) ||
				(status==_ST_ABORT && newst==_ST_ABORT_END)
				) ? true : false;
	}
	OctetsStream& marshal(OctetsStream & os) const
	{
		os << buyer;
		os << seller;
		os << price;
		os << point;
		os << sellid;
		os << status;
		return os;
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		os >> buyer;
		os >> seller;
		os >> price;
		os >> point;
		os >> sellid;
		os >> status;
		return os;
	}
};
}
#endif
