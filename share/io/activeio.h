#ifndef __ACTIVEIO_H
#define __ACTIVEIO_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "conf.h"
#include "pollio.h"
#include "netio.h"
#include "variant.h"

namespace GNET
{
class ActiveIO : PollIO
{
	enum Type { STREAM, DGRAM };
	Type type;
	NetSession *assoc_session;
	SockAddr sa;

	void PollIn()  { Close(); }
	void PollOut() { Close(); }

	ActiveIO(int x, const SockAddr &saddr, const NetSession &s, Type t) : PollIO(x), type(t), assoc_session(s.Clone()), sa(saddr)
	{
		assoc_session->LoadConfig();
		connect(fd, sa, sa.GetLen());
	}
	ActiveIO(int x, const SockAddr &saddr, const NetSession &s, Type t, variant conf) : PollIO(x), type(t), assoc_session(s.Clone()), sa(saddr)
	{
		assoc_session->LoadConfig( conf );
		connect(fd, sa, sa.GetLen());
	}
	~ActiveIO()
	{
		if (type == STREAM)
		{
			int optval = -1;
			socklen_t optlen = sizeof(optval);
			int optret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);
			if ( optret == 0 && optval == 0 )
			{
				PollIO::Register(new StreamIO(dup(fd), assoc_session), true, false);
			}
			else
			{
				int rv = connect(fd, sa, sa.GetLen());
				if (rv == 0 || rv == -1 && errno == EISCONN)
					PollIO::Register(new StreamIO(dup(fd), assoc_session), true, false);
				else
				{
					assoc_session->OnAbort(sa);
					assoc_session->Destroy();
				}
			}
		}
		else 
			PollIO::Register(new DgramClientIO(dup(fd), assoc_session, sa), true, false);
	}

public:
	static ActiveIO *Open(const NetSession &assoc_session) 
	{
		Conf *conf = Conf::GetInstance();
		Conf::section_type section = assoc_session.Identification();
		Conf::value_type type = conf->find(section, "type");
		SockAddr sa;

		int s = -1, optval = 1;
		Type t = STREAM;
		if (!strcasecmp(type.c_str(), "tcp"))
		{
			struct sockaddr_in *addr = sa;
			memset(addr, 0, sizeof(*addr));
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(conf->find(section, "address").c_str());
			unsigned short port=atoi(conf->find(section, "port").c_str()); // for icpc bugs
			addr->sin_port = htons(port);
			assoc_session.OnCheckAddress(sa);

			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

			optval = atoi(conf->find(section, "so_sndbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "so_rcvbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "tcp_nodelay").c_str());
			if (optval) setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
		}
		else if (!strcasecmp(type.c_str(), "udp"))
		{
			t = DGRAM;
			struct sockaddr_in *addr = sa;
			memset(addr, 0, sizeof(*addr));
			s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(conf->find(section, "address").c_str());
			unsigned short port=atoi(conf->find(section, "port").c_str()); // for icpc bugs
			addr->sin_port = htons(port);
			assoc_session.OnCheckAddress(sa);

			optval = atoi(conf->find(section, "so_broadcast").c_str());
			if (optval)
				setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "so_sndbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "so_rcvbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
		}
		else if (!strcasecmp(type.c_str(), "unix"))
		{
			struct sockaddr_un *addr = sa;
			memset(addr, 0, sizeof(*addr));
			s = socket(AF_UNIX, SOCK_STREAM, 0);
			addr->sun_family = AF_UNIX;
			strncpy(addr->sun_path, conf->find(section, "address").c_str(), sizeof(addr->sun_path)-1);
			assoc_session.OnCheckAddress(sa);

			optval = atoi(conf->find(section, "so_sndbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(conf->find(section, "so_rcvbuf").c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
		}
		return s != -1 ? (ActiveIO*)PollIO::Register(new ActiveIO(s, sa, assoc_session, t), true, true) : NULL;
	}

	static ActiveIO *Open(const NetSession &assoc_session, variant v ) 
	{
		variant conf = v.Clone();
		SockAddr sa;
		int s = -1, optval = 1;
		Type t = STREAM;
		std::string type         = conf("type");
		std::string address      = conf("address");
		std::string port         = conf("port");
		std::string so_sndbuf    = conf("so_sndbuf");
		std::string so_rcvbuf    = conf("so_rcvbuf");
		std::string tcp_nodelay  = conf("tcp_nodelay");
		std::string so_broadcast = conf("so_broadcast");
		if (!strcasecmp(type.c_str(), "tcp"))
		{
			struct sockaddr_in *addr = sa;
			memset(addr, 0, sizeof(*addr));
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(address.c_str());
			addr->sin_port = htons(atoi(port.c_str()));
			assoc_session.OnCheckAddress(sa);

			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

			optval = atoi(so_sndbuf.c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(so_rcvbuf.c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
			optval = atoi(tcp_nodelay.c_str());
			if (optval) setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
		}
		else if (!strcasecmp(type.c_str(), "udp"))
		{
			t = DGRAM;
			struct sockaddr_in *addr = sa;
			memset(addr, 0, sizeof(*addr));
			s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(address.c_str());
			addr->sin_port = htons(atoi(port.c_str()));
			assoc_session.OnCheckAddress(sa);

			optval = atoi(so_broadcast.c_str());
			if (optval)
				setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
			optval = atoi(so_sndbuf.c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(so_rcvbuf.c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
		}
		else if (!strcasecmp(type.c_str(), "unix"))
		{
			struct sockaddr_un *addr = sa;
			memset(addr, 0, sizeof(*addr));
			s = socket(AF_UNIX, SOCK_STREAM, 0);
			addr->sun_family = AF_UNIX;
			strncpy(addr->sun_path, address.c_str(), sizeof(addr->sun_path)-1);
			assoc_session.OnCheckAddress(sa);

			optval = atoi(so_sndbuf.c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(so_rcvbuf.c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
		}
		return s != -1 ? (ActiveIO*)PollIO::Register(new ActiveIO(s, sa, assoc_session, t, conf), true, true) : NULL;
	}
};

};

#endif
