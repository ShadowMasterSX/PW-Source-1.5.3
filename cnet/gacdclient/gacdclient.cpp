
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "ganticheaterclient.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include "mystdin.hpp"
#include "commander.hpp"

using namespace GNET;

int main(int argc, char *argv[])
{
    const char filename[] = "/tmp/testfile";
    const int flength = 1024;
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    lseek(fd, flength + 1, SEEK_SET);
    write(fd, "\0", 1);
    lseek(fd, 0, SEEK_SET);

	printf("fd %d\n", fd);

    char *mapped_mem = (char*)mmap(0, flength, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);


	if (argc != 2 )
	{
		std::cerr << "Usage: " << argv[0] << " configurefile" << std::endl;
		exit(-1);
	}
    if( strcmp("java", argv[1]) == 0 )
    {
        printf("hi\n");
        if( *(int*)mapped_mem == 116 )
        *(int*)mapped_mem = 1023;
        munmap(mapped_mem, flength);
        return 0;
    }
    *(int*)mapped_mem = 116;
	Commander::GetInstance()->SharePointer((int**)&mapped_mem);

	Conf *conf = Conf::GetInstance(argv[1]);
	Log::setprogname("gacdclient");
	{
		GAntiCheaterClient *manager = GAntiCheaterClient::GetInstance(0);
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	{
		GAntiCheaterClient *manager = GAntiCheaterClient::GetInstance(1);
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}

	StdInIO::Start(new MyStdin());

	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run();
    munmap(mapped_mem, flength);
	return 0;
}

