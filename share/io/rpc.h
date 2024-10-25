#ifndef __RPC_H
#define __RPC_H

#include <map>

#include "thread.h"
#include "marshal.h"
#include "protocol.h"
#include "timer.h"
#include "maperaser.h"

namespace GNET
{

class Rpc : public Protocol
{
public:
	struct Data : public Marshal
	{
		virtual ~Data() { }
		virtual Data *Clone() const = 0;
		virtual void Destroy() { delete this; }
		virtual Data& operator = (const Data &rhs) { return *this; }
	};
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
		XID(const XID& rhs) : id(rhs.id) { }
		unsigned int GetId() const { return id; }
		void SetId(unsigned int _id) { id = _id; }
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
			std::vector<Rpc *> tmp;
			{
				Thread::RWLock::WRScoped l(locker_map);
				MapEraser<Map> e(map);
				for(Map::iterator it = map.begin(), ie = map.end(); it != ie; ++it)
				{
					Rpc *rpc = (*it).second;
					if (!rpc->TimePolicy(rpc->timer.Elapse()))
					{
						tmp.push_back(rpc);
						e.push(it);
					}
				}
			}
			for ( std::vector<Rpc *>::iterator it = tmp.begin(), ie = tmp.end(); it != ie; ++it )
			{
				(*it)->Timeout();
				(*it)->Destroy();
			}	
		}
	};

protected:
	typedef std::map<XID, Rpc*> Map;
	static Thread::RWLock locker_map;
	static HouseKeeper housekeeper;
	static Map map;
	virtual void Timeout() { OnTimeout(); }
public:
	Data *argument;
	Data *result;
	XID  xid;
	Timer timer;

public:
	~Rpc ()
	{
		argument->Destroy();
		result->Destroy();
	}
	Rpc(Type type, Data *arg, Data *res) : Protocol(type), argument(arg), result(res) { }
	Rpc(const Rpc &rhs) : Protocol(rhs), argument(rhs.argument->Clone()), result(rhs.result->Clone()), xid(rhs.xid) { }
	OctetsStream& marshal(OctetsStream &os) const
	{
		return os << xid << (xid.IsRequest() ? *argument : *result); 
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		os >> xid;
		if (xid.IsRequest()) return os >> *argument;
		Thread::RWLock::RDScoped l(locker_map);
		Map::iterator it = map.find(xid);
		return it == map.end() ? os : os >> *((*it).second->result);
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (xid.IsRequest())
		{
			Server(argument, result, manager, sid);
			xid.ClrRequest();
			manager->Send(sid, this);
			return;
		}
		Rpc *rpc;
		{
		Thread::RWLock::WRScoped l(locker_map);
		Map::iterator it = map.find(xid);
		if (it == map.end()) return;
		rpc = (*it).second;
		map.erase(it);
		}
		rpc->Client(rpc->argument, rpc->result, manager, sid);
		rpc->Destroy();
	}

	virtual void Server(Data *argument, Data *result, Manager *manager, Manager::Session::ID sid)
	{ Server(argument,result); }
	virtual void Client(Data *argument, Data *result, Manager *manager, Manager::Session::ID sid)
	{ Client(argument,result); }
	virtual void Server(Data *argument, Data *result) {  }
	virtual void Client(Data *argument, Data *result) {  }
	virtual void OnTimeout() { OnTimeout(argument); }
	virtual void OnTimeout(Data *argument) { }
	virtual bool TimePolicy(int timeout) const { return timeout < 5; }

	static Rpc *Call(Type type, const Data *arg)
	{
		Rpc *rpc = (Rpc *)Protocol::Create(type);
		rpc->xid.SetRequest();
		if(arg)
			*rpc->argument = *arg;
		rpc->timer.Reset();

		Thread::RWLock::WRScoped l(locker_map);
		Map::iterator it = map.find(rpc->xid);
		if (it != map.end())
		{
			(*it).second->Destroy();
			(*it).second = rpc;
		}
		else
		{
			map.insert(std::make_pair(rpc->xid, rpc));
		}

		return rpc;
	}

	static Rpc *Call(Type type, const Data &arg) { return Call(type, &arg); }

	////////////////////////////////////////////////////
	// helper

	// argument wrapper
	template < typename DataType >
	struct Argument : public Data
	{
		typedef DataType data_type;

		DataType data;

		Argument() { /* for create seed */ }
		explicit Argument(const data_type & _data) : data(_data) { }
		virtual Data *Clone() const { return new Argument(*this); }

		virtual Data& operator = (const Data &rhs)
		{
			const Argument * o = dynamic_cast<const Argument *>(&rhs);
			if (o && o != this) data = o->data;
			return *this;
		}

		virtual OctetsStream& marshal(OctetsStream & os) const { os << data; return os; }
		virtual const OctetsStream& unmarshal(const OctetsStream & os) { os >> data; return os; }
	};
	static Rpc *Call(Type type, const Octets & arg) { return Call(type, Argument<Octets>(arg)); }

	// None wrapper
	struct NoneData : public Marshal
	{
		virtual OctetsStream& marshal(OctetsStream & os) const { return os; }
		virtual const OctetsStream& unmarshal(const OctetsStream & os) { return os; }
	};

	static Rpc *Call(Type type) { return Call(type, Argument<NoneData>()); }

	// result wrapper
	template < typename DataType >
	struct Result : public Data
	{
		typedef DataType data_type;

		int retcode;
		DataType data;

		virtual Data *Clone() const { return new Result(*this); }

		virtual Data& operator = (const Data &rhs)
		{
			const Result * o = dynamic_cast<const Result *>(&rhs);
			if (o && o != this)
			{
				retcode = o->retcode;
				data = o->data;
			}
			return *this;
		}

		virtual OctetsStream& marshal(OctetsStream & os) const { os << retcode << data; return os; }
		virtual const OctetsStream& unmarshal(const OctetsStream & os) { os >> retcode >> data; return os; }
	};
};

};

#endif
