#include "transman.h"
#include <stdio.h>
#include "thread.h"
#include "pollio.h"
#include "conf.h"
#include "pthread.h"
using namespace GNET;

void* update_trans(void* )
{
	sleep(15);
	{
		StorageEnv::Storage * ptrans = StorageEnv::GetStorage("translog");
		StorageEnv::CommonTransaction txn;
		printf("====== set(1,1) to _ST_COMMIT ======\n");
		int status=SellRecord::value_type::_ST_COMMIT;
		TransMan::GetInstance().UpdateTrans( TransMan::TransKey(1,1),status,ptrans,txn );
		printf("====== set(1,1) to _ST_COMMIT_END ======\n");
		status=SellRecord::value_type::_ST_COMMIT_END;
		TransMan::GetInstance().UpdateTrans( TransMan::TransKey(1,1),status,ptrans,txn );
	}
	StorageEnv::checkpoint();
	return NULL;
}
int main()
{
	int zoneid=1,aid=2;
	Conf::GetInstance("gamedbd.conf");
	IntervalTimer::StartTimer( 2000000 );
	StorageEnv::Open();
	TransMan& trans=TransMan::GetInstance("./translog"); //this line must follow "StorageEnv::Open()"
	trans.Init(zoneid,aid);
	//add a new trans,send to au and add to expire list
	{
		StorageEnv::Storage * ptrans = StorageEnv::GetStorage("translog");
		StorageEnv::CommonTransaction txn;
		trans.AddTrans(
				32, //buyer
				48, //seller
				100000, //price
				300, //point
				1,
				ptrans,
				txn); //sellid
	}
	Thread::Pool::AddTask( PollIO::Task::GetInstance() );
	/* create a new thread
	 */  
	pthread_t th1;
	pthread_create(&th1,NULL,update_trans,NULL);
	Thread::Pool::Run();
}
