#include <string>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "ASSERT.h"
#include <strtok.h>
#include <timer.h>

#include <common/types.h>
#include <common/message.h>
#include "msgio.h"
#include "../world.h"

MsgReceiver *
MsgReceiver::Clone() const 
{ 
	return new MsgReceiver(*this); 
}

void 
MsgReceiver::OnOpen(NetIO *io)
{
	//printf("msgreceiver onOpen %p\n",this);
	_io = io;
}

void 
MsgReceiver::OnClose(const NetIO *io)

{
	//printf("msgreceiver OnClose %p\n",this);
	if(_peer_id >= 0) _manager->DelReceiver(_peer_id);
}

void 
MsgReceiver::OnAbort(const NetIO *io)
{
}

void 
MsgReceiver::OnRecv(NetIO *io, Octets & ibuf)
{
	size_t size = ibuf.size();
	size_t offset = 0;
	if(_peer_id == -1)
	{
		//只处理更改消息
		if(size < sizeof(MSG)) return ;
		{
			MSG * msg = (MSG*)ibuf.begin();
			if(msg->message == GM_MSG_IDENTIFICATION 
					&& msg->source.type == GM_TYPE_SERVER
					&& msg->content_length == sizeof(rect))
			{
				_peer_id = msg->source.id;
				_peer_tag = msg->param;
				offset += sizeof(MSG);
				offset += msg->content_length;

				msg->message = GM_MSG_IDENTIFICATION;
				msg->source = world_manager::GetServerID();
				msg->content_length = sizeof(rect);
				//msg->param = _manager->IsInstance()?1:0;
				msg->param = _manager->GetWorldTag();
				Octets oct;
				oct.insert(oct.end(),msg,sizeof(MSG));
				oct.insert(oct.end(),&(_manager->GetLocalRegion()), sizeof(rect));
				io->Send(oct);

				printf("Add receiver %d\n",_peer_id);
				_manager->AddReceiver(_peer_id);
			}
			else
			{
				printf("游戏服务器接受到不符合的身份消息1\n");
				io->Close();
				return;
			}
		}
	}

	while(offset <= size - sizeof(MSG))
	{
		MSG * msg = (MSG*)((char *)ibuf.begin() + offset);
		if(offset + sizeof(MSG) + msg->content_length > size) break;

		//暂时限制了一个消息不能过大 目前不会超过65536
		ASSERT(msg->content_length >= 0 && msg->content_length < 65536);
		msg->content = ((char *)ibuf.begin()) + offset + sizeof(MSG);
		_manager->ReceiveMessage<0>(_peer_tag,*msg);
		offset +=  sizeof(MSG) + msg->content_length;
	}
	ibuf.erase(ibuf.begin(), (char*)ibuf.begin() + offset);
}
void 
MsgReceiver::Destroy() 
{ 
	delete this; 
}



MsgDispatcher *
MsgDispatcher::Clone() const 
{
	//printf("dispatcher clone\n");
	return new MsgDispatcher(*this); 
}

void
MsgDispatcher::OnOpen(NetIO *io)
{
	MSG msg;
	memset(&msg,0,sizeof(msg));
	msg.message = GM_MSG_IDENTIFICATION;
	msg.source = world_manager::GetServerID();
	msg.param = _manager->GetWorldTag();
	msg.content_length = sizeof(rect);
	Octets oct;
	oct.insert(oct.end(),&msg,sizeof(MSG));
	oct.insert(oct.end(),&(_manager->GetLocalRegion()), sizeof(rect));
	io->Send(oct);
	_io = io;
}

void 
MsgDispatcher::OnRecv(NetIO *io, Octets &ibuf)
{
	if(_peer_id == -1)
	{
		size_t size = ibuf.size();
		if(size < sizeof(MSG)) return ;
		{
			MSG * msg = (MSG*)ibuf.begin();
			if(msg->message == GM_MSG_IDENTIFICATION 
					&& msg->source.type == GM_TYPE_SERVER
					&& msg->content_length == sizeof(rect))
			{
				rect * pRect = (rect*)(((char*)msg) + sizeof(MSG));
				_peer_region = *pRect;
				_peer_id = msg->source.id;
				_peer_tag = msg->param;
				ibuf.clear();
				printf("Add dispatcher %d\n",_peer_id);
				_manager->AddDispatcher(_peer_id,this,msg->param);
			}
			else
			{
				printf("游戏服务器接受到不符合的身份消息2\n");
				io->Close();
				return;
			}
		}
	}
	else
	{
		ASSERT(false && "不应该再次收到消息了");
	}
}

int
MsgDispatcher::Send(const MSG & msg)
{	
	//$$$这样发送的效率稍微有点低，因为多拷贝了一次数据
	Octets oct;
	oct.insert(oct.end(),&msg,sizeof(MSG));
	if(msg.content_length) oct.insert(oct.end(),msg.content,msg.content_length);
	if(!_io->Send(oct)) return -1;
	return 0;
}

void 
MsgDispatcher::Destroy() 
{
	//printf("dispatcher destroy\n");
	delete this; 
}

void
MsgDispatcher::OnClose(const NetIO *io)
{
	//printf("msgdispatcher OnClose %p %d\n",this,_peer_id);
	if(_peer_id >= 0) _manager->DelDispatcher(_peer_id);
	OnAbort(io);
}

namespace
{
	class reconnect: public abase::timer_task , public ONET::Thread::Runnable
	{	
		std::string _name;
		void * _addr;
		size_t _addr_len;
		MsgIOManager * _man;
		virtual void OnTimer(int index,int rtimes)
		{
			ONET::Thread::Pool::AddTask(this);
		}
		virtual void Run()
		{
			ONET::ActiveIO::Open(MsgDispatcher(_name,_man,_addr,_addr_len),(const sockaddr*)_addr,_addr_len);
			delete this;
		}

	protected:
		~reconnect()
		{
			if(_addr) free(_addr);
		}
	public:
		reconnect(const void * addr,size_t len, std::string name,MsgIOManager * man)
		{
			_addr = malloc(len);
			_addr_len = len;
			memcpy(_addr,addr,len);
			_name = name;
			_man = man;
		}

		void Start()
		{
			int delay= rand() % 10 + 15;
			int rst = SetTimer(g_timer,delay*20,1,delay*20);
			//printf("OnAbort/OnClose at Dispatcher, reconnect after %d\n",delay);
			ASSERT(rst >= 0);
		}
	};
}

void 
MsgDispatcher::OnAbort(const NetIO *io)
{
	reconnect * pConnect = new reconnect(_addr,_addr_len,_id,_manager);
	pConnect->Start();
}

/* ----------------- MsgIOManager -----------------------*/
static unsigned long get_S_addr(const char *__szName)
{
	unsigned long	rst;
	rst = inet_addr(__szName);
	if(rst == INADDR_NONE)
	{
		struct hostent *lpHostEntry;
		lpHostEntry = gethostbyname(__szName);
		if (lpHostEntry == NULL)
		{
			rst = 0;
		}
		else
		{
			rst = (*((struct in_addr *)*lpHostEntry->h_addr_list)).s_addr;
		}
	}
	return	rst;
}

static bool MakeIPAddr(sockaddr_in & addr,const char * ipaddr)
{       
	memset(&addr,0,sizeof(addr));
	char buf[256];
	memset(buf,0,sizeof(buf));
	strncpy(buf,ipaddr,sizeof(buf) - 1);
	char * sp = strchr(buf,':');
	if(sp == NULL) return false;
	*sp++ = 0;
	
	//试图寻找存在的假名
	Conf *conf = Conf::GetInstance();
	std::string str = conf->find("AddrAlias",buf);
	if(str.empty()) str = std::string(buf);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(sp));
	addr.sin_addr.s_addr = get_S_addr(str.c_str());
	return true;
}

static bool MakeUnixAddr(sockaddr_un & addr,const char * unixaddr)
{       
	memset(&addr,0,sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, unixaddr, sizeof(addr.sun_path) - 1);
	return true;
}

#define MAX_RUNNER_NUM (MAX_GS_NUM + MAX_CS_NUM)
MsgIOManager::MsgIOManager(world_manager * planes_man):_planes_man(planes_man),
						 msg_runner_map(MAX_GS_NUM,NULL),
						 msg_receiver_map(MAX_GS_NUM,false),
						 msg_runner_lock(MAX_GS_NUM,pthread_rwlock_t()) 
{
	for(size_t i = 0; i < msg_runner_lock.size(); i ++)
	{
		pthread_rwlock_init(msg_runner_lock.begin() + i,NULL);
	}
	pthread_rwlock_init(&_list_lock,NULL);
}

bool
MsgIOManager::InitClient(const sockaddr_in &my_addr, const char * servername,const char * servers, int is_instance,rect & inner_region)
{
	Conf *conf = Conf::GetInstance();
	abase::strtok tok(servers,";,\r\n");
	const char * token;
	while((token = tok.token()))
	{
		if(!*token) continue;
		//printf("parse server '%s'\n",token);
		if(strcmp(token, servername) == 0) continue;

		std::string osvr;
		if(is_instance)
			osvr = "Instance_";
		else
			osvr = "World_";
		osvr += token;

		rect rt(0,0,0,0);
		sscanf(conf->find(osvr.c_str(),"local_region").c_str(),"{%f,%f} , {%f,%f}",&rt.left,&rt.top,&rt.right,&rt.bottom);
		if(rt.top >= rt.bottom || rt.left >= rt.right) return false;
		int tag = atoi(conf->find(osvr.c_str(),"tag").c_str());
	//	if(!is_instance) inner_region.Cut(rt);  不再使用是否副本来判定切割，而是使用world tag
		if(_world_tag == tag) inner_region.Cut(rt); 
		
		sockaddr_in in_addr;
		std::string str = "MsgReceiverTCP_";
		str += token;
		if(!MakeIPAddr(in_addr,conf->find(str.c_str(),"listen_addr").c_str()))
		{
			printf("无法对%s的listen地址进行解析\n",str.c_str());
			return false;
		}

		if(in_addr.sin_addr.s_addr != my_addr.sin_addr.s_addr)
		{
			printf("连接远程游戏服务器:'%s'，使用TCP方式:%x\n",token,in_addr.sin_addr.s_addr);
			ONET::ActiveIO::Open(MsgDispatcher("MsgTCPSession",this,&in_addr,sizeof(in_addr)),
						(sockaddr *)&in_addr,sizeof(in_addr));
			continue;
		}

		//是同一台机器使用UNIX连接
		str = "MsgReceiverUNIX_";
		str += token;
		sockaddr_un un_addr;
		MakeUnixAddr(un_addr,str.c_str());
		if(!MakeUnixAddr(un_addr,conf->find(str.c_str(),"listen_addr").c_str()))
		{
			printf("无法对%s的listen地址进行解析\n",str.c_str());
			return false;
		}
		printf("连接远程游戏服务器:'%s'，使用UNIX方式:%s\n",token,un_addr.sun_path);
		ONET::ActiveIO::Open(MsgDispatcher("MsgUNIXSession",this,&un_addr,sizeof(un_addr)),
					(sockaddr *)&un_addr,sizeof(un_addr));
	}
	return true;
}

bool
MsgIOManager::Init(const char * name,const rect & local_region,rect & inner_region)
{
	_world_tag = world_manager::GetWorldTag();

	_local_region = local_region;
	inner_region = local_region;

	std::string str = "MsgReceiverTCP_";
	str += name;
	if(ONET::PassiveIO::Open(MsgReceiver(str.c_str(),this)) == NULL)
	{
		printf("Create MsgReceiver failed,address:%s\n",str.c_str());
		return false;
	}

	Conf *conf = Conf::GetInstance();
	sockaddr_in my_addr;
	if(!MakeIPAddr(my_addr,conf->find(str.c_str(),"listen_addr").c_str() ))
	{
		printf("自己的listen地址不正确\n");
		return false;
	}
	
	str = "MsgReceiverUNIX_";
	str += name;
	if(ONET::PassiveIO::Open(MsgReceiver(str.c_str(),this)) == NULL)
	{
		printf("Create UNIX MsgReceiver failed\n");
		return false;
	}

	Conf::section_type section = "General";
	std::string servers = conf->find(section,"world_servers").c_str();
	if(!InitClient(my_addr,name,servers.c_str(),0,inner_region)) return false;
	servers = conf->find(section,"instance_servers").c_str();
	if(!InitClient(my_addr,name,servers.c_str(),1,inner_region)) return false;
	return true;
}


int MsgIOManager::GetWorldTag()
{
	return _world_tag;
}


void MsgIOManager::AddGSNode(int id, const rect & region, int world_tag)
{
	//第一步，加入到统一的列表中
	size_t i;
	for(i =0 ; i < _msg_gs_list.size(); i ++)
	{
		if(_msg_gs_list[i].id == id)
		{
			_msg_gs_list[i].tag = world_tag;
			_msg_gs_list[i].region =region;
			break;
		}
	}
	if(i == _msg_gs_list.size())
	{
		_msg_gs_list.push_back(node_t(id, region,world_tag));
	}

	ASSERT(_world_tag == world_manager::GetWorldTag());
	if(_world_tag == world_tag)
	{
		rect rt = region;
		rt.top -=100.f;                 //$$$$$$ 这个距离是临时的
		rt.bottom +=100.f;
		rt.left -=100.f;
		rt.right +=100.f;
		bool isoverlap = rt.IsOverlap(_local_region);
		for(i = 0; i < _msg_near_list.size(); i ++)
		{
			if(_msg_near_list[i].id == id)
			{
				if(isoverlap)
				{
					_msg_near_list[i].region = region;
					_msg_near_list[i].tag = world_tag;
				}
				else
				{
					_msg_near_list.erase(_msg_near_list.begin() + i);
				}
				isoverlap = false;
				break;
			}

		}
		if(isoverlap) _msg_near_list.push_back(node_t(id,region,world_tag));
	}
	else
	{
		//tag 不同，表示是不同的位面，需要进行放入其他的空间中
		for(i = 0; i < _msg_other_list.size(); i ++)
		{
			if(_msg_other_list[i].id == id)
			{
				_msg_other_list[i].region = region;
				_msg_other_list[i].tag = world_tag;
				break;
			}
		}
		if(i == _msg_other_list.size())
		{
			_msg_other_list.push_back(node_t(id,region,world_tag));
		}
	}
}

void  MsgIOManager::SendMessage(int id, const MSG & msg)
{
	rdlock_scoped alock(msg_runner_lock[id]);
	if(MsgDispatcher * pRunner = msg_runner_map[id])
	{
		if(pRunner->Send(msg) != 0)
		{
			GLog::log(GLOG_ERR, "Send remote message failed. worldtag:%d:destgsid:%d:message:%d:source:%d:%d:target:%d:%d:length:%u",
							_world_tag, id, msg.message, msg.source.type, msg.source.id, msg.target.type, msg.target.id, sizeof(MSG)+msg.content_length);
		}			
	}
}

