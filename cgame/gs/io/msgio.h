
#ifndef __ONLINEGAME_GS_MSGIO_H__
#define __ONLINEGAME_GS_MSGIO_H__

#include <sys/types.h>
#include <sys/socket.h>

#include <verbose.h>
#include <io/passiveio.h>
#include <io/activeio.h>
#include <amemobj.h>

using namespace ONET;
class world_manager;
class MsgIOManager;
struct MSG;
class MsgBase : public abase::ASmallObject
{
protected:
	MsgIOManager *_manager;
	int _peer_id;
	int _peer_tag;
	NetIO * _io;
	sockaddr * _addr;
	socklen_t  _addr_len;
	MsgBase(MsgIOManager * man):_manager(man),_peer_id(-1),_io(NULL),_addr(NULL),_addr_len(0){}
	MsgBase(const MsgBase & rhs)
		:_manager(rhs._manager),_peer_id(rhs._peer_id),_io(rhs._io),_addr(NULL),_addr_len(0)
	{
		SetAddr(rhs._addr,rhs._addr_len);
	}
public:

	~MsgBase()
	{
		Clear();
	}
	void SetAddr(const void * addr, socklen_t size)
	{
		Clear();
		if(addr)
		{
			_addr = (sockaddr*) malloc(size);
			memcpy(_addr,addr,size);
			_addr_len = size;
		}
	}
	MsgIOManager * GetMan() { return _manager;}
	void Clear()
	{
		if(_addr) free(_addr);
		_addr = NULL;
		_addr_len = 0;
	}
};

class MsgReceiver : public NetSession ,public MsgBase
{
	std::string _id;
	std::string Identification() const { return _id;}
        MsgReceiver *Clone() const;
	virtual void OnOpen(NetIO *io); 
	virtual void OnClose(const NetIO *io);
	virtual void OnRecv(NetIO *io, Octets &ibuf);
	virtual void OnAbort(const NetIO *io);
	virtual void Destroy();

public:
	MsgReceiver(std::string name,MsgIOManager * man):MsgBase(man),_id(name){}

};

class MsgDispatcher : public NetSession, public MsgBase
{
	rect _peer_region;
	std::string _id;
	std::string Identification() const { return _id;}
	virtual void OnOpen(NetIO *io);
	virtual void OnClose(const NetIO *io);
	virtual void OnRecv(NetIO *io, Octets &ibuf);
	virtual void OnAbort(const NetIO *io);
        MsgDispatcher *Clone() const;
	virtual void Destroy();

public:
	MsgDispatcher(std::string name,MsgIOManager * man,const void * addr,socklen_t len)
		:MsgBase(man),_id(name)
		{
			SetAddr(addr,len);
		}
	int Send(const MSG & msg);
	const rect & GetRegion()
	{
		return _peer_region;
	}
	
};

class MsgIOManager
{
	struct node_t
	{	
		node_t(){}
		node_t(int id, const rect & region,int tag):id(id),region(region),tag(tag){}
		int id;
		rect region;
		int tag;
	};
	world_manager * _planes_man;
	rect _local_region;
	abase::vector<MsgDispatcher*> msg_runner_map;       
	abase::vector<bool> msg_receiver_map;               
	abase::vector<pthread_rwlock_t> msg_runner_lock;    
	abase::vector<node_t> _msg_gs_list;		//所有与自己相联结的服务器表
	abase::vector<node_t> _msg_near_list;		//与自己接壤的服务器表
	abase::vector<node_t> _msg_other_list;		//所有的tag不同的服务器表
	int _world_tag;
	mutable pthread_rwlock_t _list_lock;		//对应的锁
private:
	void AddGSNode(int id, const rect & region, int world_tag);
/*
	void AddGSNode(int id, const rect & region)
	{
		size_t i;
		for(i =0 ; i < _msg_gs_list.size(); i ++)
		{
			if(_msg_gs_list[i].id == id) 
			{
				_msg_gs_list[i].region =region;
				break;
			}
		}
		if(i == _msg_gs_list.size())
		{
			_msg_gs_list.push_back(node_t(id, region));
		}

		rect rt = region;
		rt.top -=100.f;			//$$$$$$ 这个距离是临时的
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
				}
				else
				{
					_msg_near_list.erase(_msg_near_list.begin() + i);
				}
				return ;
			}

		}
		if(isoverlap) _msg_near_list.push_back(node_t(id,region));
	}
	void AddInstanceGSNode(int id, const rect & region)
	{
		size_t i;
		for(i =0 ; i < _msg_gs_list.size(); i ++)
		{
			if(_msg_gs_list[i].id == id) 
			{
				_msg_gs_list[i].region =region;
				_msg_gs_list[i].instance = true;
				break;
			}
		}
		if(i == _msg_gs_list.size())
		{
			_msg_gs_list.push_back(node_t(id, region,true));
		}

		for(i =0 ; i < _msg_instance_list.size(); i ++)
		{
			if(_msg_instance_list[i].id == id) 
			{
				_msg_instance_list[i].region =region;
				break;
			}
		}
		if(i == _msg_instance_list.size())
		{
			_msg_instance_list.push_back(node_t(id, region,true));
		}
	}
	*/

	void DelGSNode(int id)
	{
		size_t i;
		for(i = 0; i < _msg_gs_list.size();i ++)
		{
			if(_msg_gs_list[i].id == id)
			{
				_msg_gs_list.erase(_msg_gs_list.begin() + i);
				if(i) i--;
				break;
			}
		}
		ASSERT(i ==0 || _msg_gs_list.size() > i);
		for(i = 0; i < _msg_near_list.size();i ++)
		{
			if(_msg_near_list[i].id == id)
			{
				_msg_near_list.erase(_msg_near_list.begin() + i);
				break;
			}
		}

		for(i = 0; i < _msg_other_list.size();i ++)
		{
			if(_msg_other_list[i].id == id)
			{
				_msg_other_list.erase(_msg_other_list.begin() + i);
				break;
			}
		}
		
	}

	bool InitClient(const sockaddr_in &my_addr, const char * servername,const char * str, int is_instance,rect & inner_region);

public:
	explicit MsgIOManager(world_manager * planes = NULL);
	void SetPlane(world_manager * planes) { _planes_man = planes;}
	bool Init(const char * servername,const rect &local_region,rect & inner_region);
	const rect &  GetLocalRegion() const 
	{ 
		return _local_region;
	}
public:
	//bool IsInstance() {return _is_instance;}
	int GetWorldTag();
	void AddReceiver(int id)
	{
		ASSERT(id >=0);
		msg_receiver_map[id] = true;
	}

	void DelReceiver(int id)
	{
		ASSERT(id >=0);
		msg_receiver_map[id] = false;
	}
	
	void AddDispatcher(int id, MsgDispatcher * runner,int world_tag)
	{
		if(id >= 0 && id < (int)msg_runner_map.size())
		{
			wrlock_scoped alock1(_list_lock);
			wrlock_scoped alock2(msg_runner_lock[id]);
			MsgDispatcher * old_runner = msg_runner_map[id];
			msg_runner_map[id] = runner;
			delete old_runner;
			AddGSNode(id, runner->GetRegion(), world_tag);
	/*		if(is_instance) 
				AddInstanceGSNode(id, runner->GetRegion());
			else
				AddGSNode(id, runner->GetRegion());*/
		}
		else
		{
			ASSERT(false);
		}
	}

	void DelDispatcher(int id)
	{
		if(id >= 0 && id < (int)msg_runner_map.size())
		{
			wrlock_scoped alock1(_list_lock);
			wrlock_scoped alock2(msg_runner_lock[id]);
			msg_runner_map[id] = NULL;
			DelGSNode(id);
		}
		else
		{
			printf("array overflow%d\n",id);
			ASSERT(false);
		}
	}
	
	template<int>
	void ReceiveMessage(int msg_tag, const MSG & msg)
	{
		_planes_man->ReceiveMessage(msg_tag,msg);
	}
	
	void  SendMessage(int id, const MSG & msg);

	int BroadcastMessage(const rect & rt,const MSG & message,float extend_size = 0.f)
	{
		rect region = rt;
		region.left -= extend_size;
		region.right += extend_size;
		region.top -= extend_size;
		region.bottom += extend_size;

		rdlock_scoped alock(_list_lock);
		size_t i;
		for(i = 0; i < _msg_near_list.size(); i ++)
		{
			node_t & node = _msg_near_list[i];
			if(region.IsOverlap(node.region))
			{
				SendMessage(node.id, message);
			}
		}
		return 0;
	}
	
	bool IsActiveID(int id) const 
	{
		return msg_runner_map[id] && msg_receiver_map[id];
	}

	int GetServerNear(const A3DVECTOR & pos) const
	{
		rdlock_scoped alock(_list_lock);
		size_t i;
		for(i = 0; i < _msg_near_list.size(); i ++)
		{
			int idx = _msg_near_list[i].id;
			if(_msg_near_list[i].tag != _world_tag) continue;
			if(_msg_near_list[i].region.IsIn(pos.x,pos.z) && IsActiveID(idx))
			{
				return idx;
			}
		}
		return -1;
	}

	int GetGlobalServer(const A3DVECTOR & pos,int tag) const
	{
		rdlock_scoped alock(_list_lock);
		size_t i;
		for(i = 0; i < _msg_gs_list.size(); i ++)
		{
			const node_t & node = _msg_gs_list[i];
			if(node.tag != tag) continue;
			int idx = node.id;
			if(node.region.IsIn(pos.x,pos.z) && IsActiveID(idx))
			{
				return idx;
			}
		}
		return -1;
	}

};

#endif

