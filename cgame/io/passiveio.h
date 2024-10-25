#ifndef __PASSIVEIO_H
#define __PASSIVEIO_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "conf.h"
#include "pollio.h"
#include "netio.h"
#include <errno.h>

namespace ONET
{

class PassiveIO : PollIO
{
	NetSession *assoc_session;
	bool closing;

	int UpdateEvent()
	{
		return closing ? -1 : POLLIN;
	}

	void PollIn()
	{
		int s = accept(fd, 0, 0);
		if (s != -1)
		{
			(new NetIO(s, assoc_session->Clone()))->Register();
			PollIO::WakeUp();
		}
	}

	PassiveIO (int x, const NetSession &y) : PollIO(x), assoc_session(y.Clone()), closing(false) { }
	static bool MakeIPAddr(sockaddr_in & addr,const char * ipaddr)
	{      
		memset(&addr,0,sizeof(addr));
		char buf[64];
		memset(buf,0,sizeof(buf));
		strncpy(buf,ipaddr,sizeof(buf) - 1);
		char * sp = strchr(buf,':');
		if(sp == NULL) return false;
		*sp++ = 0;      
		addr.sin_family = AF_INET;
		addr.sin_port = htons(atoi(sp));
		addr.sin_addr.s_addr = inet_addr(buf);
		return true;
	}

public:
	virtual ~PassiveIO ()
	{
		assoc_session->Destroy();
		close(fd);
	}

	void Close() { closing = true; }

	static PassiveIO *Open(const NetSession &assoc_session)
	{
		Conf *conf = Conf::GetInstance();
		Conf::section_type section = assoc_session.Identification(); 
		Conf::value_type type = conf->find(section, "type");

		int s, optval = 1;
		if (!strcasecmp(type.c_str(), "tcp"))
		{
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			std::string str = conf->find(section, "listen_addr");
			MakeIPAddr(addr,str.c_str());
			if (addr.sin_addr.s_addr == INADDR_NONE) addr.sin_addr.s_addr = INADDR_ANY;

			setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

			optval = atoi(conf->find(section, "so_sndbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "so_rcvbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "tcp_nodelay").c_str());
			if (optval) setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

			if(bind (s, (sockaddr *)&addr, sizeof(addr))== -1){
				close(s);
				s = -1;
			}
		}
		else if (!strcasecmp(type.c_str(), "unix"))
		{
			s = socket(AF_UNIX, SOCK_STREAM, 0);
			struct sockaddr_un addr;
			addr.sun_family = AF_UNIX;

			strncpy(addr.sun_path, conf->find(section, "listen_addr").c_str(),sizeof(addr.sun_path) - 1);
			addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
			unlink(addr.sun_path);

			optval = atoi(conf->find(section, "so_sndbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "so_rcvbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));

			if(bind (s, (sockaddr *)&addr, sizeof(addr)) == -1)
			{
				printf("errno %d\n",errno);
				close(s);
				s = -1;
			}
			else
			{
				chmod(addr.sun_path, 0666);
			}
		}
		else
			s = -1;

		if(s != -1)
		{
			optval = atoi(conf->find(section, "listen_backlog").c_str());
			listen (s, optval ? optval : SOMAXCONN);
			PassiveIO * pIO = new PassiveIO(s, assoc_session);
			pIO->Register();
			return pIO;
		}
		return NULL;
	}
};

};

#endif
