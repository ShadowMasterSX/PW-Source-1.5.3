#ifndef __NETIO_H
#define __NETIO_H

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <stdlib.h>
#include <map>

#include "octets.h"
#include "pollio.h"
#include "security.h"
#include "thread.h"
#include "conf.h"
#include "log.h"
#include "maperaser.h"
#include "variant.h"
namespace GNET
{

class SockAddr
{
	Octets addr;
public:	
	SockAddr() {}
	template<typename T> SockAddr(const T &sa) : addr(&sa, sizeof(sa)) { }
	SockAddr(const SockAddr &rhs) : addr(rhs.addr) { }
	socklen_t GetLen() const { return addr.size(); }
	template<typename T> operator T* () { addr.resize(sizeof(T)); return (T *)addr.begin(); }
	template<typename T> operator const T* () const { return (const T *)addr.begin(); }
};

enum { 
	CLOSE_ONRECV     = 0x10000, // 对端调用close正常关闭
	CLOSE_ONRESET    = 0x20000, // 连接被对端reset
	CLOSE_ONSEND     = 0x30000, // 发送时发生错误
	CLOSE_ACTIVE     = 0x40000, // 本端主动关闭连接
	CLOSE_ONERROR    = 0x50000, // 协议解析或者状态错误
	CLOSE_TIMEOUT    = 0x60000, // Session状态超时
	CLOSE_EXPIRE     = 0x70000, // 本端由于TTL超时主动关闭
	CLOSE_EXCEPTION  = 0x80000, // 处理协议时产生异常
};


class NetSession
{
	friend class NetIO;
	friend class StreamIO;
	friend class DgramServerIO;
	friend class DgramClientIO;
	enum { DEFAULTIOBUF = 8192 };
	PollIO  *assoc_io;
	bool	sending;
	bool    closing;
	bool    close_discard;
	int     status;
	Octets	ibuffer;
	Octets	obuffer;
	Octets  isecbuf;
	Octets  osecbuf;
	Security *isec;
	Security *osec;

	Octets& GetOBuffer()
	{
		osec->Update(osecbuf);
		obuffer.insert(obuffer.end(), osecbuf.begin(), osecbuf.end());
		osecbuf.clear();
		return obuffer;
	}

	Octets& GetIBuffer()
	{
		return ibuffer;
	}

	void SendFinish()
	{
		if ( NoMoreData() )
		{
			sending = false;
			assoc_io->ForbidSend();
		}
	}

protected:
	Thread::Mutex locker;
	bool urgency_support;
	virtual ~NetSession ()
	{
		isec->Destroy();
		osec->Destroy();
	}
	NetSession(const NetSession &rhs) : assoc_io(rhs.assoc_io), sending(false), closing(false), close_discard(rhs.close_discard),status(0),
		isec(rhs.isec->Clone()), osec(rhs.osec->Clone()), locker("NetSession::locker",true), urgency_support(rhs.urgency_support)
	{
		ibuffer.reserve(rhs.ibuffer.capacity());
		obuffer.reserve(rhs.obuffer.capacity());
	}

	bool Output(const Octets &data)
	{
		if (obuffer.size() + osecbuf.size() + data.size() > obuffer.capacity())
		{
			Log::log(LOG_ERR,"FATAL, data overflow MAXIUM buffer size, obuffer=%d(capacity=%d),osecbuf=%d,data=%d",
					obuffer.size(),obuffer.capacity(),osecbuf.size(),data.size());
		   	return false;
		}
		osecbuf.insert(osecbuf.end(), data.begin(), data.end());
		return true;
	}

	Octets& Input()
	{
		{
		Thread::Mutex::Scoped l(locker);
		isec->Update(ibuffer);
		}
		isecbuf.insert(isecbuf.end(), ibuffer.begin(), ibuffer.end());
		ibuffer.clear();
		assoc_io->PermitRecv();
		return isecbuf;
	}

	void SendReady()
	{
		if ( sending ) return;
		sending = true;
		assoc_io->PermitSend();
	}

	void SetISecurity(Security::Type type, const Octets &key)
	{
		Thread::Mutex::Scoped l(locker);
		isec->Destroy();
		isec = Security::Create(type);
		isec->SetParameter(key);
	}
	
	void SetOSecurity(Security::Type type, const Octets &key)
	{
		Thread::Mutex::Scoped l(locker);
		osec->Destroy();
		osec = Security::Create(type);
		osec->SetParameter(key);
	}
public:
	NetSession() : assoc_io(NULL), sending(false), closing(false), close_discard(false),status(0),ibuffer(DEFAULTIOBUF), obuffer(DEFAULTIOBUF), 
		isec(Security::Create(NULLSECURITY)), osec(Security::Create(NULLSECURITY)), locker("NetSession::locker",true)
		, urgency_support(false) { }

	void LoadConfig()
	{
		Conf *conf = Conf::GetInstance();
		Conf::section_type section = Identification();
		size_t size;
		if ((size = atoi(conf->find(section, "ibuffermax").c_str()))) ibuffer.reserve(size);
		if ((size = atoi(conf->find(section, "obuffermax").c_str()))) obuffer.reserve(size);
		if ((size = atoi(conf->find(section, "close_discard").c_str()))) close_discard = size;
		if ((size = atoi(conf->find(section, "urgency_support").c_str()))) urgency_support = size;
		if ((size = atoi(conf->find(section, "isec").c_str()))) 
		{
			Conf::value_type key = conf->find(section, "iseckey");
			SetISecurity(size, Octets(&key[0], key.size()));
		}
		if ((size = atoi(conf->find(section, "osec").c_str()))) 
		{
			Conf::value_type key = conf->find(section, "oseckey");
			SetOSecurity(size, Octets(&key[0], key.size()));
		}
	}

	void LoadConfig( variant conf )
	{
		if ( conf.exists("ibuffermax") ) ibuffer.reserve(atoi(static_cast<std::string>(conf("ibuffermax")).c_str()));
		if ( conf.exists("obuffermax") ) obuffer.reserve(atoi(static_cast<std::string>(conf("obuffermax")).c_str()));
		if ( conf.exists("close_discard") ) close_discard = atoi(static_cast<std::string>(conf("close_discard")).c_str());
		if ( conf.exists("urgency_support") ) urgency_support = atoi(static_cast<std::string>(conf("urgency_support")).c_str());
		if ( conf.exists("isec") && conf.exists("iseckey") )
		{
			std::string type = conf("isec");
			std::string key  = conf("iseckey");
			SetISecurity( atoi(type.c_str()), Octets(&key[0], key.size()));
		}
		if ( conf.exists("osec") && conf.exists("oseckey") )
		{
			std::string type = conf("osec");
			std::string key  = conf("oseckey");
			SetOSecurity( atoi(type.c_str()), Octets(&key[0], key.size()));
		}
	}

	void Close( int code=0, bool locked=false)
	{
		if (locked)
		{
			_Close();
		}
		else
		{
			Thread::Mutex::Scoped l(locker);
			_Close();
		}
		if(!status)
			status = code;
	}

	virtual std::string Identification () const = 0;
	virtual bool NoMoreData() const = 0;
	virtual void OnRecv() = 0;
	virtual void OnSend() = 0;
	virtual void OnOpen(const SockAddr& local, const SockAddr& peer) { }
	virtual void OnOpen() { }
	virtual void OnClose() { }
	virtual void OnAbort(const SockAddr& sa) { }
	virtual	NetSession* Clone() const = 0;
	virtual void Destroy() { delete this; }
	virtual void OnCheckAddress(SockAddr &) const { }
	int GetStatus() {  return status; }

private:
	void _Close()
	{
		if ( closing ) return;
		closing = true;
		// thread-safe-window . lock outside
		assoc_io->PermitSend();
	}
};

class NetIO : public PollIO
{
protected:
	NetSession *session;
	NetIO(int fd, NetSession *s) : PollIO(fd), session(s) { s->assoc_io = this; }
};

class StdInIO : public PollIO
{
protected:
	StdInIO() : PollIO(0) { }
public:
	void PollIn()
	{
		std::string line;
		std::getline(std::cin, line);
		OnGetLine(line);
	}
	void PollOut()
	{
	}
	void PollClose()
	{
	}
	static void Start(StdInIO *stdinio)
	{
		Register(stdinio, true, false);
	}
	virtual void OnGetLine(std::string line) = 0;
};

class StreamIO : public NetIO
{
	void PollIn()
	{
		int recv_bytes;
		Octets& ibuf = session->GetIBuffer();
		do
		{
			if ((recv_bytes = read(fd, ibuf.end(), ibuf.capacity() - ibuf.size())) > 0)
			{
				ibuf.resize(ibuf.size() + recv_bytes);
				session->OnRecv();
				if ( ibuf.size() == ibuf.capacity() )
					ForbidRecv();
				return;
			}
		} while (recv_bytes == -1 && errno == EINTR);

		if (recv_bytes != -1 || errno != EAGAIN)
		{
			Thread::Mutex::Scoped l(session->locker);
			session->OnSend();
			session->GetOBuffer().clear();
			if(recv_bytes==0)
				session->Close(CLOSE_ONRECV, true);
			else
				session->Close(CLOSE_ONRESET|errno, true);
		}
	}

	void PollOut()
	{
		int send_bytes;
		Thread::Mutex::Scoped l(session->locker);
		session->OnSend();
		Octets& obuf = session->GetOBuffer();
		do
		{
			if ((send_bytes = write(fd, obuf.begin(), obuf.size())) > 0)
			{
				obuf.erase(obuf.begin(), (char*)obuf.begin() + send_bytes);
				if ( obuf.size() == 0 )
					session->SendFinish();
				return;
			}
		} while (send_bytes == -1 && errno == EINTR);

		if (send_bytes != -1 || errno != EAGAIN)
		{
			obuf.clear();
			session->Close(CLOSE_ONSEND|errno, false);
		}
	}

	void PollClose()
	{
		Thread::Mutex::Scoped l(session->locker);
		if ( session->closing && ( session->close_discard || session->GetOBuffer().size() == 0 ))
		{
			Close();
		}
	}
public:
	~StreamIO()
	{
		session->OnClose();
		session->Destroy();
	}

	StreamIO(int fd, NetSession *s) : NetIO(fd, s)
	{
		struct sockaddr_in local, peer;
		socklen_t local_len = sizeof(local);
		socklen_t peer_len  = sizeof(peer);
		if ( getsockname(fd, (struct sockaddr *)&local, &local_len) == -1 || getpeername(fd, (struct sockaddr *)&peer, &peer_len) == -1 )
			session->OnOpen();
		else
			session->OnOpen(SockAddr(local), SockAddr(peer));
	}
};

class DgramClientIO : public NetIO
{
	SockAddr peer;
	void PollIn()
	{
		int recv_bytes;
		Octets& ibuf = session->GetIBuffer();
		do
		{
			if ((recv_bytes = read(fd, ibuf.end(), ibuf.capacity() - ibuf.size())) > 0)
			{
				ibuf.resize(ibuf.size() + recv_bytes);
				session->OnRecv();
				return;
			}
		} while (recv_bytes == -1 && errno == EINTR);
	}

	void PollOut()
	{
		Thread::Mutex::Scoped l(session->locker);
		session->OnSend();
		Octets& obuf = session->GetOBuffer();
		sendto(fd, obuf.begin(), obuf.size(), 0, peer, peer.GetLen());
		obuf.clear();
		if ( session->closing )
			Close();
		else
			session->SendFinish();
	}
public:
	~DgramClientIO()
	{
		session->OnClose();
		session->Destroy();
	}

	DgramClientIO(int fd, NetSession *s, const SockAddr &sa) : NetIO(fd, s), peer(sa) 
	{
		struct sockaddr_in local;
		socklen_t local_len = sizeof(local);
		if ( getsockname(fd, (struct sockaddr *)&local, &local_len) == -1 )
			session->OnOpen();
		else
			session->OnOpen(SockAddr(local), peer);
	}
};

class DgramServerIO : public NetIO
{
	struct compare_SockAddr
	{
		bool operator() (const SockAddr &sa1, const SockAddr &sa2) const
		{
			const struct sockaddr_in *s1 = sa1;
			const struct sockaddr_in *s2 = sa2;

			return	s1->sin_addr.s_addr < s2->sin_addr.s_addr  ||
				s1->sin_addr.s_addr == s2->sin_addr.s_addr && 
				s1->sin_port < s2->sin_port;
		}
	};
	typedef std::map<SockAddr, NetSession *, compare_SockAddr> Map;
	Map map;

	void PollIn()
	{
		struct sockaddr_in local, peer;
		socklen_t peer_len;
		int recv_bytes;
		Octets& ibuf = session->GetIBuffer();
		do
		{
			peer_len = sizeof(peer);
			while ((recv_bytes = recvfrom(fd, ibuf.begin(), ibuf.capacity(), 0, (struct sockaddr*)&peer, &peer_len)) > 0)
			{
				Thread::Mutex::Scoped l(session->locker);
				Map::iterator it = map.find(peer);
				if (it == map.end())
				{
					NetSession *s = session->Clone();
					it = map.insert(it, std::make_pair(peer, s));
					socklen_t local_len = sizeof(local);
					getsockname(fd, (struct sockaddr *)&local, &local_len);
					s->OnOpen(SockAddr(local), SockAddr(peer));
				}
				NetSession *s = (*it).second;
				Octets &i = s->GetIBuffer();
				if (i.size() + recv_bytes < i.capacity())
					i.insert(i.end(), ibuf.begin(), recv_bytes);
				s->OnRecv();
			}
		} while (recv_bytes == -1 && errno == EINTR);
	}

	void PollOut()
	{
		Thread::Mutex::Scoped l(session->locker);
		MapEraser<Map> e(map);
		for (Map::iterator it = map.begin(), ie = map.end(); it != ie; ++it)
		{
			const SockAddr &sa  = (*it).first;
			NetSession *session = (*it).second;
			session->OnSend();
			Octets& obuf = session->GetOBuffer();
			if (obuf.size())
			{
				sendto(fd, obuf.begin(), obuf.size(), 0, sa, sa.GetLen());
				obuf.clear();
			}
			if (session->closing)
			{
				e.push(it);
				session->OnClose();
				session->Destroy();
			}
			else
				session->SendFinish();
		}
		session->OnSend();
		if ( session->closing )
			Close();
		else
			session->SendFinish();
	}
public:
	~DgramServerIO()
	{
		for (Map::iterator it = map.begin(), ie = map.end(); it != ie; ++it)
		{
			NetSession *session = (*it).second;
			session->OnClose();
			session->Destroy();
		}
		session->Destroy();
	}

	DgramServerIO(int fd, NetSession *s) : NetIO(fd, s)
	{
	}
};

};

#endif
