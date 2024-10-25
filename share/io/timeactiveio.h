#ifndef __TIMEACTIVEIO_H
#define __TIMEACTIVEIO_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <map>

#include "conf.h"
#include "pollio.h"
#include "netio.h"
#include "timer.h"
#include "mutex.h"
#include "maperaser.h"
#include "variant.h"

namespace GNET
{

class TimeActiveIO : PollIO
{
	class Checker : Timer::Observer
	{
		class Bundle
		{
			Timer timer;
			int sock;
			int timeout;
		public:
			~Bundle() { close(sock); }
			Bundle( int s, int timo ) : sock(dup(s)), timeout(timo) { }
			bool CheckTimeout()
			{
				bool rv = timer.Elapse() > timeout;
				if ( rv ) shutdown( sock, SHUT_RDWR );
				return rv;
			}
		};
		typedef std::map< int, Bundle* > Map;
		Map map;
		Thread::Mutex locker_map;
		static Checker checker;
		Checker() : locker_map("TimeActiveIO::Checker::locker_map") { Timer::Attach( this ); }
		void Update()
		{
			Thread::Mutex::Scoped l(locker_map);
			MapEraser<Map> e(map);
			for ( Map::iterator it = map.begin(), ie = map.end(); it != ie; ++it )
				if ( (*it).second->CheckTimeout() )
				{
					delete (*it).second;
					e.push( it );
				}
		}
	public:
		void Monitor( int s, int timeout )
		{
			Thread::Mutex::Scoped l(locker_map);
			map.insert( std::make_pair ( s, new Bundle(s, timeout) ) );
		}

		void Unmonitor( int s )
		{
			Thread::Mutex::Scoped l(locker_map);
			Map::iterator it = map.find(s);
			if ( it != map.end() )
			{
				delete (*it).second;
				map.erase(it);
			}
		}

		static Checker& GetInstance() { return checker; }
	};

	NetSession *assoc_session;
	SockAddr sa;

	void PollIn()  { Close(); }
	void PollOut() { Close(); }

	TimeActiveIO(int x, const SockAddr &saddr, const NetSession &s, int timeout) : PollIO(x), assoc_session(s.Clone()), sa(saddr)
	{
		assoc_session->LoadConfig();
		connect(fd, sa, sa.GetLen());
		Checker::GetInstance().Monitor( fd, timeout );
	}

	TimeActiveIO(int x, const SockAddr &saddr, const NetSession &s, int timeout, variant conf) : PollIO(x), assoc_session(s.Clone()), sa(saddr)
	{
		assoc_session->LoadConfig( conf );
		connect(fd, sa, sa.GetLen());
		Checker::GetInstance().Monitor( fd, timeout );
	}
public:
	~TimeActiveIO()
	{
		Checker::GetInstance().Unmonitor( fd );
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

	static TimeActiveIO *Open(const NetSession &assoc_session, int timeout = 5) 
	{
		Conf *conf = Conf::GetInstance();
		Conf::section_type section = assoc_session.Identification();
		Conf::value_type type = conf->find(section, "type");
		SockAddr sa;

		int s = -1, optval = 1;
		if (!strcasecmp(type.c_str(), "tcp"))
		{
			struct sockaddr_in *addr = sa;
			memset(addr, 0, sizeof(*addr));
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(conf->find(section, "address").c_str());
			unsigned short port=atoi(conf->find(section, "port").c_str());  //for icpc bugs
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
		return s != -1 ? (TimeActiveIO*)PollIO::Register(new TimeActiveIO(s, sa, assoc_session, timeout), true, true) : NULL;
	}

	static TimeActiveIO *Open(const NetSession &assoc_session, variant v, int timeout = 5 ) 
	{
		variant conf = v.Clone();
		SockAddr sa;

		int s = -1, optval = 1;
		std::string type         = conf("type");
		std::string address      = conf("address");
		std::string port         = conf("port");
		std::string so_sndbuf    = conf("so_sndbuf");
		std::string so_rcvbuf    = conf("so_rcvbuf");
		std::string tcp_nodelay  = conf("tcp_nodelay");
		if (!strcasecmp(type.c_str(), "tcp"))
		{
			struct sockaddr_in *addr = sa;
			memset(addr, 0, sizeof(*addr));
			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			addr->sin_family = AF_INET;
			addr->sin_addr.s_addr = inet_addr(address.c_str());
			addr->sin_port = htons( atoi(port.c_str()) );
			assoc_session.OnCheckAddress(sa);

			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));

			optval = atoi(so_sndbuf.c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
			optval = atoi(so_rcvbuf.c_str());
			if (optval) setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
			optval = atoi(tcp_nodelay.c_str());
			if (optval) setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
		}
		return s != -1 ? (TimeActiveIO*)PollIO::Register(new TimeActiveIO(s, sa, assoc_session, timeout, conf), true, true) : NULL;
	}


};

};

#endif
