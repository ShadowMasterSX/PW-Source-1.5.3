#ifndef __ACTIVEIO_H
#define __ACTIVEIO_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "conf.h"
#include "pollio.h"
#include "netio.h"

namespace ONET
{

class ActiveIO : PollIO
{
	ActiveIO(int x, const sockaddr *y, socklen_t z, const NetSession &s) : 
		PollIO(x), closing(false), len(z), assoc_session(s.Clone())
	{
		addr = (sockaddr *)malloc(len);
		memcpy(addr, y, len);
		int rv = connect(fd, addr, len);
		if(rv == -1 && errno !=EINPROGRESS)
		{
			//connect error
			//perror("connect error");
			Close();
			PollIO::WakeUp();
		}
	}

	int UpdateEvent()
	{
		return closing ? -1 : (POLLIN | POLLOUT);
	}

	void PollIn()
	{
		Close();
	}

	void PollOut()
	{
		Close();
	}

	bool closing;
	socklen_t len;
	struct sockaddr *addr;
	NetSession *assoc_session;
public:
	~ActiveIO()
	{
		int rv = connect(fd, addr, len);
		if (rv == 0 || rv == -1 && errno == EISCONN)
		{
			//这里为何设置了阻塞？ 先暂时注释掉，察看一下情况
			//fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
			(new NetIO(fd, assoc_session))->Register();
			
			PollIO::WakeUp();
		}
		else
		{
			assoc_session->OnAbort(NULL);
			assoc_session->Destroy();
			close(fd);
			assoc_session = NULL;
			fd = -1;
		}
		free(addr);
	}

	void Close() { closing = true; }

	static ActiveIO *Open(const NetSession &assoc_session,const sockaddr * inaddr = NULL, size_t inlen = 0) 
	{
		Conf *conf = Conf::GetInstance();
		Conf::section_type section = assoc_session.Identification();
		Conf::value_type type = conf->find(section, "type");

		int s, optval = 1;
		if (!strcasecmp(type.c_str(), "tcp"))
		{
			struct sockaddr_in addr;
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = inet_addr(conf->find(section, "address").c_str());
			std::string tmpStr = conf->find(section, "port");
			addr.sin_port = htons(atoi(tmpStr.c_str()));
			fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);

			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

			optval = atoi(conf->find(section, "so_sndbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "so_rcvbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "tcp_nodelay").c_str());
			if (optval) setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
			if(inaddr == NULL) {
				inaddr = (sockaddr *) &addr;
				inlen = sizeof(addr);
			}

			if (s != -1)
			{
				ActiveIO * pIO = new ActiveIO(s,inaddr,inlen, assoc_session);
				pIO->Register();
				return pIO;
			}
		}
		else if (!strcasecmp(type.c_str(), "unix"))
		{
			struct sockaddr_un addr;
			s = socket(AF_UNIX, SOCK_STREAM, 0);
			addr.sun_family = AF_UNIX;
			strncpy(addr.sun_path, conf->find(section, "address").c_str(), sizeof(addr.sun_path)-1);
			fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);

			optval = atoi(conf->find(section, "so_sndbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "so_rcvbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));

			if(inaddr == NULL) {
				inaddr = (sockaddr *) &addr;
				inlen = sizeof(addr);
			}
			if (s != -1)
			{
				ActiveIO * pIO = new ActiveIO(s,inaddr,inlen, assoc_session);
				pIO->Register();
				return pIO;
			}
		}
		return NULL;
	}
};

};

#endif
