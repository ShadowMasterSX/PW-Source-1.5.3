#include "gamedbclient.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>

#include "getrole.hrp"
#include "delrole.hrp"

using namespace GNET;

#include "db_if.h"

using namespace GDB;

class test : public Result
{
public:
	virtual void OnTimeOut(){}
	virtual void OnFailed()
	{
		printf("failed\n");
	}
	virtual void OnGetRole(int id,const base_info * pInfo, const vecdata * pData){}
};

class TestTask : public Thread::Runnable
{
public:
	TestTask( ) : Thread::Runnable(1)
	{
	}

	virtual void Run( )
	{
		while( true )
		{
			sleep(1);
		};
		/*
		sleep( 1 );
		{
			RolePair2 arg;
			arg.key.id = 1;
			strcpy( arg.value.name, "sunzhenyu" );
			arg.value.exp = 10000;
			Rpc *rpc = Rpc::Call(RPC_PUTROLE2, &arg);
			bool success = GamedbClient::GetInstance()->SendProtocol(*rpc);
			PollIO::WakeUp();
			printf("Manager Send = %d\n", success );
		}

		sleep( 1 );
		{
			RoleId arg( 1 );
			Rpc *rpc = Rpc::Call(RPC_GETROLE, &arg);
			((GetRole *)rpc)->_callback = &ppc;
			bool success = GamedbClient::GetInstance()->SendProtocol(*rpc);
			PollIO::WakeUp();
			printf("Manager Send = %d\n", success );
		}

		sleep( 1 );
		*/
		//{
		//	RoleId arg( 1 );
		//	Rpc *rpc = Rpc::Call(RPC_DELROLE, &arg);
		//	bool success = GamedbClient::GetInstance()->SendProtocol(*rpc);
		//	PollIO::WakeUp();
		//	printf("Manager Send = %d\n", success );
		//}


		delete this;
	}
};



int main(int argc, char *argv[])
{
	if (argc != 2 && access(argv[1], R_OK) == -1)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile" << std::endl;
		exit(-1);
	}

	Conf *conf = Conf::GetInstance(argv[1]);
	Log::setprogname("dbclient");
	{
		GamedbClient *manager = GamedbClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}

	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::AddTask( new TestTask() );
	Thread::Pool::Run();
	return 0;
}

