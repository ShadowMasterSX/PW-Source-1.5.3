/*
		POLL IO ，提供了一个使用EPOLL多路复用进行连接管理的接口。
		作者: 崔铭
		公司：完美时空
		日期：2009-06-08

2011-06-16修改： 
		增加了应用层的延迟功能，用于避免非常多的包发送。 默认这个延迟功能是关闭的。 
		对于Session，应该调用Manager的  SetSessionDelayMode 来开启，例如： manager.SetSessionDelayMode(session_id, false);
		这个延迟有两个全局参数可以调整，延时的时间和数据包的大小阈值，使用如下函数进行设置 ioman.SetDelayThreshold(50,500); 这个值应该只在初始化时设置。
*/


#ifndef __GNET_IOMAN_POLLIO_H
#define __GNET_IOMAN_POLLIO_H
#include "marshal.h"
#include "ASSERT.h"

namespace GNET
{

class PollIO;
class IOManImp;
class IOMan			//IO管理器
{
	IOManImp * _imp;
	friend class PollIO;
	friend class StreamIO;
	void UpdateIOEventMask(PollIO * io, int mask1);		//纯内部使用，外面不要用
public:
	Marshal::OctetsStream input_data;	//use in protocol.cpp

	struct IO_EVENT_T 
	{
		struct CONTROL_T
		{
			int mask1;
			int mask2;
			bool update;
		}control[2];

		IO_EVENT_T()
		{
			control[0].mask1 = 0;
			control[0].mask2 = 0;
			control[0].update = false;
			control[1].mask1 = 0;
			control[1].mask2 = 0;
			control[1].update = false;
		}
	};


	IOMan();
	~IOMan();

	PollIO* Register(PollIO *io, bool init_permit_recv, bool init_permit_send);
	int Init(bool single_thread);
	void WakeUp();
	void Poll(int timeout);
	bool IsSingleThread();
	unsigned int GetPollCount();	//取回调用poll的次数
	void Fini();			//将IOMan析构，将所有注册的PollIO析构. 注意，这个应该是最后调用，先要把timer,threadpool停了再调。
	bool IsFinalizing();		//是否处在析构中(在调用了Fini后，这个值会返回true)
	void SetDelayThreshold(int milisecond, uint32_t size = 500);	//对于设置了delay模式的IO，将会延迟多久进行发送，这个值不能小于20毫秒。 size 是一个字节限制，如果一次发送后累计超过这么多内容，则会试图提前发送 
public:
	static IOMan _man;	//为兼容模式准备的，平时也可以用
};

class PollIO			//IO的基类，一般不直接使用，NetIO 从它继承而来
{
protected:
	IOMan * _parent;			//这个IO所属那个IO Man
private:
	int  _cur_event;			//当前要通知的事件，这个变量由IOMan负责维护
	bool _tcp_nodelay;			//是否在内部执行类似tcp_delay的策略 ,如果是false，则加入一些延迟和积累。这是为了避免Nagle算法带来的不可控的延时和开启TCP_NODELAY后的高带宽占用，从而引入的一个平衡策略 默认值为true
	struct IOMan::IO_EVENT_T _event_notify;	//要修改到的新事件组合，这个变量由io对象修改，IOman使用
	struct timeval _send_timeval;		//最后一次发送数据的时间戳


	virtual void PollIn()  = 0;
	virtual void PollOut() {}
	virtual void PollClose() {}
	virtual void BeforeRelease() = 0;
	virtual void CommitRelease() {delete this;}
	virtual size_t IOWaitSize() const { return 0;}	//定义为还有多少字节等待发送 

	virtual void OnRegister() {}

	friend class IOMan;
	friend class IOManImp;
public:
	virtual bool CanTransferFD() const { return false;}

protected:
	int _fd;		//连接的文件描述符，在PollIO的构造函数里，这个描述符被设置为非阻塞IO

	virtual ~PollIO();
	PollIO(int fd);

public:	

	inline int GetFD() { return _fd;}

	void PermitRecv();
	void PermitSend();
	void ForbidRecv();
	void ForbidSend();
	void Close();

	inline void SetDelayMode(bool nodelay) { _tcp_nodelay = nodelay; }
	inline bool GetDelayMode() const { return _tcp_nodelay;}

	static bool CompatibleMode();	//与旧IO库接口兼容模式
	static bool VerboseMode();	//有大量信息输出模式

#ifdef __OLD_IOLIB_COMPATIBLE__
	static void WakeUp()
	{
		IOMan::_man.WakeUp();
	}
	static int Init(bool single_thread_mode)
	{
		if (!CompatibleMode()) 
		{
			ASSERT(false && "所使用的IO动态库不支持兼容模式，请检查");
		}
		return IOMan::_man.Init(single_thread_mode);
	}

	static void Poll(int t)
	{
		IOMan::_man.Poll(t);
	}
#endif
};

};

#endif

