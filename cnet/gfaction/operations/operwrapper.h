/* 
 * This file defines a base class for wrapper operations.
 */ 

#ifndef __GNET_OPERWRAPPER_H
#define __GNET_OPERWRAPPER_H
#include "factionopaddinfo"
#include "fmemberinfo"
#include "gmailsyncdata"
#include "factionopsyncinfo"	//sync data with gameserver
#include "factiondata.hxx"

#include "operation.h"
#include "itimer.h"
#include "reference.h"
namespace GNET
{
//context is a marshalable object, all object is marshaled to a buffer
class OperWrapper;	
class OpState
{
	protected:
		int timeout;
	public:
		OpState(int _timeout) : timeout(_timeout) { }
		virtual ~OpState() { }
		virtual void Run(OperWrapper* op) { }
		virtual bool TimePolicy(int time_elapse) { return time_elapse<timeout; }
};
class OpInitState : public OpState
{
	public:
		OpInitState(int _timeout) : OpState(_timeout) { }
		void Run(OperWrapper* op);
};
class OpSyncState : public OpState
{
	public:
		OpSyncState(int _timeout) : OpState(_timeout) { }
		void Run(OperWrapper* op);
};
class OpAddInfoState : public OpState
{
	public:
		OpAddInfoState(int _timeout) : OpState(_timeout) { }
		void Run(OperWrapper* op);
};

class OpExecuteState : public OpState
{
	public:
		OpExecuteState(int _timeout) : OpState(_timeout) { }
		void Run(OperWrapper* op);
};

class OpEndState : public OpState
{
	public:
		OpEndState(int _timeout) : OpState(_timeout) { }
		void Run(OperWrapper* op);
};

class OperWrapper : public IntervalTimer::Observer 
{
	friend class operation;

	friend class OpState;
	friend class OpInitState;
	friend class OpSyncState;
	friend class OpAddInfoState;
	friend class OpExecuteState;
	friend class OpEndState;
public:
	typedef HardReference<OperWrapper> href_t;
	typedef WeakReference<OperWrapper> wref_t;
private:	
	typedef	std::map<unsigned int,href_t> WrapperMap;
	static Thread::Mutex locker_map;
	static WrapperMap& GetMap() {
		static WrapperMap wrappermap;
		return wrappermap;
	}
	
	//state of the OperWrapper
	Thread::Mutex locker_self;
	OpState* state;
	void ChangeState(OpState* _newstate) { state=_newstate; m_timer.Reset(); }
	IntervalTimer   m_timer;
	
private:
	Operation* 		p;		//core operation of this OperWrapper
	int				roleid;	//roleid of the operator
	unsigned int	tid;	//transaction id, that can be used to identify a wrapper
	int				retcode;//retcode of this OperWrapper
	
	//context	informations
	FactionOPSyncInfo			syncdata; //gameserver syncdata
	FactionOPAddInfo            addinfo;  //other server(uniquename server)'s information
	Marshal::OctetsStream		op_params;//operation parameters from client
	
	OperWrapper(int _roleid,Operation::Type _type);
	
public:	
	~OperWrapper() {
		Thread::Mutex::Scoped l(locker_self);
		if (p!=NULL)
		{
			delete p;
		}
	}
	//wrapper management
	static unsigned int CreateWrapper(int _roleid,Operation::Type _type)	
	{
		return (new OperWrapper(_roleid,_type))->tid;
	}
	static wref_t FindWrapper(unsigned int tid) {
		Thread::Mutex::Scoped l(locker_map);
		WrapperMap::iterator it=GetMap().find(tid);
		return it==GetMap().end() ? wref_t():wref_t( (*it).second );
	}

	// context management
	int GetOperator() { return roleid; }
	int GetOpType() { return p!=NULL ? (int)p->GetType() : -1; }
	FactionOPSyncInfo& GetSyncData() { return syncdata; }
	void SetSyncData(const FactionOPSyncInfo& _sync) { syncdata=_sync; }
	Marshal::OctetsStream& GetParams() { return op_params; }
	void SetParams(const Marshal::OctetsStream& os) { op_params=os; }
	FactionOPAddInfo& GetAddInfo() { return addinfo; }
	void SetAddInfo(const FactionOPAddInfo& add) { addinfo=add; }

	//execute
	void Execute();
	//set result
	void SetResult(void* pResult)
	{
		if (p) p->SetResult(pResult);
	}
	//timer related functions
	bool Update();
};

}; //end of namespace
#endif
