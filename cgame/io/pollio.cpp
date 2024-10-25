#include "pollio.h"

namespace ONET
{

PollIO::IOMap PollIO::iomap;
PollIO::FDSet PollIO::fdset;

#if defined _REENTRANT
PollIO::IOMap PollIO::ionew;
pthread_mutex_t PollIO::locker_ionew = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PollIO::locker_poll  = PTHREAD_MUTEX_INITIALIZER;
#endif

#if defined USE_KEVENT
int PollIO::kq;
#elif defined USE_EPOLL
int PollIO::ep;
int PollIO::maxevents;
#endif

void PollIO::Init()
{
#if defined USE_KEVENT
	kq = kqueue();
#elif defined USE_EPOLL
	ep = epoll_create(8192);
	maxevents = 0;
#endif
#if defined _REENTRANT
	PollControl::Init();
#endif
}

void PollIO::WakeUp() { PollControl::WakeUp(); }
int PollControl::writer;

};

