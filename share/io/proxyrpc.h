#ifndef __PROXYRPC_H
#define __PROXYRPC_H

#include <map>

#include "marshal.h"
#include "rpc.h"
#include "protocol.h"
#include "maperaser.h"

namespace GNET
{

class ProxyRpc : public Protocol
{
protected:
	class XID : public Marshal
	{
		union
		{
			struct
			{
				unsigned int count:31;
				unsigned int is_request:1;
			};
			unsigned int id;
		};
		static unsigned int xid_count;
		static Thread::Mutex locker_xid;
	public:
		XID() : id(0) { }
		OctetsStream& marshal(OctetsStream &os) const { return os << id; }
		const OctetsStream& unmarshal(const OctetsStream &os) { return os >> id; }
		bool IsRequest() const { return is_request; }
		void ClrRequest() { is_request = 0; }
		void SetRequest()
		{
			is_request = 1;
			Thread::Mutex::Scoped l(locker_xid);
			count = xid_count++;
		}
		XID& operator =(const XID &rhs) { if (&rhs != this) id = rhs.id; return *this; }
		bool operator < (const XID &rhs) const { return count < rhs.count; }
	};

	class HouseKeeper : public Timer::Observer
	{
	public:
		HouseKeeper() { Timer::Attach(this); }
		void Update()
		{
			Thread::RWLock::WRScoped l(locker_map);
			MapEraser<Map> e( map );
			for(Map::iterator it = map.begin(), ie = map.end(); it != ie; ++it)
			{
				ProxyRpc *rpc = (*it).second;
				if (!rpc->TimePolicy(rpc->timer.Elapse()))
				{
					rpc->OnTimeout();
					rpc->Destroy();
					e.push(it);
				}
			}
		}
	};
private:
	typedef std::map<XID, ProxyRpc*> Map;
	static Thread::RWLock locker_map;
	static Map map;
protected:
	static HouseKeeper housekeeper;
	Manager *proxy_manager;
	Manager::Session::ID proxy_sid;
	ProxyRpc::XID proxy_xid;
	OctetsStream proxy_data;

	ProxyRpc::XID xid;
	Timer timer;

public:
	~ProxyRpc ()
	{
	}

	ProxyRpc(Type type) : Protocol(type), proxy_manager(NULL) { }
	ProxyRpc(Type type, Rpc::Data *argument, Rpc::Data *result)
				: Protocol(type), proxy_manager(NULL) { }
	ProxyRpc(const ProxyRpc &rhs) : Protocol(rhs),
				proxy_manager(rhs.proxy_manager), proxy_sid(rhs.proxy_sid), proxy_xid(rhs.proxy_xid),
				/*proxy_data(rhs.proxy_data),*/ xid(rhs.xid)
	{
		proxy_data.swap(remove_const(rhs.proxy_data));
	}
	Protocol *Clone() const { return new ProxyRpc(*this); }

	OctetsStream& marshal(OctetsStream &os) const
	{
		os << xid;
		size_t xidsize = sizeof(unsigned int);
		if( proxy_data.size() > xidsize )
			os.insert( os.end(), ((char*)proxy_data.begin())+xidsize, proxy_data.size()-xidsize );
		return os;
	}

	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		remove_const(os).swap(proxy_data);
		proxy_data >> xid;
		return os;
	}

	void SetResult( const Rpc::Data * res )
	{
		proxy_data.clear();
		proxy_data << proxy_xid << *res;
	}

	void SetResult( const Rpc::Data & res)
	{
		SetResult(&res);
	}
	
	void SetArgument(const Rpc::Data *arg)
	{
		proxy_data.clear();
		proxy_data << xid << *arg;
	}

	void SetArgument(const Rpc::Data & arg)
	{
		SetArgument(&arg);
	}

	void SendToSponsor( )
	{
		if( proxy_manager )
		{
			xid = proxy_xid;
			proxy_manager->Send( proxy_sid, *this );
		}
	}

	virtual bool Delivery(Manager::Session::ID proxy_sid, const OctetsStream &osArg) { return false; }
	virtual bool Delivery(Manager::Session::ID proxy_sid, 		OctetsStream &osArg) { return Delivery(proxy_sid,(const OctetsStream &) osArg); }
	virtual void PostProcess(Manager::Session::ID proxy_sid, const OctetsStream &osArg, const OctetsStream &osRes) { }
	virtual void OnTimeout() { proxy_data>>xid; OnTimeout(proxy_data); }
	virtual void OnTimeout(const OctetsStream &osArg) { }
	virtual bool TimePolicy(int timeout) const { return timeout < 5; }

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (xid.IsRequest())
		{
			xid.ClrRequest();
			proxy_manager = manager;
			proxy_sid     = sid;
			proxy_xid     = xid;

			xid.SetRequest();	// reset xid

			if( Delivery( proxy_sid, proxy_data ) )
			{
				Thread::RWLock::WRScoped l(locker_map);
				Map::iterator it = map.find(xid);
				if (it != map.end())
				{
					(*it).second->Destroy();
					(*it).second = (ProxyRpc*)Clone();
				}
				else
				{
					map.insert(std::make_pair(xid, (ProxyRpc*)Clone()));
				}
			}
			return;
		}
		
		ProxyRpc *rpc;
		{
		Thread::RWLock::WRScoped l(locker_map);
		Map::iterator it = map.find(xid);
		if (it == map.end()) return;
		rpc = (*it).second;
		map.erase(it);
		}

		proxy_manager = rpc->proxy_manager;
		proxy_sid = rpc->proxy_sid;
		proxy_xid = rpc->proxy_xid;

		{
			XID tmp;
			rpc->proxy_data >> xid;
		}
		PostProcess( proxy_sid, rpc->proxy_data, proxy_data );
		SendToSponsor( );

		rpc->Destroy();
	}
};

};

#endif

