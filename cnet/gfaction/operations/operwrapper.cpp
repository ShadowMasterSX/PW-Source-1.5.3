#include "operwrapper.h"
#include "gfs_io.h"
namespace GNET
{
	Thread::Mutex OperWrapper::locker_map("OperWrapper::locker_map");
	GNET::OpInitState	 __state_init(5);
	GNET::OpSyncState	 __state_sync(5);
	GNET::OpAddInfoState __state_addinfo(10);
	GNET::OpExecuteState __state_execute(5);
	GNET::OpEndState	 __state_end(0);

	void OpInitState::Run(OperWrapper* op)
	{
		bool blErrOccur=false;
		{
			Thread::Mutex::Scoped l(op->locker_self);
			//add timepolicy here
			Operation* p=op->p;
			if (p->NeedSync())
			{
				//DEBUG_PRINT("INIT:发送同步协议到游戏服务器\n");
				//TODO:send FactionBeginSync to gameserver
				if (gps_send_beginsync(op->tid,op->roleid))
				{
					op->ChangeState(&__state_sync);
				}
				else
				{
					gfs_send_factionrequest_re(op->GetOpType(),op->roleid,ERR_FC_NETWORKERR);
					op->ChangeState(&__state_end);
				}
			}
			else if (p->NeedAddInfo())
			{
				//DEBUG_PRINT("INIT:索要附加信息\n");
				if ( (op->retcode=p->QueryAddInfo())==ERR_SUCCESS )
					op->ChangeState(&__state_addinfo);
				else
					blErrOccur=true;
			}
			else
			{
				//DEBUG_PRINT("INIT:执行操作\n");
				if ( ( op->retcode=p->Execute() )==ERR_SUCCESS )
					op->ChangeState(&__state_execute);
				else
					blErrOccur=true;
			}
		}
		if (blErrOccur)
			__state_execute.Run(op);
	}
	void OpSyncState::Run(OperWrapper* op)
	{
		bool blErrOccur=false;
		{
			Thread::Mutex::Scoped l(op->locker_self);
			Operation* p=op->p;
			//DEBUG_PRINT("SYNC:收到游戏服务器的同步相应，开始执行操作\n");
			if (p->NeedAddInfo())
			{
				//DEBUG_PRINT("SYNC:索要附加信息\n");
				if ( (op->retcode=p->QueryAddInfo())==ERR_SUCCESS )
					op->ChangeState(&__state_addinfo);
				else
					blErrOccur=true;
			}
			else
			{
				//DEBUG_PRINT("SYNC:执行操作\n");
				if ( (op->retcode=p->Execute())==ERR_SUCCESS )
					op->ChangeState(&__state_execute);
				else
					blErrOccur=true;
			}
		}
		if (blErrOccur)
			 __state_execute.Run(op);
	}
	void OpAddInfoState::Run(OperWrapper* op)
	{
		bool blErrOccur=false;
		{
			Thread::Mutex::Scoped l(op->locker_self);
			Operation* p=op->p;
			//DEBUG_PRINT("ADDINFO:执行操作\n");
			if ( (op->retcode=p->Execute())==ERR_SUCCESS )
				op->ChangeState(&__state_execute);
			else
				blErrOccur=true;
		}
		if (blErrOccur)
			__state_execute.Run(op);
	}
	void OpExecuteState::Run(OperWrapper* op)
	{
		Thread::Mutex::Scoped l(op->locker_self);
		Operation* p=op->p;
		if (p->NeedSync())
		{
			//TODO:send FactionEndSync to gameserver
			//DEBUG_PRINT("EXECUTE:发送结束同步的协议到游戏服务器\n");
			if (op->retcode!=ERR_SUCCESS)
				op->syncdata.player_sp=0;
			gps_send_endsync(op->tid,op->roleid,op->syncdata);
		}
		//DEBUG_PRINT("EXECUTE:操作结束\n");
		op->ChangeState(&__state_end);
	}
	void OpEndState::Run(OperWrapper* op)
	{
		//do nothing
	}

	OperWrapper::OperWrapper(int _roleid,Operation::Type _type) : 
		locker_self("OperWrapper::locker_self"),
		p(Operation::Create(_type)),
		roleid(_roleid)
	{
		static unsigned int idcount=0;
		IntervalTimer::Attach(this,1000000/IntervalTimer::Resolution());
		if (p==NULL)
		{
			tid=0;
			retcode=ERR_FC_INVALID_OPERATION;
			//DEBUG_PRINT("OperWrapper:: invalid operation type %d\n",_type);
			gfs_send_factionrequest_re(_type,_roleid,ERR_FC_INVALID_OPERATION);
			ChangeState(&__state_end);
		}
		else
		{
			Thread::Mutex::Scoped l(locker_map);
			tid=++idcount;
			GetMap()[tid]=href_t(this);
			p->AttachWrapper(this,tid);
			ChangeState(&__state_init);
		}
	}
	void OperWrapper::Execute()
	{
		//DEBUG_PRINT("Execute OperWrapper....,state=%p\n",state);
		state->Run(this);
	}
	//timer related functions
	bool OperWrapper::Update()
	{
		size_t elapse=m_timer.Elapse();
		//if (!state->TimePolicy(m_timer.Elapse()))
		if (!state->TimePolicy(elapse))
		{
			//DEBUG_PRINT("timeout, delete OperWrapper %p(tid=%d)\n",this,this->tid);
			if (state!=&__state_end)
				gfs_send_factionrequest_re(GetOpType(),roleid,ERR_FC_OP_TIMEOUT);
			{
				Thread::Mutex::Scoped l(locker_map);
				GetMap().erase(tid);
			}
			return false;
		}
		else
		{
			//DEBUG_PRINT("time elapse is %d\n",elapse);	
			return true;
		}
	}
};
