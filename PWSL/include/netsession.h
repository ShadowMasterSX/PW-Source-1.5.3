/*
 *
 *		NetSession类， 这个类维护了一个连接的状态，发送和接受的数据，加密和其他的一些内容
 *		作者：未知
 *		修改：2009-06 崔铭 杨延昭  修改了很多逻辑和结构
 *		公司：完美时空
 *
 */

#include "netio.h"
#include "mutex.h"
#include "security.h"
#include "conf.h"
#include <vector>
#include <stdio.h>

#ifndef __ONLINEGAME_NETSESSION_H__
#define __ONLINEGAME_NETSESSION_H__

namespace GNET
{

class Octets;
class Security;
class NetSession
{
	friend class NetIO;
	friend class StreamIO;
	friend class DgramServerIO;
	friend class DgramClientIO;
	friend class UnixIO;
	enum { DEFAULTIOBUF = 8192 };
protected:
	PollIO  *_assoc_io;			//本session 所关联的IO对象
	bool	_sending;			//是否有数据等待发送
	bool    _closing;			//是否正在等待关闭, 可能会等到发送缓冲全为空时才关闭
	bool    _close_discard;			//是否忽略关闭事件， 这个是由配置文件设置的 用途尚不明朗
						//大略的作用是当为false时，关闭POLLIO的动作将延迟到发送缓冲为空以后。
	int     _status;				//断线状态和errno
	size_t	_output_wait_size;		//有多少数据等待发送，由PSession每次调用Send后进行更新 ，因此不是完全准确的，用于延迟发送的一个参考值 

	//下边这两个缓冲区（读和写），由于设计上是只有一个POLLIO线程在操作，所以可以不加锁
	//这两个缓冲区是密文存放的
	Octets	_ibuffer;			//从连接读出来的最原始数据存放的缓冲区 
	Octets	_obuffer;			//要发送出去的最终数据缓冲区
	size_t  _obuffer_offset;		//_obuffer中有效数据的起始, 之前的是已经发出的数据  目前只有StreamIO使用
	size_t  _ibuffer_maxsize;		//从网络上读取时, _ibuffer的最大长度。而不是以前的_ibuffer.capacity()
	size_t  _obuffer_maxsize;
	size_t	_buffer_threshold;		//解释见下面的Config里的bufferthreshold的注释

	//解密只发生在接收到数据后，加密只发生在将要发出数据前，且都只由POLLIO线程来操作，所以操作这两个不加锁
	//这两个缓冲区是明文存放的
	Octets  _iplainbuf;			//对_ibuffer解密后得到的输入数据，可以用于解码了
	Octets  _oplainbuf;			//要发送的原始数据，经过加密后会放入_obuffer中

	Security *_decrypt;			//输入端使用的加密模块，用于解密进入的数据
	Security *_encrypt;			//发送端使用的加密模块，用于加密放出的数据

	std::vector<int> _send_fd_list;		//发送fd的缓冲区，如果有fd，会尽量先发送fd的
	int _cur_send_fd;			//对于支持发送FD的session,这个保存当前发送的fd

	Octets& GetOBuffer();	//获取输出缓冲，在获取的同时会试图加密。  StreamIO已经不再调用这个函数了
	bool UpdateOutBuffer();	//更新输出缓冲，会进行加密。如果没有加到密，则返回false.  只有StreamIO使用这个函数, 它会用这个返回值来做一些优化

	inline Octets& GetIBuffer() { return _ibuffer; }	//获取输入缓冲(尚未解密的)

	inline	bool SendFinish()	//用于POLLIO发送成功后. 调用此函数时，该NetSession是锁定的 返回值代表是否没有数据了
	{
		if ( NoMoreDataForSend() )
		{
			_sending = false;
			_assoc_io->ForbidSend();
			return true;
		}
		return false;
	}

	void _Close()	//关闭。NetSession内部使用，不直接使用
	{
		if ( _closing ) return;
		_closing = true;
		if (_close_discard)	//无论缓冲区有无消息，都断
		{
			_assoc_io->Close();
			return;
		}
		//试图发送一次消息 在没有屏蔽SIGPIPE时候可能会Broke pipe
		OnSend();
		if(_obuffer.empty()) UpdateOutBuffer();
		int send_bytes = _obuffer.size() - _obuffer_offset;
		if (send_bytes)
		{
			send_bytes = write(_assoc_io->GetFD(), (char*) _obuffer.begin() + _obuffer_offset, send_bytes);
			if (send_bytes >0)
			{
				_obuffer_offset += send_bytes;
				if ( _obuffer.size() == _obuffer_offset) { _obuffer.clear(); _obuffer_offset = 0;}
			}
		}

		//关闭可能存在的待传输send_fd_list(仅用于UNIXIO)
		if(!_send_fd_list.empty()) std::for_each(_send_fd_list.begin(), _send_fd_list.end(), ::close);
		_send_fd_list.clear();
		if(_cur_send_fd != -1) close(_cur_send_fd);
		_cur_send_fd = -1;
		//无论发送成功与否，都把链接断掉
		_assoc_io->Close();
		return;
	}

protected:
	GNET::Mutex _locker;
	virtual ~NetSession ()
	{
		_decrypt->Destroy();
		_encrypt->Destroy();
	}
	NetSession(const NetSession &rhs) : _assoc_io(rhs._assoc_io), _sending(false), _closing(false), _close_discard(rhs._close_discard),_status(0),
		_obuffer_offset(0),_ibuffer_maxsize(rhs._ibuffer_maxsize),_obuffer_maxsize(rhs._obuffer_maxsize),_buffer_threshold(rhs._buffer_threshold),
		_decrypt(rhs._decrypt->Clone()), _encrypt(rhs._encrypt->Clone()), _locker()
	{
		_output_wait_size = 0;
		_ibuffer.reserve(_ibuffer_maxsize);
		_obuffer.reserve(_obuffer_maxsize);
		_cur_send_fd = -1;
	}

	inline bool Output(const Octets &data)  //将一段数据压入以发送
	{
		_oplainbuf.insert(_oplainbuf.end(), data.begin(), data.end());
		return true;
	}

	inline void ClearBufByThreshold(Octets& data)
	{
		if(_buffer_threshold && data.capacity() > _buffer_threshold)
		{
			Octets clear_buf;
			clear_buf.swap(data);
		}
		else
		{
			data.clear();
		}
	}

	Octets& Input();		  //获得输入缓冲，已经解过密的

	inline void SendReady()	//当用户向os缓冲区加入数据后，用此来触发发送操作,调用此函数时，该NetSession必须处于上锁状态
	{
		if ( _sending ) return;
		_sending = true;
		_assoc_io->PermitSend();
	}
public:
	void SetISecurity(Security *s);	//设置输入加密模块, 内部会加锁,  s由Session来释放
	void SetOSecurity(Security *s);	//设置输出加密模块, 内部会加锁,  s由Session来释放

	struct Config
	{
		//NetSession所需要的参数
		size_t ibuffermax;		//接受缓冲大小
		size_t obuffermax;		//发送缓冲大小
		size_t bufferthreshold;		//内部两个发送缓冲区的阈值，超过这个值，在加密和发送完成后，这两个缓冲区的内存将被释放（由于缓冲至少要能容纳下一个协议，所以会占用到预计之外的内存）
						//这个值为0则不释放空间 建议使用obuffermanx的2倍~3倍大小。
		Octets decrypt_key;
		Octets encrypt_key;
		unsigned char decrypt_type;	//收到对端消息后的解密方式
		unsigned char encrypt_type;	//发送消息到对端时使用的加密方式
		bool close_discard;
		bool latency_mode;		//0 在poll的时候才发送数据，高吞吐模式，write系统调用的次数较少     1 尽可能立刻发送，低延迟模式 同时加密和压缩会尽可能这时完成

		//NetIO所需要的参数, 通常情况下只需修改port和address
		short socket_type;	//0--tcp, 1--udp, 2--unix 3--unix(support fd transfer)
		unsigned short port;	//监听或连接的端口 for tcp & udp
		std::string address;	//client时，是对端的IP地址（非域名），server时，是需要绑定的地址
		int so_sndbuf;		//为0时，使用系统默认的值 
		int so_rcvbuf;		//为0时，使用系统默认的值 
		int nodelay;		//TCP_NODELAY
		int broadcast;
		int listen_backlog;	//server方式时，listen时所带的参数

		Config(): ibuffermax(DEFAULTIOBUF), obuffermax(DEFAULTIOBUF), bufferthreshold(0), decrypt_type(0), encrypt_type(0), close_discard(false),latency_mode(0),
			socket_type(0), port(0), so_sndbuf(0), so_rcvbuf(0), nodelay(1), broadcast(0), listen_backlog(0)
		{
		}

		//从Conf文件中加载。这是标准的模式，和以前老share库使用的关键字是相同的
		bool Load(const char *identification, Conf *pconf = NULL)
		{
			Conf *conf = pconf ? pconf: Conf::GetInstance();

			Conf::section_type section = identification;
			Conf::value_type type = conf->find(section, "type");
			if(!strcasecmp(type.c_str(), "tcp")) socket_type = 0; else
			if(!strcasecmp(type.c_str(), "udp")) socket_type = 1; else
			if(!strcasecmp(type.c_str(), "unix")) socket_type = 2; else 
			if(!strcasecmp(type.c_str(), "unixio")) socket_type = 3; else return false;

			port = atoi(conf->find(section, "port").c_str());
			address =  conf->find(section, "address").c_str();
			so_sndbuf = atoi(conf->find(section, "so_sndbuf").c_str());
			so_rcvbuf = atoi(conf->find(section, "so_rcvbuf").c_str());
			nodelay = atoi(conf->find(section, "tcp_nodelay").c_str());
			broadcast = atoi(conf->find(section, "so_broadcast").c_str());
			listen_backlog = atoi(conf->find(section, "listen_backlog").c_str());

			ibuffermax = atoi(conf->find(section, "ibuffermax").c_str());
			obuffermax = atoi(conf->find(section, "obuffermax").c_str());
			bufferthreshold = atoi(conf->find(section, "bufferthreshold").c_str());
			if(int tmp = atoi(conf->find(section, "isec").c_str()))
			{
				Conf::value_type key = conf->find(section, "iseckey");
				decrypt_key = Octets(&key[0], key.size());
				decrypt_type = tmp;
			}

			if(int tmp = atoi(conf->find(section, "osec").c_str()))
			{
				Conf::value_type key = conf->find(section, "oseckey");
				encrypt_key = Octets(&key[0], key.size());
				encrypt_type = tmp;
			}

			close_discard = atoi(conf->find(section, "close_discard").c_str());
			latency_mode = atoi(conf->find(section, "send_no_latency").c_str());
			return true;
		}
	};

	//获得配置的参数, 派生类可以从文件中读取,然后填入cnf中, 默认实现方式，应该是从Manager中拷一份
	virtual void GetConfig(Config &cnf)const = 0;

	virtual void Init()
	{
		Config cnf;
		GetConfig(cnf);

		if (cnf.ibuffermax) { _ibuffer.reserve(cnf.ibuffermax); _ibuffer_maxsize = cnf.ibuffermax;}
		if (cnf.obuffermax) { _obuffer.reserve(cnf.obuffermax); _obuffer_maxsize = cnf.obuffermax;}
		_close_discard = cnf.close_discard;
		_buffer_threshold = cnf.bufferthreshold;
		if (cnf.decrypt_type)
		{
			Security * sec = Security::Create(cnf.decrypt_type);
			sec->SetParameter(cnf.decrypt_key);
			SetISecurity(sec);
		}
		if (cnf.encrypt_type)
		{
			Security * sec = Security::Create(cnf.encrypt_type);
			sec->SetParameter(cnf.encrypt_key);
			SetOSecurity(sec);
		}
	}

	void SetDelayMode(bool nodelay)
	{
		if(_assoc_io) _assoc_io->SetDelayMode(nodelay);
	}

	NetSession() : _assoc_io(NULL), _sending(false), _closing(false), _close_discard(false),_status(0),
		_obuffer_offset(0), _ibuffer_maxsize(DEFAULTIOBUF),_obuffer_maxsize(DEFAULTIOBUF), _buffer_threshold(0),
		_decrypt(new NullSecurity), _encrypt(new NullSecurity),_locker()
		{ 
			_ibuffer.reserve(_ibuffer_maxsize);
			_obuffer.reserve(_obuffer_maxsize);
		}


	void Close( int code=0, bool locked=false);  //关闭一个session. code为错误码。如果已经锁定，则调用时locked须传入true
	void ErrorClose(int code);		     //关闭。　调用该函数时session必须处于上锁状态
	int GetStatus() {  return _status; }
private:
	virtual bool NoMoreDataForSend() const = 0;		//给发送模块一个提示，是否还有数据需要发送, 若没有将会关闭发送，当下一个SendReady被调用时才会唤醒
	virtual void OnRecv() = 0;				//收到输入消息。在这个函数里可以对输入消息进行解码、处理。必须一次性处理完。如果有遗留的消息未处理，
								//只有当下一次有输入消息事件，才有机会再次处理。
	virtual void OnRecvFD(int fd) = 0;			//对于支持传输fd的UnixIO，当收到fd时，会调用这个函数
	virtual void OnSend() = 0;				//socket已经可写，准备发送，此时应该调用Output将所需要发送的数据压入
	virtual void OnUnixSend() = 0;				//支持UnixIO，并发送fd的特殊的函数，只有UnixIO会调用它，其他情况都会调用OnSend
	virtual void OnOpen(const SockAddr& local, const SockAddr& peer) { }
	virtual void OnOpen() { }
	virtual void OnClose() { }
public:
	virtual std::string Identification () const = 0;	//该session的标识, 为保持兼容性，目前的实现与Manager的Identification一致
	virtual int IntIdentification() const {return -1;}	//使用Int的标识,仅仅用于调试输出
	virtual void OnAbort(const SockAddr& sa) { }		//连接失败,当客户端连向服务器失败时的处理
	virtual	NetSession* Clone() const = 0;
	virtual void Destroy() { delete this; }
	virtual void OnCheckAddress(SockAddr &) const { }
protected:
	int Detach(Octets & o_plainbuf, Octets & o_outputbuf, Octets & o_securebuf, Octets & i_securebuf);	//内部使用
	int Attach(const Octets & o_plainbuf, const Octets & o_outputbuf, const Octets & o_securebuf, const Octets & i_securebuf);	//内部使用

};

}

#endif

