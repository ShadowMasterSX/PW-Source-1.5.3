#ifndef __POLLIO_H
#define __POLLIO_H

#include <sys/types.h>
#if defined USE_KEVENT
#include <sys/event.h>
#elif defined USE_EPOLL
#include <sys/epoll.h>
#endif

#include <sys/time.h>
#include <sys/poll.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include "map.h"
#include <set>
#include <vector>
#include <algorithm>
#include <functional>

#include "thread.h"
#include "timer.h"
#include "itimer.h"
#include "statistic.h"
namespace GNET
{

class PollIO
{
	typedef gnet_map<int, PollIO*> IOMap;
	typedef gnet_map<PollIO*, int> EventMap;
	typedef std::set<PollIO*> EventSet;
	enum { POLLCLOSE = 0x80000000 };
	int  event;
	int  newevent;
	bool update;
#if defined USE_KEVENT
	typedef std::vector<struct kevent> FDSet;
	static int kq;
	enum { READ_ENABLED = 1, WRITE_ENABLED = 2, READ_ADDED = 4, WRITE_ADDED = 8 };
	int status;
#elif defined USE_EPOLL
	typedef std::vector<struct epoll_event> FDSet;
	static int ep;
#elif defined USE_SELECT
	typedef std::vector<int> FDSet;
	static fd_set rfds, wfds, all_rfds, all_wfds;
	static int maxfd;
#else
	struct PollFD : public pollfd
	{
		bool operator <  (const PollFD &rhs) const { return fd <  rhs.fd; }
		bool operator == (const PollFD &rhs) const { return fd == rhs.fd; }
	};
	typedef std::vector<PollFD> FDSet;
#endif

	static IOMap iomap;
	static FDSet fdset;
	static IOMap ionew;
	static EventSet eventset;
	static Thread::Mutex locker_poll;
	static Thread::Mutex locker_event;
	static int Init();
	static bool wakeup_flag;

	virtual void PollIn()  = 0;
	virtual void PollOut() { }
	virtual void PollClose() { }
#if defined USE_KEVENT
	static void LoadEvent(const EventMap::value_type event_pair)
	{
		PollIO *io   = event_pair.first;
		int newevent = io->event;
		int fd       = io->fd;

		if ( newevent & POLLCLOSE )
		{
			iomap[fd] = NULL;
			delete io;
			return;
		}

		if ( newevent & POLLOUT )
		{
			if (!(io->status & WRITE_ENABLED))
			{
				fdset.push_back(struct kevent());
				struct kevent &kv = fdset.back();
				EV_SET(&kv, fd, EVFILT_WRITE, (io->status & WRITE_ADDED) ? EV_ENABLE : EV_ADD, 0, 0, io);
				io->status |= (WRITE_ENABLED | WRITE_ADDED);
			}
		}
		else
		{
			if (io->status & WRITE_ENABLED)
			{
				fdset.push_back(struct kevent());
				struct kevent &kv = fdset.back();
				EV_SET(&kv, fd, EVFILT_WRITE, EV_DISABLE, 0, 0, io);
				io->status &= ~WRITE_ENABLED;
			}
		}

		if ( newevent & POLLIN )
		{
			if (!(io->status & READ_ENABLED))
			{
				fdset.push_back(struct kevent());
				struct kevent &kv = fdset.back();
				EV_SET(&kv, fd, EVFILT_READ, (io->status & READ_ADDED) ? EV_ENABLE : EV_ADD, 0, 0, io);
				io->status |= (READ_ENABLED | READ_ADDED);
			}
		}
		else
		{
			if (io->status & READ_ENABLED)
			{
				fdset.push_back(struct kevent());
				struct kevent &kv = fdset.back();
				EV_SET(&kv, fd, EVFILT_READ, EV_DISABLE, 0, 0, io);
				io->status &= ~READ_ENABLED;
			}
		}
	}

	static void UpdateEvent()
	{
		fdset.clear();
		UpdateEventMap();
	}

	static void TriggerEvent(const struct kevent &kv)
	{
		PollIO *io = (PollIO *)kv.udata;
		if (kv.filter == EVFILT_READ)  io->PollIn();
		if (kv.filter == EVFILT_WRITE) io->PollOut();
		io->PollClose();
	}
#elif defined USE_EPOLL
	static void LoadEvent(const EventMap::value_type event_pair)
	{
		PollIO *io   = event_pair.first;
		int oldevent = event_pair.second;
		int newevent = io->event;
		int fd       = io->fd;

		struct epoll_event ev;
		if ( newevent & POLLCLOSE )
		{
			if (oldevent)
				epoll_ctl(ep, EPOLL_CTL_DEL, fd, &ev);
			iomap[fd] = NULL;
			delete io;
		}
		else
		{
			int status = 0;
			if (newevent & POLLOUT) status |= EPOLLOUT;
			if (newevent & POLLIN)  status |= EPOLLIN;
			if (status)
			{
				ev.events = status;
				ev.data.ptr = io;
				epoll_ctl(ep, (oldevent & (POLLOUT | POLLIN)) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, io->fd, &ev);
			} else if (oldevent)
				epoll_ctl(ep, EPOLL_CTL_DEL, io->fd, &ev);
		}
	}

	static void UpdateEvent()
	{
		UpdateEventMap();
	}

	static void TriggerEvent(const struct epoll_event &ev)
	{
		PollIO *io = (PollIO *)ev.data.ptr;
		if (ev.events & (EPOLLIN | EPOLLERR | EPOLLHUP))
			io->PollIn();
		if (ev.events & EPOLLOUT)
			io->PollOut();
		io->PollClose();
	}
#elif defined USE_SELECT
	static void LoadEvent(const EventMap::value_type event_pair)
	{
		PollIO *io   = event_pair.first;
		int newevent = io->event;
		int fd       = io->fd;

		if ( newevent & POLLCLOSE )
		{
			FD_CLR(fd, &all_rfds);
			FD_CLR(fd, &all_wfds);
			iomap[fd] = NULL;
			delete io;
		}
		else
		{
			if ( newevent & POLLIN ) FD_SET(fd, &all_rfds); else FD_CLR(fd, &all_rfds);
			if ( newevent & POLLOUT) FD_SET(fd, &all_wfds); else FD_CLR(fd, &all_wfds);
			maxfd = std::max(maxfd, fd);
		}
	}

	static void UpdateEvent()
	{
		UpdateEventMap();
		rfds = all_rfds;
		wfds = all_wfds;
		fdset.clear();
		for (int i = 0; i <= maxfd; i++)
			if ( FD_ISSET(i, &rfds) || FD_ISSET(i, &wfds) )
				fdset.push_back(i);
	}

	static void TriggerEvent(int fd)
	{
		PollIO *io = iomap[fd];
		if (FD_ISSET(fd, &rfds)) io->PollIn();
		if (FD_ISSET(fd, &wfds)) io->PollOut();
		io->PollClose();
	}
#else
	static void LoadEvent(const EventMap::value_type event_pair)
	{
		PollIO *io   = event_pair.first;
		int newevent = io->event;
		int fd       = io->fd;

		PollFD pfd;
		pfd.fd = fd;

		if ( newevent & POLLCLOSE )
		{
			iomap[fd] = NULL;
			delete io;
			fdset.erase(std::remove(fdset.begin(), fdset.end(), pfd), fdset.end());
		}
		else
		{
			FDSet::iterator it = std::lower_bound(fdset.begin(), fdset.end(), pfd);
			if ( newevent )
			{
				if ( it == fdset.end() || (*it).fd != fd )
					it = fdset.insert(it, pfd);
				(*it).events = newevent;
			}
			else
			{
				if ( it != fdset.end() && (*it).fd == fd )
					fdset.erase(it);
			}
		}
	}

	static void UpdateEvent()
	{
		UpdateEventMap();
	}

	static void TriggerEvent(const pollfd &fds)
	{
		PollIO *io = iomap[fds.fd];
		if (fds.revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL))
			io->PollIn();
		if (fds.revents & POLLOUT)
			io->PollOut();
		io->PollClose();
	}
#endif

	static void UpdateEventMap()
	{
		EventMap map;
		{
			Thread::Mutex::Scoped l(locker_event);
			for(IOMap::const_iterator it = ionew.begin(), ie = ionew.end(); it != ie; ++it)
			{
				PollIO *io = (*it).second;
				iomap[(*it).first] = io;
#if 1
				if (io->newevent) eventset.insert(io);
#else
				;
#endif
			}
			ionew.clear();
			for (EventSet::iterator it = eventset.begin(), ie = eventset.end(); it != ie; ++it)
			{
				map.insert( std::make_pair(*it, (*it)->event) );
				(*it)->update   = false;
				(*it)->event    = (*it)->newevent;
				(*it)->newevent = 0;
			}
			eventset.clear();
			wakeup_flag = true;
		}
		std::for_each(map.begin(), map.end(), std::ptr_fun(&LoadEvent));
	}

	void WakeUpEventSet()
	{
		eventset.insert(this);
		update = true;
		WakeUp();
	}

	void _PermitRecv()
	{
		if ( update )
		{
			newevent |= POLLIN;
		} 
		else if ( (event & POLLIN) == 0 )
		{
			newevent = event | POLLIN;
			WakeUpEventSet();
		}
	}

	void _PermitSend()
	{
		if ( update )
		{
			newevent |= POLLOUT;
		} 
		else if ( (event & POLLOUT) == 0 )
		{
			newevent = event | POLLOUT;
			WakeUpEventSet();
		}
	}
protected:
	int fd;

	virtual ~PollIO()
	{
		while ( close(fd) == -1 && errno == EINTR );
	}
#if defined USE_KEVENT
	PollIO(int x) : event(0), newevent(0), update(false), status(0), fd(x)
#else
	PollIO(int x) : event(0), newevent(0), update(false), fd(x)
#endif
	{
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK);
	}

public:	
	static PollIO* Register(PollIO *io, bool init_permit_recv, bool init_permit_send)
	{
		Thread::Mutex::Scoped l(locker_event);
		ionew[io->fd] = io;
#if 1
		/*
		   修改：增加设置update=true
		刚建立连接的StreamIO, 如果在Register()和UpdateEventMap()调用之间上层应用调用Send发送数据，	
		_PermitSend会被调用，这里如果不设置update=true，newevent的POLLIN会被错误清除，导致该连接无法接收数据，
		netstat显示Recv-Q不断增加。      liuguichen 20121212
		 */
		if ( init_permit_recv ) { io->newevent |= POLLIN; io->update = true; }
		if ( init_permit_send ) { io->newevent |= POLLOUT; io->update = true; }
		WakeUp();
#else
		if ( init_permit_recv ) io->_PermitRecv();
		if ( init_permit_send ) io->_PermitSend();
#endif
		return io;
	}

	void PermitRecv()
	{
		Thread::Mutex::Scoped l(locker_event);
		_PermitRecv();
	}

	void PermitSend()
	{
		Thread::Mutex::Scoped l(locker_event);
		_PermitSend();
	}

	void ForbidRecv()
	{
		Thread::Mutex::Scoped l(locker_event);
		if ( update )
		{
			newevent &= ~POLLIN;
		} 
		else if ( event & POLLIN )
		{
			newevent = event & ~POLLIN;
			WakeUpEventSet();
		}
	}

	void ForbidSend()
	{
		Thread::Mutex::Scoped l(locker_event);
		if ( update )
		{
			newevent &= ~POLLOUT;
		} 
		else if ( event & POLLOUT )
		{
			newevent = event & ~POLLOUT;
			WakeUpEventSet();
		}
	}

	void Close()
	{
		Thread::Mutex::Scoped l(locker_event);
		newevent |= POLLCLOSE;
		if ( !update )
			WakeUpEventSet();
	}

	static int Poll(int timeout)
	{
		Thread::Mutex::Scoped l(locker_poll);
		static int __init_dummy = Init();
		UpdateEvent();
#if defined USE_KEVENT
		ResumeTimer();
		int nevents, nchanges = fdset.size();
		fdset.resize(iomap.size() * 2);
		if (timeout < 0)
			nevents = kevent(kq, &fdset[0], nchanges, &fdset[0], fdset.size(), 0);
		else
		{
			struct timespec ts;
			ts.tv_sec = timeout / 1000;
			ts.tv_nsec = (timeout - (ts.tv_sec * 1000)) * 1000000;
			nevents = kevent(kq, &fdset[0], nchanges, &fdset[0], fdset.size(), &ts);
		}
		wakeup_flag = false;
		SuspendTimer();
		if (nevents > 0)
			std::for_each(fdset.begin(), fdset.begin() + nevents, std::ptr_fun(&TriggerEvent));
#elif defined USE_EPOLL
		ResumeTimer();
		int maxevents = iomap.size();
		fdset.resize(maxevents);
		int nevents = epoll_wait(ep, &fdset[0], maxevents, timeout);
		wakeup_flag = false;
		SuspendTimer();
		if (nevents > 0)
			std::for_each(fdset.begin(), fdset.begin() + nevents, std::ptr_fun(&TriggerEvent));
#elif defined USE_SELECT
		int nevents;
		ResumeTimer();
		if (timeout < 0)
		{
			nevents = select(maxfd + 1, &rfds, &wfds, 0, 0);
		}
		else
		{
			struct timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - (tv.tv_sec * 1000)) * 1000;
			nevents = select(maxfd + 1, &rfds, &wfds, 0, &tv);
		}
		wakeup_flag = false;
		SuspendTimer();
		if (nevents > 0)
			std::for_each(fdset.begin(), fdset.end(), std::ptr_fun(&TriggerEvent));
		FD_ZERO(&rfds); FD_ZERO(&wfds);
#else
		ResumeTimer();
		int nevents = poll(&fdset[0], fdset.size(), timeout);
		wakeup_flag = false;
		SuspendTimer();
		if (nevents > 0)
			std::for_each(fdset.begin(), fdset.end(), std::ptr_fun(&TriggerEvent));
#endif
		return __init_dummy;
	}

	class Task : public Thread::Runnable
	{
		static Task *instance;
		Task(int prior=1) : Runnable(prior){ }
		void Run()
		{
			if (Thread::Pool::Size() == 1)
			{
				PollIO::Poll(1000);
				Timer::Update();
			}
			else
				PollIO::Poll(-1);
			STAT_MIN5("Poll",1);
			Thread::Pool::AddTask(this,true);
		}
	public:
		static Task *GetInstance() { return instance ? instance : instance = new Task(Thread::Pool::Size() == 1 ? INT_MAX : 0); }
	};

	static void WakeUp();
};
#ifdef _REENTRANT_
class PollControl : public PollIO
{
	friend class PollIO;
	static int writer;

	void PollIn()  { for(char buff[256]; read(fd, buff, 256) == 256; ); }
	PollControl(int r, int w) : PollIO(r) { writer = w; fcntl(w, F_SETFL, fcntl(w, F_GETFL)|O_NONBLOCK); }
	
	~PollControl()
	{
		while ( close(writer) == -1 && errno == EINTR );
	}

	static void WakeUp() { write(writer, "", 1); }

	static void Init()
	{
		int pds[2];		
		pipe(pds);
		PollIO::Register(new PollControl(pds[0], pds[1]), true, false);
	}
};
#else
class PollControl
{	
public:
	static void WakeUp() {  }
	static void Init()   {  }
};
#endif
inline int PollIO::Init()
{
	signal(SIGPIPE, SIG_IGN);
#if defined USE_KEVENT
	kq = kqueue();
#elif defined USE_EPOLL
	ep = epoll_create(8192);
#elif defined USE_SELECT
	maxfd = 0; FD_ZERO(&rfds); FD_ZERO(&wfds);
#endif
	PollControl::Init();
	return 0;
}

inline void PollIO::WakeUp()
{	
	if ( wakeup_flag )
	{
		wakeup_flag = false;
		PollControl::WakeUp();
	}
}

};

#endif
