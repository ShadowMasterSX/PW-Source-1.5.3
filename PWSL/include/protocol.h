/*
 *	本头文件主要实现了以下内容：
 *	协议的基类 class Procotol，为处理网络协议流而配合的Session类 class PSession ，管理一类连接和一族协议的管理器PManager。 
 *	RPC/ProxyRPC的基类 class TimerProtocol; RPC超时和存根管理器 class RpcTimeoutManager。
 *
 *	作者：崔铭 杨延昭
 *	修改：杨延昭 整个体系几乎完全变动，增加了关联每个Manager的协议族的概念；重写了所有代码 本文件只能在GCC下使用
 *	时间：2009-07
 *	公司：完美时空
 *
*/
#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

#include <vector>
#include <set>
#include <hashmap.h>
#include <sstream>
#include "marshal.h"
#include "gnet_timer.h"
#include "mutex.h"
#include "threadpool.h"
#include "netsession.h"
#include "log.h"

namespace GNET
{

class PSession;
class PManager;
class ProtocolStubManager;
class IOMan;
class ActiveIO;
class PassiveIO;
class TimerProtocol;

typedef uint32_t PROTOCOL_TYPE;
typedef uint32_t SESSION_ID;
enum
{
	INVALID_SESSION_ID = 0
};

class Protocol : public Marshal
{
public:
	struct Data {
	/*	这个Data应该实现的函数 这些函数在rpcgen都会自动生成，自定义的协议则需要手写
		必须实现且应该实现正确的函数，影响逻辑：
		PROTOCOL_TYPE GetType() const;								//返回协议类型
		GNET::Marshal::OctetsStream& marshal(GNET::Marshal::OctetsStream & os) const;		//打包
		const GNET::Marshal::OctetsStream& unmarshal(const GNET::Marshal::OctetsStream &os);	//从流中恢复
		int PriorPolicy( ) const;								//优先级 0表示是直接用pollio线程处理
		bool SizePolicy(size_t size) const;							//大小是否合法
		unsigned int MaxSize()	const;								//Data的最大可能大小是多少
		
		不影响逻辑的函数，但影响日志和输出调试信息
		std::stringstream& trace(std::stringstream& os) const;					//dump协议
		const char *GetName() const;								//协议名，会出现在日志中
		*/
	};
#ifdef __OLD_IOLIB_COMPATIBLE__
	typedef PROTOCOL_TYPE Type;			//兼容性定义
	typedef PManager Manager;			//兼容性定义
	Type type;
#endif
protected:
	Protocol() {}
	Protocol(PROTOCOL_TYPE type, ProtocolStubManager *stubman);
	Protocol(PROTOCOL_TYPE type);
	virtual ~Protocol(){}
public:
#ifdef __OLD_IOLIB_COMPATIBLE__
	virtual PROTOCOL_TYPE GetType() const { return type;}	//兼容性代码
	Protocol(const Protocol& rhs):type(rhs.type){}
	static ActiveIO*  Client(PManager * mananger);		//以Client方式启动manager
	static PassiveIO* Server(PManager * mananger);		//以Server方式启动manager
	static bool	  Dummy(PManager *manager);		//Dummy方式启动manager,这种方式下，不监听、不主动连接任何端口，只接受AttachSession
	static ActiveIO*  Client(PManager * mananger,const NetSession::Config & cfg);	//以Client方式启动manager,指定地址、端口等配置
	static PassiveIO* Server(PManager * mananger,const NetSession::Config & cfg);	//以Server方式启动manager,指定地址、端口等
#else
	virtual PROTOCOL_TYPE GetType() const= 0;		//协议的类型
	Protocol(const Protocol& rhs){}
#endif

	virtual void Process(PManager *, SESSION_ID sid) = 0;	//协议的处理函数
	virtual Protocol *Clone() const = 0;
	virtual void Destroy() { delete this; }
	virtual int  PriorPolicy() const { return 0; }		//协议的处理优先级。旧库中常被用来保证顺序, 现在已无效。顺序改用GetSequence接口。 在兼容模式下，这个函数的返回值被用作Sequence之用。
	virtual unsigned int MaxPolicySize() const {return 0;}	//返回0表示未开启发送encode优化
	virtual bool SizePolicy(size_t) const { return true; }	//协议大小的检查
	virtual int GetSequence() const { return -1;}		//是否需要保证顺序处理 <0表示无顺序，否则是顺序号
	virtual const char *GetName() const { return "noname"; }//协议的名称，仅仅用于调试输出
	virtual std::string trace() const {return std::string();}	//协议的内容，仅仅用于调试输出

	void Encode(Marshal::OctetsStream& os) const ;		 //将协议编码至输出流中 这函数实际上调用了PManager的接口

	class Exception 
	{
	public:
	 	enum ECODE 
		{ 
			GENERIC,	//通用错误（未知错误)
			STATE, 		//当前Session的State不允许处理该协议
			SIZE, 		//收到的协议长度超过了允许的最大长度
			TYPE, 		//收到了未知的协议型类
			UNMARSHAL,	//在进行协议的unmarshal时出错
		};

		ECODE code;		//异常代码
		SESSION_ID sid;		//产生异常的session
		PROTOCOL_TYPE type;	//产生异常的协议类型
		size_t size;		//产生异常时的size
		std::string state_name; //产生异常时的state名字
		int loglevel;

		Exception():code(GENERIC),sid(0), type(0), size(0), loglevel(LOG_DEBUG){}
		Exception(ECODE code, SESSION_ID sid, PROTOCOL_TYPE type, size_t size, const char* state_name, int loglevel):
		code(code), sid(sid), type(type), size(size), state_name(state_name), loglevel(loglevel) {}

		void Log() const
		{
			static const char *errstr[] = {"generic", "state", "size","unknown type", "unmarshal"};
			Log::log(loglevel, "Protocol %s error. sid=%d,type=%d,size=%d,state=%s",
				errstr[code], sid, type, size, state_name.c_str());
		}
	};
};

class SessionState
{
	typedef std::set<PROTOCOL_TYPE> Set;
	Set set;	//该状态所处理的协议
	Set ignore;	//在该状态下，收到后需要抛弃的协议。收到既不在set中，又不在ignore中的协议，会被断开.
	int timeout;
	std::string name;
	public:
	SessionState (PROTOCOL_TYPE *first, size_t len, int t = 0, const char* pname=NULL) : set(first, first+len),timeout(t)
	{ if (pname) name = pname;}
	SessionState (PROTOCOL_TYPE *first, PROTOCOL_TYPE *last, int t = 0, const char*pname = NULL) : set(first, last), timeout(t) 
	{ if (pname) name = pname;}
	void AddIgnore(PROTOCOL_TYPE t) {ignore.insert(t);} //收到这种协议需要抛弃
	void AddIgnore(PROTOCOL_TYPE *first, size_t len) {ignore.insert(first, first +len);}
	void AddIgnore(const SessionState *state) {ignore.insert(state->set.begin(), state->set.end());}
	bool IgnorePolicy(PROTOCOL_TYPE type) const {return ignore.find(type) != ignore.end();}
	bool TypePolicy(PROTOCOL_TYPE type) const { return set.find(type) != set.end(); } 
	bool TimePolicy(int t) const { return timeout <= 0 || t < timeout; }
	const char *GetName() const {return name.c_str();}
	void AddProtocol(PROTOCOL_TYPE type) { set.insert(type);}
};

struct FDTransferPData;
class PManager
{
	friend class PSession;
private:
	//对PSession的管理
	typedef abase::hash_map<SESSION_ID, PSession *> SessionMap;
	SessionMap _map;		//当前session的MAP
	RWLock _locker_session_map;	//上面Map的操作锁
	SESSION_ID _session_id;		//下一个产生Session的 ID，这是针对每个Manager都有一个空间的
	NetSession::Config _config;	//ActiveIO,PassiveIO及NetSession所需要的配置
	PassiveIO *_assoc_passiveio;	//该Manager所关联的PassiveIO
	IOMan * _ioman;
	bool _single_thread;
	Runnable *_check_session_task;	//用于检查session定期关闭和超时的task

	void AddSession(SESSION_ID sid, PSession *session);
	void DelSession(SESSION_ID sid, int status);
	void AbortSession(const SockAddr &sa, PSession *);
	PSession *GetSession(SESSION_ID sid);
	inline SESSION_ID NextSessionID();

	inline void ProcessProtocol(Protocol *p, SESSION_ID sid);
	static void PostEncode(PROTOCOL_TYPE, size_t size, const char *pname, const char *content);
	bool __internal_init_1(IOMan * ioman, const NetSession::Config* spec_config = NULL);
	bool __internal_init_2(IOMan * ioman);
protected:
	GNET::ThreadPool * _th_pool;		//线程池对象指针
public:
	int protocol_count;
#ifdef __OLD_IOLIB_COMPATIBLE__
	class Session
	{
		public:
		typedef SESSION_ID ID;
		typedef SessionState State;
	};
#endif
	PManager();
	virtual ~PManager();

	//以Server或Client方式启动PManager
	PassiveIO* InitServer(IOMan *ioman );		//以Server方式启动manager,监听某端口，有两种可能会返回NULL: 1.配置加载失败(conf::GetInstance未调用). 2.端口已被占用
	ActiveIO*  InitClient(IOMan *ioman );		//以Client方式启动manager,连接某端口，配置加载失败及socket创建失败时会返回NULL
	bool	   InitDummy(IOMan *ioman );		//以Dummy方式起动manager,这种方式下不监听、不主动连接任何端口，只接受外部传入session(AttachSession)
	PassiveIO* InitCopy(IOMan * ioman, const PManager * src);	//从原来的一个Manager复制配置和监听的端口（使用了dup2复制了一份fd） 线程池不会进行复制
	PassiveIO* InitServer(IOMan *ioman, const NetSession::Config &cfg);//以特定的配置来启动Server方式的Manager,不会再加载配置文件
	ActiveIO* InitClient(IOMan *ioman, const NetSession::Config &cfg); //以特定的配置来启动Client方式的Manager,不会再加载配置文件

	void SetThreadPool(ThreadPool *pool) {_th_pool = pool;}		//该Manager关联到某线程池, 该manager收到的协议，会在这个线程池中调用Process。
	ThreadPool *GetThreadPool() {return _th_pool;}

	inline IOMan * GetIOMan() const { return _ioman;}

	//修改某一个Session的输入或输出时的Security
	bool SetISecurity(SESSION_ID sid, Security::Type type, const Octets &key);
	bool SetOSecurity(SESSION_ID sid, Security::Type type, const Octets &key);

	static void EncodeProtocol(const Protocol *, Marshal::OctetsStream& os);		 //将协议编码至输出流中
	static Protocol *DecodeProtocol(const Marshal::OctetsStream& is, PSession *s); 		//从一段流中解码出协议, 会检查Session的状态
	Protocol *DecodeProtocol(const Marshal::OctetsStream& is) throw (Protocol::Exception); 	//从一段流中解码出协议

	void RawProcessProtocol(Protocol *, SESSION_ID sid);	//直接对Protocol调用Process（会捕捉异常）
	//向session发送一个协议
	bool Send(SESSION_ID id, const Protocol *protocol, bool urg = false);
	bool Send(SESSION_ID id, const Protocol &protocol, bool urg = false) { return Send(id,&protocol,urg); }
	bool SendFD(SESSION_ID id, int fd, const Octets & o);
	bool SendFD(SESSION_ID id, int fd , const Protocol &protocol);	// for test...

	bool RawSend(SESSION_ID id, const Octets& o, bool urg = false); 	//向Session发送一段原始数据，这个数据必须能够被正确的解码

	template<typename PDATA>
	static bool EncodeProtocolData(const PDATA & pdata, Marshal::OctetsStream& os)
	{
		//对协议数据进行编码，放入os字节流中, 格式为：
		//    [协议类型]  [协议长度] [数据]
		//新版，优化过的协议发送 可以少复制一次缓冲区
		//先压入最大的协议长度，然后最后再对协议长度进行修正
		//需要ProtocolData提供MaxSize()
		size_t size_policy = pdata.MaxSize();
		if(size_policy == 0) size_policy = 0x19999999;
		os << Marshal::Begin;
		os << CompactUINT(pdata.GetType());
		size_t offset = os.raw_size();
		os<< CompactUINT(size_policy);
		size_t offset2 = os.raw_size();
		pdata.marshal(os);
		size_t psize = os.raw_size() - offset2;
		if (psize <= size_policy)
		{
			os << SpecCompactUINT(offset, psize, size_policy);
			os << Marshal::Commit;
			if (PollIO::VerboseMode())
			{
				std::stringstream content;
				pdata.trace(content);
				PostEncode(pdata.GetType(), os.size(), pdata.GetName(),content.str().c_str());
			}
			return true;
		}
		os << Marshal::Rollback;
		Log::log(LOG_ERR,"FATAL, Protocol Size Excceed(type=%d,size=%d).", pdata.GetType(), psize);
		return false;
	}

	template<typename PDATA>
	bool SendProtocolData(SESSION_ID id, const PDATA * pdata, bool urg = false)
	{
		return SendProtocolData(id, *pdata, urg);
	}

	template<typename PDATA>
	bool SendProtocolData(SESSION_ID id, const PDATA & pdata, bool urg = false)
	{
		Marshal::OctetsStream os;
		if (EncodeProtocolData(pdata, os))
		{
			return RawSend(id, os, urg);
		}
		return false;
	}

#ifdef __OLD_IOLIB_COMPATIBLE__
	bool Close(SESSION_ID id, int status=CLOSE_ACTIVE, int delay_secs =0) { return CloseSession(id, status, delay_secs);}
	bool ChangeState(SESSION_ID id, SessionState *state, bool strict = true){return ChangeSessionState(id, state, strict);}
	void SetVerbose(SESSION_ID sid, int priority){ SetSessionLogPriority(sid, priority);}
#endif
	bool CloseSession(SESSION_ID id, int status=CLOSE_ACTIVE, int delay_secs = 0); 		//主动关闭一个session, delay_secs是指延迟多长时间关闭（单位为秒）
												//延迟期间，该Session将不接收任何协议，但可以发送协议,相当于单向关闭
												//若延迟期间链接断掉，则不正延迟，直接进入关闭状态。
	bool ChangeSessionState(SESSION_ID id, SessionState *state, bool strict = true); 	//改变一个session的State, 严格的strict检查意味着当收到不在state中定义的协议，会断Session。
	virtual void CheckSessionTimePolicy() {}				//检查所管理的Session是否超时, 现在IO库内部已经根据 IsStatePolicyEnable来决定是否自动检查Timeout，不必再手动调用它
	void __CheckSessionTimeout();						//检查所管理的Session是否超时，该接口只由IO库内部调用。
	void SetSessionLogPriority(SESSION_ID sid, int priority); 		//设置某个Session的Log优先级
	void SetSessionDelayMode(SESSION_ID id, bool nodelay);

	int DetachSession(SESSION_ID sid, FDTransferPData & data);		//将Session从管理器中脱离(会调用OnDelSession)，返回一个fd,设置FDTransferPData里除type和customdata外的所有成员 如有任何错误返回-1
	SESSION_ID AttachSession(int socket_fd);				//为某个FD创建对应的Session并且注册到Manager中(会调用OnAddSession)。
										//(如果该fd不是socket或已经被关闭，将注册失败,返回INVALID_SESSION_ID) 否则返回 Session ID
	SESSION_ID AttachSession(int fd, const FDTransferPData & data);		//为某个FD创建对应的Session，并且根据data恢复session原有的状态 返回值同上
	SESSION_ID AttachSession(const FDTransferPData & data);			//同上，会自动从FDStubManager中寻找匹配的FD
	SESSION_ID GetCurSessionID() const;					//取出当前的SessionID值，只供参考和判断情形使用，不要做关键逻辑
	

	virtual void OnAddSession(SESSION_ID) = 0; 				//当一个Session打开时
	virtual void OnAddSession(SESSION_ID sid, int fd); 			//带fd的打开操作，
	//当Session被关闭时
	virtual void OnDelSession(SESSION_ID) = 0;
	virtual void OnDelSession(SESSION_ID sid, int status) {  OnDelSession(sid); }

	//连接失败时, 用于客户端
	virtual void OnAbortSession(SESSION_ID) { }
	virtual void OnAbortSession(const SockAddr&) { }
	
	virtual void OnSetTransport(SESSION_ID sid, const SockAddr& local, const SockAddr& peer) { }
	virtual bool OnCheckAccumulate(size_t) const { return true; }
	
	virtual bool OnSend(SESSION_ID id, const Protocol *protocol) { return true; }
	virtual const SessionState *GetInitState() const = 0; 		//Session的初始state

	//收到未知的协议号
	virtual bool CheckUnknownProtocol(SESSION_ID , PROTOCOL_TYPE , unsigned int ) { return false; }
	virtual void DispatchUnknownProtocol(SESSION_ID , PROTOCOL_TYPE , const Marshal::OctetsStream & ) { }

	virtual void OnRecvSocket(PSession * session, Octets addr, int socket);	//从session关联的Unix Domain 连接接收到了一个socket，session层已经检测了socket的合法性，
										//这个函数的默认实现是调用了FDStubManager::Register，如果想自己定义实现，可以重载此函数
										//addr是这个socket的本地地址端口+外部地址端口，用于未来关联数据的使用
										//这个函数是在session加锁状态下，从PollIO线程调用的，因此不要使用发送数据和其他能够直接和间接使session加锁的功能

	virtual void OnSessionException(const Protocol::Exception& e)  //某个Session发生异常。e中包括了发生异常时的一些基本信息
	{ 
		e.Log(); 
		CloseSession (e.sid, CLOSE_EXCEPTION); 
	}
	virtual bool IsStatePolicyEnable() const {return true;}			//对接收到的协议"类型"及"状态"是否作检查,以及是否进行状态超时。
										//双方不信任时为true。较信任时可以返回false
										//为false时，如果收到和manager无关的未知协议，会触发“未知协议”的异常，依然可以断开。
	virtual bool IsTrustable() const {return false;}			//该Manager下所有的Session是否是可信的, 可信是指和本manager相连的那些session不会试图造假欺骗。
										//可信的话，有些内部的检查会放宽。一般情况下只有内部网络才会设置可信。
	virtual std::string Identification() const = 0;				//该Manager在Config文件中的标识
	virtual void OnCheckAddress(SockAddr &) const { } 			//参数为在InitServer或InitClient后，Manager所绑定或使用的IP地址端口

	virtual bool LoadSessionConfig(NetSession::Config &cnf) 		//加载Manager的配置，为ActiveIO和PassiveIO设置参数, 在InitXXX系列使用时调用。可以选择不从conf读中取
	{
		return cnf.Load(Identification().c_str());
	}
	virtual const ProtocolStubManager * GetStubManager() const; 		//返回所关联的存根管理器, 在兼容模式下，返回全局管理器。在新IO下，子类要重写这个函数。

public:
	//传送FD的函数，link是连接客户端的外部manager, tunnel是连接内部服务器的manager, tunnel_sid是对应UnixDomain的session，
	//		fd_trans_p_type是由用户定义的协议号，用户需要自己注册这个协议号并正确处理分发, customdata是用户自定义的数据，会发送到tunnel的另外一端，可以自行处理
	//		tunnel目的端会收到这个fd和协议 fd_trans_p_type(实际内容是一个FDTransferPData(FDTransferProtocol) 用户需要手动注册fd_trans_p_type和对应的处理类
	//		tunnel目的端会先收到fd，然后会以addr为key,fd 为value保存在FDStubManager中，然后收到协议FDTransferPData，这时应用data中的addr找到指定的fd，然后使用Attach功能关联到需要处理的Manager上。
	//		不过AttachSession时，会调用到Register ，进而立刻调用OnAddSession
	static bool TransferFD(PManager *link, SESSION_ID link_sid, PManager * tunnel, SESSION_ID tunnel_sid, PROTOCOL_TYPE fd_trans_p_type, const Octets & customdata);
	//在分发协议到线程池中之前，可以再次修改优先级、顺序号、组号，而不是以rpcalls.xml中的为准, 甚至所使用的线程池。
	//sequence和group为-1时，表示没有顺序号或组号。
	//prior为0时，会直接处理。 sequence和group都小于0时，会调用ThreadPool的PolicyAddTask
	//当group小于0时，group_seq无意义
	//注意：1.对于ProxyRpc而言，SendManager和RecvManager对于同一个协议的处理应该完全相同
	//	2.在协议的Process、Rpc的Client、Server和OnTimeout,以及Proxy的PostProcess,Delivery,OnTimeout处理函数中时，已经到了合适的线程池中
	virtual void TranslateProtocolAttr(const Protocol *p, SESSION_ID sid, GNET::ThreadPool* &thpool, int & prior, int & sequence, int & group, int & group_seq) {}
};

class ProtocolStubManager
{
	typedef abase::hash_map<PROTOCOL_TYPE, abase::pair<Protocol*, bool> > StubMap;
	StubMap _map;
public:
	ProtocolStubManager() {}
	~ProtocolStubManager();

	bool InsertStub(Protocol*);			//插入协议存根(由调用者释放存根)
	bool InsertStub(PROTOCOL_TYPE type, Protocol*);	//插入存根，可用在Protocol正在构造，尚不能调用GetType()时(由调用者释放存根)
	bool InsertStub2(PROTOCOL_TYPE type, Protocol*);	//插入存根，存根由StubManager来释放
	const Protocol *GetStub(PROTOCOL_TYPE type) const; 	//获取存根
	Protocol * CreateProtocol(PROTOCOL_TYPE type) const; 	//创建一个协议

	static ProtocolStubManager * GetGlobalStub();	//保持兼容性而提供的全局存根
};

class FDStubManager		//用于传输FD的临时维护数据结构，由于传过来的fd可能分布于多个Manager，因此这个FDStubManager是全局的
{
	static FDStubManager _manager;
	typedef std::pair<int,time_t> FD_ELEMENT;
	typedef std::map<Octets, FD_ELEMENT > MAP;
	MAP _fd_map;
	time_t _last_checktimeout;

	GNET::Mutex _locker;
public:
	enum
	{
		CHECK_INTERVAL = 10,	//每隔多少秒检查一次超时
		CHECK_TIMEOUT = 30,	//30秒超时时间，应该已经足够了
	};
	static FDStubManager & GetInstance() { return _manager;}

	FDStubManager():_last_checktimeout(0) {}

	void Register(Octets addr, int fd);				//保存一个fd
	int FindAndGet(Octets addr);					//取出一个fd
	void CheckTimeout(int timeout);					//检查超时	不定期调用，内部检查时间间隔，会控制每10秒左右真正检查一次

};

struct FDTransferPData  : public Protocol::Data
{
	PROTOCOL_TYPE type;
	Octets addr;
	Octets o_plainbuf;
	Octets o_outputbuf;
	Octets o_securebuf;
	Octets i_securebuf;
	Octets custom_buf;

public:
	FDTransferPData():type((PROTOCOL_TYPE)-1) {}	//默认是一个无效的type 
	FDTransferPData(PROTOCOL_TYPE type):type(type) {}

	PROTOCOL_TYPE GetType() const { return type;}
	GNET::Marshal::OctetsStream& marshal(GNET::Marshal::OctetsStream & os) const
	{
		os << addr << o_plainbuf << o_outputbuf << o_securebuf << i_securebuf << custom_buf;
		return os;
	}

	const GNET::Marshal::OctetsStream& unmarshal(const GNET::Marshal::OctetsStream &os)
	{
		os >> addr >> o_plainbuf >> o_outputbuf >> o_securebuf >> i_securebuf >> custom_buf;
		return os;
	}

	int PriorPolicy( ) const {return 110;}	//任意一个数字，如果想变更，重载FDTransferProtocol即可
	bool SizePolicy(size_t size) const {return true;}
	unsigned int MaxSize() const {return 0x19999999;}
		
	std::stringstream& trace(std::stringstream& os) const { os << "FDTransferPData:NULL"; return os;}
	const char *GetName() const { return "FDTransferPData";}
};

//FDTransferProtocol模板，如果需要传送FD，并在接收端处理，可以重载此类并手动完成注册操作
class FDTransferProtocol : public Protocol
{
protected:
	FDTransferPData  _data;
public:

	FDTransferProtocol(){}
	FDTransferProtocol(PROTOCOL_TYPE type, ProtocolStubManager *stubman):Protocol(type, stubman){ _data.type = type;}
	FDTransferProtocol(PROTOCOL_TYPE type):Protocol(type){ _data.type = type;}
	FDTransferProtocol(const FDTransferProtocol &rhs) : _data(rhs._data){}

	PROTOCOL_TYPE GetType() const {return _data.GetType();}
	OctetsStream& marshal(OctetsStream & os) const { return _data.marshal(os); }
	const OctetsStream& unmarshal(const OctetsStream &os) { return _data.unmarshal(os); }
	const char * GetName() const { return _data.GetName(); }
	std::string trace() const { std::stringstream os; return _data.trace(os).str();}
	int PriorPolicy() const { return _data.PriorPolicy();}
	bool SizePolicy(size_t size) const { return _data.SizePolicy(size); }
};

class TimerProtocol: public Protocol
{
protected:
	Timer _timer;
public:
	TimerProtocol(){}
	TimerProtocol(PROTOCOL_TYPE type, ProtocolStubManager *stubman):Protocol(type, stubman){}
	TimerProtocol(PROTOCOL_TYPE type):Protocol(type){}
	TimerProtocol(const TimerProtocol& rhs):Protocol(rhs){ _timer.Reset();}
	virtual ~TimerProtocol() {}
	virtual void OnTimeout() =0;
	virtual bool TimePolicy(int t) const {return t <5;}
	bool  CheckTime() const { return TimePolicy(_timer.Elapse());}
};

class ChannelProtocol : public Protocol         //通道协议, 该协议中包含了另一个子协议
{
public:
	ChannelProtocol() {}
	ChannelProtocol(PROTOCOL_TYPE type, ProtocolStubManager *stubman):Protocol(type,stubman){}
	ChannelProtocol(PROTOCOL_TYPE type):Protocol(type) {}

	typedef std::set<PROTOCOL_TYPE> PSET;
	virtual const PSET& GetProtocolSet() const = 0;		//该通道协议所能允许的子协议列表
	virtual Octets& GetChannelData() = 0;			//协议数据(子协议打包后的数据)，写成虚函数是希望和原IO库的rpcgen兼容，不把变量名定死

	void SetupChannelData(const Protocol &p) {SetupChannelData(&p);}        //填入子协议
	void SetupChannelData(const Protocol *p)
	{
		const PSET & Set = GetProtocolSet();
		ASSERT(Set.find(p->GetType()) != Set.end());

		Marshal::OctetsStream os;
		PManager::EncodeProtocol(p,os);
		GetChannelData() = os;
	}
	template<typename PDATA>
	void SetupChannelPData(const PDATA &pdata)		//以ProtocolData的方式填入子协议
	{
		Marshal::OctetsStream os;
		PManager::EncodeProtocolData(pdata, os);
		GetChannelData() = os;
	}
	bool CheckChannelData();				//检查子协议数据是否合法（协议类型是否在所允许的子协议列表中）,数据错误时返回false,打出一条log
	Protocol *DecodeChannelData(PManager *manager);		//根据通道协议数据，生成子协议对象，该子协议在使用完后必须Destroy，数据格式错误时会返回NULL,并打出一条log
};
}

#endif

