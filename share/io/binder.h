#ifndef __GNET_BINDER_H
#define __GNET_BINDER_H

#ifdef USE_ZLIB
#include <zlib.h>
#endif

#include "protocol.h"
#include "log.h"
#include "statistic.h"
#include "itimer.h"
#include "mppc.h"

#define PROTOCOL_BINDER		0
#define PROTOCOL_COMPRESSBINDER	16
namespace GNET
{
class AbstractBinder : public Protocol
{
public:
	typedef std::vector<Protocol *> ProtocolVector;
	typedef std::vector<Octets> OctetsVector;
protected:
	ProtocolVector pv;
	OctetsVector ov;
	size_t total;
	size_t maxsize;
	mutable Thread::RWLock locker_vector;
	
	virtual Protocol* Clone() const = 0;
public:
	explicit AbstractBinder(Type type, size_t max) : Protocol(type), maxsize(max), locker_vector("Binder::locker_vector")
   	{ total=0; }
	AbstractBinder(const AbstractBinder &rhs)
		: Protocol(rhs) , total(rhs.total), maxsize(rhs.maxsize), locker_vector("Binder::locker_vector")
	{
		Thread::RWLock::RDScoped l(rhs.locker_vector);
		std::transform(rhs.pv.begin(), rhs.pv.end(), std::back_inserter(pv), std::mem_fun(&Protocol::Clone));
	}
	virtual ~AbstractBinder()
	{
		clear();
	}
	virtual void clear()
	{
		Thread::RWLock::WRScoped l(locker_vector);
		std::for_each(pv.begin(), pv.end(), std::mem_fun(&Protocol::Destroy));
		pv.clear();
		ov.clear();
		total = 0;
	}
	virtual void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped l(locker_vector);
		for (ProtocolVector::const_iterator it = pv.begin(), ie = pv.end(); it != ie; ++it)
		{
			bool err_log = false;
			try
			{
				(*it)->Process(manager,sid);
			}
			catch (Protocol::Exception &)
			{
				err_log = true;
				manager->Close(sid);
			}
			if ( err_log )
				Log::log(LOG_ERR,"Binder::Process: process protocol error. sid=%d",sid);
		}
	}
	size_t bind(const Octets & oct)
	{
		Thread::RWLock::WRScoped l(locker_vector);
		ov.push_back(oct);
		total += oct.size();
		return oct.size();
	}
	size_t bind(const Protocol *p) { return bind(p->Encode()); }
	size_t bind(const Protocol &p) { return bind(p.Encode()); }
	int  PriorPolicy() const { return 1; }
	bool SizePolicy(size_t size) const { return size <= maxsize; }
	size_t size() const { return total; }
};

class ProtocolBinder : public AbstractBinder
{
	Protocol *Clone() const { return new ProtocolBinder(*this); }
public:
	explicit ProtocolBinder(Type type=PROTOCOL_BINDER, size_t maxsize = 0) : AbstractBinder(type, maxsize){ }
	ProtocolBinder(const ProtocolBinder &rhs) : AbstractBinder(rhs) { }
	OctetsStream& marshal(OctetsStream &os) const
	{
		Thread::RWLock::RDScoped l(locker_vector);
		for(OctetsVector::const_iterator it = ov.begin(), ie = ov.end(); it != ie; ++it)
			os.push_byte( (const char*)(*it).begin(), (*it).size() );
		return os;
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		Manager::Session::Stream &is = (Manager::Session::Stream &)(os);
		Thread::RWLock::WRScoped l(locker_vector);
		for (Protocol *p; (p = Protocol::Decode(is)); pv.push_back(p));
		return os;
	}
};

class CompressBinder : public AbstractBinder
{
	
	Protocol *Clone() const { return new CompressBinder(*this); }
public:
	explicit CompressBinder(Type type=PROTOCOL_COMPRESSBINDER, size_t maxsize = 0) : AbstractBinder(type, maxsize) { }
	CompressBinder(const CompressBinder &rhs) : AbstractBinder(rhs) { }
	OctetsStream& marshal(OctetsStream &os) const
	{
		Marshal::OctetsStream os_src;
		Octets os_com;

		{
			Thread::RWLock::RDScoped l(locker_vector);
			os_src.reserve(total);

			for(OctetsVector::const_iterator it = ov.begin(), ie = ov.end(); it != ie; ++it)
			{
				os_src.push_byte( (const char*)(*it).begin(), (*it).size() );
			}
		}

		unsigned int len_src = os_src.size();
#ifdef USE_ZLIB
		uLong len_com =  compressBound(len_src);
		os_com.reserve(len_com);
		bool success = ( Z_OK == compress((Bytef*)os_com.begin(),&len_com,(Bytef*)os_src.begin(),len_src)
			&& len_com<len_src );
#else
		unsigned int len_com =  mppc::compressBound(len_src);
		os_com.reserve(len_com);
		bool success;
		if( len_src <= 8192 )
		{
			success = ( 0 == mppc::compress((unsigned char*)os_com.begin(),(int*)&len_com,
									(const unsigned char*)os_src.begin(),len_src)
				&& len_com<len_src);
		}
		else
		{
			success = ( 0 == mppc::compress2((unsigned char*)os_com.begin(),(int*)&len_com,
									(const unsigned char*)os_src.begin(),len_src)
				&& len_com<len_src);
		}
#endif
		if( success )
		{
			os.reserve( 2 * sizeof(int) + len_com + os.size() );
			os << CompactUINT(len_src) << CompactUINT(len_com);
			os.push_byte( (const char*)os_com.begin(), len_com );
			STAT_MIN5("CompressSrcSize",len_src);
			STAT_MIN5("CompressComSize",len_com);
			return os;
		}

		// no compress
		os.reserve( 2 * sizeof(int) + len_src + os.size() );
		os << CompactUINT(len_src) << CompactUINT(len_src);
		os.push_byte( (const char*)os_src.begin(), len_src );
		return os;
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		Manager::Session::Stream &is = (Manager::Session::Stream &)(os);

		size_t	len_src, len_com;
		is >> CompactUINT(len_src) >> Marshal::Begin >> CompactUINT(len_com);
		if( len_com < len_src )
		{
			is >> Marshal::Rollback;

			Marshal::OctetsStream	os_com;
			Manager::Session::Stream is_src(is.session);

			os_com.reserve(len_com+8);
			is >> os_com;
			is_src.reserve( len_src );
#ifdef USE_ZLIB
			if (Z_OK == uncompress((Bytef*)is_src.begin(),(uLongf*)&len_src,
								(const Bytef*)os_com.begin(),os_com.size()))
#else
			if ( (len_src<=8192 && 0 == mppc::uncompress((unsigned char*)is_src.begin(),(int*)&len_src,
								(const unsigned char*)os_com.begin(),os_com.size()))
				|| (len_src>8192 && 0 == mppc::uncompress2((unsigned char*)is_src.begin(),(int*)&len_src,
								(const unsigned char*)os_com.begin(),os_com.size())) )
#endif
			{
				((Octets&)is_src).resize( len_src );

				Thread::RWLock::WRScoped l(locker_vector);
				Protocol * p;
				while( !is_src.eos() )
				{
					p = Protocol::Decode(is_src);
					if( NULL == p  )
						break;
					pv.push_back(p);
				}
			}
			else
			{
				Log::log(LOG_ERR,"CompressBinder::unmarshal uncompress failed.len_src=%u,len_com=%u,ossize=%u",
						len_src, len_com, os.size() );
			}
		}
		else
		{
			is >> Marshal::Commit;

			Thread::RWLock::WRScoped l(locker_vector);
			Protocol * p;
			while( !is.eos() )
			{
				p = Protocol::Decode(is);
				if( NULL == p  )
					break;
				pv.push_back(p);
			}
		}

		return os;
	}
};

};
#endif
