#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <unistd.h>
#include <syslog.h>
#include "ASSERT.h"

#include "cmnet.h"

int bind_and_listen(unsigned short port,int qsize)
{
	int s = socket(AF_INET,SOCK_STREAM,0);
	if(s == -1) return -1;
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
//	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);
	if(bind(s,(const struct sockaddr*)&addr,sizeof(addr)) == -1)
	{
		perror("bind error");
		close(s);
		return -1;
	}
	listen(s,qsize);
	return s;
}


int bind_and_listen_un(const char * file,int qsize)
{	
	int s = socket(AF_LOCAL,SOCK_STREAM,0);
	if(s == -1) return -1;
	struct sockaddr_un addr;
	memset(&addr,0,sizeof(addr));
	addr.sun_family = AF_LOCAL;
	strncpy(addr.sun_path,file,sizeof(addr.sun_path));
	if(bind(s,(const struct sockaddr*)&addr,sizeof(addr)) == -1)
	{
		perror("bind error");
		close(s);
		return -1;
	}
	listen(s,qsize);
	return s;
}

int
write_fd(int fd, void *ptr, size_t nbytes, int sendfd)
{
#ifdef CMSG_SPACE
	struct msghdr msg;
	struct iovec iov[1];

	union {
		struct cmsghdr cm;
		char    control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr *cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*((int *) CMSG_DATA(cmptr)) = sendfd;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	return(sendmsg(fd, &msg, 0));
#else
	ASSERT(0);
	return -1;
#endif
}


int
read_fd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
#ifdef CMSG_SPACE
	struct msghdr msg;
	struct iovec iov[1];
	int n;

	union {
		struct cmsghdr cm;
		char    control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr *cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;


	if ( (n = recvmsg(fd, &msg, 0)) <= 0)
		return(n);

	if ( (cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
			cmptr->cmsg_len == CMSG_LEN(sizeof(int))) {
		if (cmptr->cmsg_level != SOL_SOCKET){
			syslog(LOG_ERR, "control level != SOL_SOCKET");
			exit(0);
		}
		if (cmptr->cmsg_type != SCM_RIGHTS){
			syslog(LOG_ERR, "control type != SCM_RIGHTS");
			exit(0);
		}
		*recvfd = *((int *) CMSG_DATA(cmptr));
	} else
		*recvfd = -1;  /* descriptor was not passed */

	return(n);
#else
	ASSERT(0);
	return -1;
#endif
}

