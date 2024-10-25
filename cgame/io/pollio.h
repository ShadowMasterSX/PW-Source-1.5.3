#ifndef __POLLIO_H
#define __POLLIO_H

#include <sys/types.h>

#if defined _REENTRANT
#include <pthread.h>
#endif

#include <sys/time.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdarg.h>

#include <map>
#include <vector>
#include <algorithm>
#include <functional>

namespace ONET
{

class PollIO
{
	typedef std::map<int, PollIO*> IOMap;
	typedef std::vector<pollfd> FDSet;

	static IOMap iomap;
	static FDSet fdset;

#if defined _REENTRANT
	static IOMap ionew;			//When poll start append it to iomap
	static pthread_mutex_t locker_ionew;
	static pthread_mutex_t locker_poll;	//Only one thread can poll
#endif

	virtual int UpdateEvent() { return 0;}
	virtual void PollIn()  { }
	virtual void PollOut() { }

	static void UpdateEvent(const IOMap::value_type iopair)
	{
		PollIO *io = iopair.second;

		if (io == NULL)
			return;

		int event = io->UpdateEvent();

		if (event == -1)
		{
			iomap[io->fd] = NULL;
			delete io;
			return;
		}

		if (event & (POLLIN | POLLOUT))
		{
			fdset.push_back(pollfd());
			pollfd &pfd = fdset.back();
			pfd.fd = io->fd;
			pfd.events  = event;
			pfd.revents = 0;
		}
	}

	static void TriggerEvent(const pollfd &fds)
	{
		PollIO *io = iomap[fds.fd];
		if(!io) return;
		if (fds.revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL))
			io->PollIn();
		if (fds.revents & POLLOUT)
			io->PollOut();
	}

protected:
	int fd;

	virtual ~PollIO()
	{
	}

	PollIO(int x) : fd(x)
	{
	}

public:	
	void Register()
	{
#if defined _REENTRANT
		pthread_mutex_lock(&locker_ionew);
		ionew[fd] = this;
		pthread_mutex_unlock(&locker_ionew);
#else
		iomap[fd] = this;
#endif
	}

	static void Poll(int timeout)
	{
#if defined _REENTRANT
		pthread_mutex_lock(&locker_poll);
		pthread_mutex_lock(&locker_ionew);
		for(IOMap::const_iterator i = ionew.begin(); i != ionew.end(); ++i)
			iomap[(*i).first] = (*i).second;
		ionew.clear();
		pthread_mutex_unlock(&locker_ionew);
#endif
		fdset.clear();
		std::for_each(iomap.begin(), iomap.end(), std::ptr_fun(&UpdateEvent));

		if (poll(&fdset[0], fdset.size(), timeout) > 0)	
			std::for_each(fdset.begin(), fdset.end(), std::ptr_fun(&TriggerEvent));
#if defined _REENTRANT
		pthread_mutex_unlock(&locker_poll);
#endif
	}

	static void Init();
	static void WakeUp();
};

class PollControl : public PollIO
{
	friend class PollIO;
	static int writer;

	int UpdateEvent() { PollIn(); return POLLIN; }
	void PollIn()  
	{ 
		for(char buff[256]; read(fd, buff, 256) > 0; );
	}
	PollControl(int r, int w) : PollIO(r) { writer = w; }
	
	~PollControl()
	{
		close(fd);
		close(writer);
	}

	static void WakeUp() { write(writer, "", 1); }

	static void Init()
	{
		int pds[2];		
		pipe(pds);
		fcntl(pds[0], F_SETFL, fcntl(pds[0], F_GETFL)|O_NONBLOCK);
		fcntl(pds[1], F_SETFL, fcntl(pds[1], F_GETFL)|O_NONBLOCK);
		new PollControl(pds[0], pds[1]);
	}
};

};

#endif
