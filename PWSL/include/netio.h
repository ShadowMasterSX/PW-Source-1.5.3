/*
		用于实际使用的NetIO类型，分别支持Stream(TCP)和Dgram(UDP)。
		作者：未知
		修改：2009-06-16 崔铭 重新组织了代码结构，将大部分代码移动到了源文件中 同时对StreamIO的PollOut进行了彻底的修改 试图减少内存复制/移动 和加锁的时间。
		公司：完美时空
*/

#ifndef __NETIO_H
#define __NETIO_H

#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>

#include "octets.h"
#include "pollio.h"

namespace GNET
{

struct SockAddr
{
	Octets addr;
public:	
	SockAddr() {}
	template<typename T> SockAddr(const T &sa) : addr(&sa, sizeof(sa)) { }
	SockAddr(const SockAddr &rhs) : addr(rhs.addr) { }
	socklen_t GetLen() const { return addr.size(); }

	bool operator < (SockAddr & rhs) const { return addr < rhs.addr;}
	template<typename T> operator T* () { addr.resize(sizeof(T)); return (T *)addr.begin(); }
	template<typename T> operator const T* () const { return (const T *)addr.begin(); }
};

enum { 
	CLOSE_ONRECV  	= 0x10000, // 对端调用close正常关闭
	CLOSE_ONRESET 	= 0x20000, // 连接被对端reset
	CLOSE_ONSEND  	= 0x30000, // 发送时发生错误
	CLOSE_ACTIVE  	= 0x40000, // 本端主动关闭连接
	CLOSE_ONERROR 	= 0x50000, // 协议解析或者状态错误
	CLOSE_TIMEOUT 	= 0x60000, // Session状态超时
	CLOSE_EXPIRE  	= 0x70000, // 本端由于TTL超时主动关闭
	CLOSE_EXCEPTION = 0x80000, // 处理协议时产生异常
	CLOSE_DETACH	= 0x90000, // SESSION被Detach出去
};

class NetSession;
class NetIO : public PollIO
{
protected:
	NetSession *session;
	NetIO(int fd, NetSession *s);

	virtual size_t IOWaitSize() const;
};

class StreamIO : public NetIO
{
	virtual void PollIn();
	virtual void PollOut();
	virtual void PollClose();
	virtual void OnRegister();
	virtual void BeforeRelease();
	inline int SendOBuffer(Octets & obuf, size_t & obuffer_size);
public:
	~StreamIO();
	StreamIO(int fd, NetSession *s);
};

class UnixIO : public NetIO	//支持文件描述符传递的IO
{
	virtual void PollIn();
	virtual void PollOut();
	virtual void PollClose();
	virtual void OnRegister();
	virtual void BeforeRelease();
	virtual bool CanTransferFD() const { return true;}
public:
	~UnixIO();
	UnixIO(int fd, NetSession *s);
};

class DgramClientIO : public NetIO
{
	SockAddr peer;
	virtual void PollIn();
	virtual void PollOut();
	virtual void OnRegister();
	virtual void BeforeRelease();
public:
	~DgramClientIO();
	DgramClientIO(int fd, NetSession *s, const SockAddr &sa);
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
				(s1->sin_addr.s_addr == s2->sin_addr.s_addr && s1->sin_port < s2->sin_port);
		}
	};
	typedef std::map<SockAddr, NetSession *, compare_SockAddr> Map;
	Map map;

	void PollIn();
	void PollOut();
	virtual void BeforeRelease();
public:
	~DgramServerIO();
	DgramServerIO(int fd, NetSession *s);
};

};

#endif
