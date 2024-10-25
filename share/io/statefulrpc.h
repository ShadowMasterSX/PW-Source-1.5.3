#ifndef __GNET_STATEFULRPC_H
#define __GNET_STATEFULRPC_H

#include "objectchangesupport.h"
#include "reference.h"
#include "taskgraph.h"
#include "transport.h"
#include "rpc.h"

namespace GNET
{

class StatefulRpc;
class RpcStateAdaptor
{
public:
	StatefulRpc *rpc;
	virtual ~RpcStateAdaptor() {}
};

class RpcServerAdaptor : public RpcStateAdaptor
{
	friend class StatefulRpc;
	Protocol::Manager *manager;
	Protocol::Manager::Session::ID sid;
	Thread::TaskGraph *graph;
	Thread::TaskGraph::Node *start_node;
public:
	~RpcServerAdaptor()
	{
		manager->Send(sid, (Rpc *)rpc);
		((Rpc *)rpc)->Destroy();
	}
	void SetStartNode(Thread::TaskGraph::Node *s) { start_node = s; }
	RpcServerAdaptor(Thread::TaskGraph *g) : graph(g) { }
};

template<typename Arg, typename Res>
class RpcServerTask : public Thread::TaskGraph
{
	friend class StatefulRpc;
	HardReference<RpcServerAdaptor> adaptor;
protected:
	RpcServerTask(Thread::TaskContext *ctx) : Thread::TaskGraph(ctx), adaptor(new RpcServerAdaptor(this)) { }
	void Start(Node *init_node) { adaptor->SetStartNode(init_node); }
	Arg* GetArgument() { return (Arg *)adaptor->rpc->argument; }
	Res* GetResult()   { return (Res *)adaptor->rpc->result;   }
	const Arg* GetArgument() const { return (const Arg *)adaptor->rpc->argument; }
	const Res* GetResult()   const { return (const Res *)adaptor->rpc->result;   }
};

class RpcClientAdaptor : public RpcStateAdaptor, public ObjectChangeSupport
{
	bool istimeout;
public:
	RpcClientAdaptor() : istimeout(false) { }
	void FireTimeout() { istimeout=true; FireObjectChange(); }
	void FireResult()  { FireObjectChange(); }
	bool IsTimeout() const { return istimeout; }
};

template<int callid, typename Arg, typename Res>
class RpcClientTask : public Thread::StatefulRunnable
{
	HardReference<RpcClientAdaptor> adaptor;
protected:
	int state;

	~RpcClientTask()
	{
	}

	RpcClientTask() : state(INIT), adaptor(new RpcClientAdaptor())
	{
		adaptor->rpc = StatefulRpc::Prepare(callid);
		adaptor->AddObjectChangeListener(this);
	}

	Rpc* Call()
	{
		return adaptor->rpc->Execute(adaptor);
	}

	void Run()
	{
		Transport* transport  = NULL;
		TransportContext* ctx = NULL;
		{
			HardReference<Thread::Mutex> l(GetContext(ctx));
			if ( ctx )
				transport = &ctx->GetTransport();
		}
		if ( ! transport || ! (*transport)(Call()) )
			adaptor->FireTimeout();
		if ( state == INIT )
			FireObjectChange();
	}

	bool IsTimeout() const { return adaptor->IsTimeout(); }
	Arg* GetArgument() { return (Arg *)adaptor->rpc->argument; }
	Res* GetResult()   { return (Res *)adaptor->rpc->result;   }
	const Arg* GetArgument() const { return (const Arg *)adaptor->rpc->argument; }
	const Res* GetResult()   const { return (const Res *)adaptor->rpc->result;   }
public:
	int GetState() const { return state; }
};

template<int callid, typename Arg>
class RpcClientTask<callid, Arg, bool> : public Thread::StatefulRunnable
{
	Arg *arg;
protected:
	int state;
	void Run()
	{
		Transport* transport  = NULL;
		TransportContext* ctx = NULL;
		{
			HardReference<Thread::Mutex> l(GetContext(ctx));
			if ( ctx )
				transport = &ctx->GetTransport();
		}
		if ( ! transport || ! (*transport)(arg) )
			state = FAIL;
		ObjectChange(this);
	}
	void ObjectChange(const ObjectChangeSupport *) { FireObjectChange(); }
	Arg* GetArgument() { return arg; }
	const Arg* GetArgument() const { return arg; }
	bool GetResult() const { return state == SUCCEED; }
public:
	~RpcClientTask() { arg->Destroy(); }
	int GetState() const { return state; }
	RpcClientTask() : arg((Arg*)Protocol::Create(callid)), state(SUCCEED)
	{
	}
};

class StatefulRpc : public Rpc
{
	WeakReference<RpcStateAdaptor> adaptor;
public:
	StatefulRpc(Type type, Data *arg, Data *res) : Rpc(type, arg, res) { }
	StatefulRpc(const StatefulRpc& rhs) : Rpc(rhs), adaptor(rhs.adaptor) { }
	static StatefulRpc* Prepare(Type type) { return (StatefulRpc *)Protocol::Create(type); }

	Rpc* Execute(const HardReference<RpcClientAdaptor>& rca)
	{
		xid.SetRequest();
		timer.Reset();
		
		Thread::RWLock::WRScoped l(locker_map);
		Map::iterator it = map.find(xid);
		if (it != map.end())
		{
			(*it).second->Destroy();
			(*it).second = this;
		}
		else
		{
			map.insert(std::make_pair(xid, this));
		}
		adaptor = rca;
		return this;
	}

	template<typename Arg, typename Res>
	void SetRpcServerTask(RpcServerTask<Arg, Res> *task) { adaptor = task->adaptor; }

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (xid.IsRequest())
		{
			xid.ClrRequest();
			Server(argument, result, manager, sid);
			HardReference<RpcStateAdaptor> ref(adaptor);
			if ( ! ref )
				manager->Send(sid, this);
			else
			{
				if ( ref )
				{
					RpcServerAdaptor *rsa = dynamic_cast<RpcServerAdaptor *>(ref.GetObject());
					rsa->rpc = (StatefulRpc *)Clone();
					rsa->manager = manager;
					rsa->sid = sid;
					rsa->graph->Thread::TaskGraph::Start( rsa->start_node );
				}
			}
			return;
		}
		StatefulRpc *rpc;
		{
		Thread::RWLock::WRScoped l(locker_map);
		Map::iterator it = map.find(xid);
		if (it == map.end()) return;
		rpc = (StatefulRpc *)(*it).second;
		map.erase(it);
		}
		rpc->Client(rpc->argument, rpc->result, manager, sid);
		HardReference<RpcStateAdaptor> ref(rpc->adaptor);
		if ( ref )
		{
			RpcClientAdaptor *rca = dynamic_cast<RpcClientAdaptor *>(ref.GetObject());
			if ( rca )
				rca->FireResult();
		}
		rpc->Destroy();
	}

	void Timeout()
	{
		OnTimeout();
		HardReference<RpcStateAdaptor> ref(adaptor);
		if ( ref )
		{
			RpcClientAdaptor *rca = dynamic_cast<RpcClientAdaptor *>(ref.GetObject());
			if ( rca )
				rca->FireTimeout();
		}
	}
};


};
#endif
