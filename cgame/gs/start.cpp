#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <ASSERT.h>
#include <threadpool.h>
#include "start.h"
#include <gsp_if.h>
#include <glog.h>
#include <amemory.h>
#include "common/message.h"
#include "worldmanager.h"
#include "arandomgen.h"
#include "world.h"
#include <malloc.h>
#include <hashmap.h>
#include "threadusage.h"
#include "global_controller.h"

bool g_mobile_server = false;

const size_t  CSIZE = 65536 ;
static int size_counter[CSIZE] = {0};
static int large_counter = 0;
static int total_mem_size = 0;

static void AllocSize(size_t n)
{
	interlocked_add(&total_mem_size,n);
	if(n <CSIZE)
	{
		interlocked_add(&size_counter[n],1);
	}
	else
	{
		interlocked_add(&large_counter,1);
	}
}

static void DeleteSize(size_t n)
{
	interlocked_add(&total_mem_size,-(int)n);
	if(n <CSIZE)
	{
		interlocked_add(&size_counter[n],-1);
	}
	else
	{
		interlocked_add(&large_counter,-1);
	}
}

//static bool no_record = true;
void * operator new[](size_t n)
{
	void * rst = abase::fast_allocator::raw_alloc(n + 4);
	*(size_t*) rst = n;
	AllocSize(n);
	return ((char*)rst) + 4;
}

void * operator new(size_t n)
{
	void * rst = abase::fast_allocator::raw_alloc(n + 4);
	*(size_t*) rst = n;
	AllocSize(n);
	return ((char*)rst) + 4;
}

void operator delete(void * p, size_t n)
{
	if(!p) return;
	void * xp = ((char*)p) -4;
	size_t size = *(size_t*)xp;
	DeleteSize(size);
	abase::fast_allocator::raw_free(xp);
//	free(p);
//	if(!no_record) fprintf(stderr,"free_ptr   %p  caller %p\n",p,*(&n-1));
}

void operator delete(void * p)
{
	if(!p) return;
	void * xp = ((char*)p) -4;
	size_t size = *(size_t*)xp;
	DeleteSize(size);
	abase::fast_allocator::raw_free(xp);
//	free(p);
//	if(!no_record) fprintf(stderr,"free_ptr   %p  caller %p\n",p,*(&p-1));
}

void operator delete[](void * p)
{
	if(!p) return;
	void * xp = ((char*)p) -4;
	size_t size = *(size_t*)xp;
	DeleteSize(size);
	abase::fast_allocator::raw_free(xp);
//	free(p);
//	if(!no_record) fprintf(stderr,"free_ptr   %p  caller %p\n",p,*(&p-1));
}

int GetCPUCount()
{
	return 4;
}

void PRINT_MEM_USAGE(const char * name)
{
	static rusage g_usage;
	if(name)
	{
		rusage tmp = g_usage;
		getrusage(RUSAGE_SELF, &g_usage);
		__PRINTF("%s消耗内存 %ld KB, 进程总占用 %ld KB\n", name, g_usage.ru_maxrss - tmp.ru_maxrss, g_usage.ru_maxrss);
	}
	else
	{
		getrusage(RUSAGE_SELF, &g_usage);
		__PRINTF("静态对象占用内存 %ld KB\n", g_usage.ru_maxrss);
	}
}

int InitWorld(const char*, const char *);
int FirstStepInit(const char *,const char * );
//#include <mcheck.h>
namespace ONET{ 
extern int __thread_lock_count;
extern int __thread_task_count;
}
namespace GMSV{
extern unsigned long long s2c_cmd_number_counter1[1024];
extern unsigned long long s2c_cmd_number_counter2[1024];
}
int main(int argn , char ** argv)
{
	printf("Compiled By "__USER__ ", "__DATE__ " "__TIME__ "\n");
	
	if(system("/bin/touch foo"))
	{
		printf("文件系统不可写，无法进行后继的初始化操作\n");
		return -1;
	}

	time_t now = time(NULL);
	struct tm tm1; 
	localtime_r(&now, &tm1);
	if(tm1.tm_gmtoff == 28800) //北京时区才修改TZ 环境变量 
	{
		putenv("TZ=Asia/Shanghai");
	}
	char * pTZ =  getenv("TZ");
	__PRINTINFO("env TZ = %s\n", (pTZ ? pTZ : "")); 

	const char * conf_file = "gs.conf";
	const char * gmconf_file = "gmserver.conf";
	const char * servername = "gs01";
	const char * alias_conf = "gsalias.conf";
	if(argn >1) servername = argv[1];
	if(argn >2) conf_file = argv[2];
	if(argn >3) gmconf_file = argv[3];
	if(argn >4) alias_conf = argv[4];

	//手机用户服务器固定前缀为"ms"
	const char * mobile_prefix = "ms";
	if(strncmp(servername, mobile_prefix, strlen(mobile_prefix)) == 0)
	{
		g_mobile_server = true;
		__PRINTINFO("手机用户服务器(%s)启动...\n", servername);
	}

	if(int rst = FirstStepInit(conf_file,alias_conf))
	{
		__PRINTINFO("第一步初始化失败，错误号:%d\n",rst);
		return rst;
	}

	if(argn >5)
	{
		do
		{
			if(fork() == 0)
			{
				for(int i = 5; i < argn; i ++)
				{
					memset(argv[i],0,strlen(argv[i]));
					argv[i] = 0;
				}
				if(fork() == 0) break; else return 0;
			}

			int i;
			for(i = 5; i < argn; i ++)
			{
				if(fork() == 0)
				{
					if(fork() == 0)
					{
						servername = strdup(argv[i]);
						memset(argv[1],0,strlen(argv[1]));
						strcpy(argv[1],argv[i]);
						for(int j =5; j < argn; j ++)
						{
							memset(argv[j],0,strlen(argv[j]));
							argv[j] = 0;
						}
						break;
					}
					else
					{
						return 0;
					}
				}
			}
			if(i == argn) return 0; //主进程退出
		}
		while(0);
	}

	if(g_mobile_server && strncmp(servername, mobile_prefix, strlen(mobile_prefix)) != 0
			|| !g_mobile_server && strncmp(servername, mobile_prefix, strlen(mobile_prefix)) == 0)
	{
		__PRINTINFO("通过fork同时启动多个gs进程时，后面的servername对应的服务器类型与第一个不匹配\n");
		return -1;
	}
	
	__PRINTINFO("初始化 %s\n",servername);

//	setenv("MALLOC_TRACE", "/tmp/m.log", 1);
//	mtrace();
	if(int rst = InitWorld(gmconf_file,servername))
	{
		__PRINTINFO("初始化失败， 错误号：%d\n",rst);
		_exit(rst);
		return rst;
	}
	fflush(stdout);
	long header_offset = ftell(stdout);
	if(header_offset <=0 || header_offset > 0x500000) 
	{
		header_offset = 2048;
	}

	//启动thread
	Conf *conf = ONET::Conf::GetInstance();
	int logic_threads = atoi(conf->find("General","logic_threads").c_str());
	if(logic_threads < 4 || logic_threads > 16) logic_threads = 4;
	ONET::Thread::Pool::CreatePool(logic_threads);
	ONET::Thread::Pool::CreateThread(GMSV::StartPollIO);
	int log_counter = 0;

//	InitHookTab();
	if(__PRINT_INFO_FLAG) ThreadUsage::Start(60, 0);
	
	setlinebuf(stdout);
	while(1) 
	{
		sleep(40);
		log_counter += 40;
		if(log_counter > 240)
		{
			log_counter -= 240;
			GLog::log(GLOG_INFO,"gameserver %d , 用户数目:%d", world_manager::GetWorldIndex(),world_manager::GetInstance()->GetOnlineUserNumber());
		}
		time_t ct = time(NULL);
		__PRINTINFO("current time:%s",ctime(&ct));
		if(__PRINT_INFO_FLAG) ThreadUsage::Dump(stdout);
		__PRINTINFO("tick:%d, thread_lock_count(60sec):%d exec_task_count:%d cur_task_count:%d\n",
				g_timer.get_tick(),ONET::__thread_lock_count, 
				ONET::__thread_task_count, ONET::Thread::Pool::QueueSize());
		__PRINTINFO("free timer count:%8d total alloced:%d\n",g_timer.get_free_timer_count(),g_timer.get_timer_total_alloced());
		__PRINTINFO("total threadpool task count:%d\n",g_timer.get_tick(),ONET::Thread::Pool::GetTotalTaskCount());
		if(__PRINT_INFO_FLAG) abase::fast_allocator::dump(stdout);
		__PRINTINFO("Large size memory block:\n");
		if(__PRINT_INFO_FLAG) abase::fast_allocator::dump_large(stdout);
		__PRINTINFO("_________________________________________________________________________\n");
		world * pPlane = world_manager::GetInstance()->GetWorldByIndex(0);
		if(pPlane)
		{
			pPlane->DumpMessageCount();
		}
		
		__PRINTINFO("cur_player_count %d\n",world_manager::GetInstance()->GetOnlineUserNumber());
		__PRINTINFO("_________________________________________________________\n");
		__PRINTINFO("total_size %d\n",total_mem_size);
		__PRINTINFO("large counter:%d\n",large_counter);
		/*size_t index = 0;
		for(size_t i = 0; i < CSIZE; i ++)
		{
			if(size_counter[i])
			{
				index ++;
				__PRINTINFO("%5d:%8d  count %5d\n",index, i, size_counter[i]);
			}
		}*/
		//打印命令耗费时间记录
		//if(log_counter < 60 )
		{
			extern unsigned long long cmd_rdtsc_counter[1024];
			extern unsigned long long cmd_number_counter[1024];
			for(size_t i = 0; i < 1024; i ++)
			{
				unsigned long long n1 = cmd_number_counter[i];
				unsigned long long n2 = cmd_rdtsc_counter[i];
				if(n2 == 0) continue;
				__PRINTINFO("cmd:%3d  %8d times average use: %6d\n",i, (int)n1, (int)(n2/n1));
			}
			__PRINTINFO("_________________________________________________________\n");
		}
		{
			for(size_t i = 0; i < 1024; i ++)
			{
				unsigned long long n1 = GMSV::s2c_cmd_number_counter1[i];
				unsigned long long n2 = GMSV::s2c_cmd_number_counter2[i];
				if(n1 == 0) continue;
				__PRINTINFO("s2c cmd:%3d  counter1 %lld counter2 %lld\n",i, n1, n2);
			}
			__PRINTINFO("_________________________________________________________\n");
		}

		
		long pos = ftell(stdout);
		if(pos >= 0)
		{
			__PRINTINFO("cur_file_size %u\n",pos);
			fflush(stdout);
			if(pos > 0x5FFFFFFF)
			{
				//考虑进行对stdout进行剪切
				//留出头部的日期数据等
				ftruncate(fileno(stdout),header_offset);
				fseek(stdout,header_offset,SEEK_SET);
			}
		}
		else
		{
			fflush(stdout);
		}

		world_manager::GetGlobalController().CheckUpdate();
	}
	return 0;
}



