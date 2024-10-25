#include "pollio.h"

namespace GNET
{

PollIO::IOMap PollIO::iomap;
PollIO::FDSet PollIO::fdset;
PollIO::IOMap PollIO::ionew;
PollIO::EventSet PollIO::eventset;
Thread::Mutex PollIO::locker_poll("PollIO::locker_poll");
Thread::Mutex PollIO::locker_event("PollIO::locker_event");
bool PollIO::wakeup_flag  = false;

#if defined USE_KEVENT
int PollIO::kq;
#elif defined USE_EPOLL
int PollIO::ep;
#elif defined USE_SELECT
int PollIO::maxfd;
fd_set PollIO::rfds;
fd_set PollIO::wfds;
fd_set PollIO::all_rfds;
fd_set PollIO::all_wfds;
#endif

#ifdef _REENTRANT_
int PollControl::writer;
#endif
PollIO::Task *PollIO::Task::instance = NULL;

};

