/*
	用于主动连接的ActiveIO和等待连接的PassiveIO； 其中ActiveIO会在连接成功以后转换成为StreamIO并关联内部保存的Session，而PassiveIO会在新连接进入后，创建一个StreamIO并关联内部session的clone对象。
	作者：未知
	修改：2009-07 杨延昭
	公司：完美时空

*/


#ifndef __ACTIVEIO_PASSIVEIO_H
#define __ACTIVEIO_PASSIVEIO_H


#include "pollio.h"
#include "netio.h"

namespace GNET
{
class NetSession;
class ActiveIO : PollIO
{
	enum Type { STREAM, DGRAM , STREAM_UNIX};
	Type _type;
	NetSession *_assoc_session;
	SockAddr _sa;

	void PollIn()  { Close(); } //不会调到这里来的。
	void PollOut() { Close(); CheckConnectResult();} //当收到POLLOUT事件时，表明非阻塞式连接的结果出来了, 检查成功与否。

	ActiveIO(int fd, const SockAddr &saddr, const NetSession &s, Type t);
	void CheckConnectResult();  //检查是否连接成功。成功的话，生出一个StreamIO

	~ActiveIO() { }
	void BeforeRelease();
public:
	static ActiveIO *Open(IOMan*, const NetSession &assoc_session);
};

class PassiveIO : public PollIO
{
	enum Type { STREAM, DGRAM ,STREAM_UNIX};
	NetSession *_assoc_session;
	Type _type;
	DgramServerIO *_dgram_server_io;

	void Accept();

	void PollIn() { Accept();}  //当发生可读事件时，表明有新的连接过来了

	PassiveIO (int x, const NetSession &y, Type t);
	virtual ~PassiveIO ();

	void BeforeRelease() {}
public:
	static PassiveIO *Open(IOMan*, const NetSession &assoc_session);

	PassiveIO * Dup() {return new PassiveIO(dup(_fd), *_assoc_session, _type); }	//复制一个，然后Register到新IoMan中
};

};

#endif
