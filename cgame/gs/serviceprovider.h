#ifndef __ONLINEGAME_GS_SERVICE_PROVIDER_H__
#define __ONLINEGAME_GS_SERVICE_PROVIDER_H__

#include <amemobj.h>
#include <hashtab.h>
#include <common/types.h>
#include "actobject.h"
#include "staticmap.h"

//npc端的服务判定
//每个服务在每个npc里都会保存一个个体
class gplayer_imp;
class service_provider : public abase::ASmallObject
{
protected:
	gactive_imp * _imp;
	int _type;
	bool _is_init;
	
private:
	friend class service_manager;

	virtual service_provider * Clone() = 0;
	virtual bool OnInit(const void * buf, size_t size) = 0;
	virtual void GetContent(const XID & player,int cs_index,int sid) = 0;
	virtual void TryServe(const XID & player, const void * buf, size_t size)=0;
	virtual void OnHeartbeat() = 0;
	virtual void OnControl(int param ,const void * buf, size_t size) = 0;
public:
	virtual bool Save(archive & ar) = 0;
	virtual bool Load(archive & ar) = 0;
public:
	service_provider():_imp(NULL),_type(-1),_is_init(false){}
	virtual ~service_provider() {}
	bool Init(gactive_imp * imp,const void * buf, size_t size)
	{
		ASSERT(_is_init == false);
		_imp = imp;
		bool bRst = OnInit(buf,size);
		if(bRst) _is_init = true;
		return bRst;
	}
	
	void ReInit(gactive_imp * imp)
	{
		_imp = imp;
		_is_init = true;
	}

	void GetServiceContent(const XID & player, int cs_index, int sid)
	{
		ASSERT(_is_init);
		GetContent(player,cs_index,sid);
	}

	void PayService(const XID & player,const void * buf, size_t size)
	{
		ASSERT(_is_init);
		TryServe(player,buf,size);
	}

	void Control(int param ,const void * buf, size_t size)
	{
		ASSERT(_is_init);
		OnControl(param,buf,size);
		
	}

	void Heartbeat()
	{
		ASSERT(_is_init);
		OnHeartbeat();
	}

	int GetProviderType()
	{
		return _type;
	}
	
};

//player端的服务执行
//由于整个世界只有一份，所以不需要从ASmallObject里继承而来
class service_executor
{
public:
	virtual ~service_executor() {}
protected:
	friend class service_manager;
	int _type;

private:
	virtual bool OnServe(gplayer_imp *pImp, const XID & provider,const A3DVECTOR & pos, const void * buf, size_t size) = 0;

	virtual bool SendRequest(gplayer_imp *pImp, const XID & provider,const void * buf, size_t size) = 0;

public:
	bool Serve(gplayer_imp *pImp, const XID & provider,const A3DVECTOR & pos,const void * buf, size_t size)
	{
		return OnServe(pImp,provider,pos,buf,size);
	}

	bool ServeRequest(gplayer_imp *pImp, const XID & provider, const void * buf, size_t size)
	{
		return SendRequest(pImp,provider,buf,size);
	}
};

class service_manager
{
public:
	typedef abase::hashtab<service_provider*,int, abase::_hash_function>  PROVIDER_TAB;
	typedef abase::hashtab<service_executor*,int, abase::_hash_function>  EXECUTOR_TAB;
	
private:
	PROVIDER_TAB	_provider_map;
	EXECUTOR_TAB	_executor_map;

private:
	
	service_manager():_provider_map(50),_executor_map(50)
	{}


	void InsertService(int type, service_provider * provider, service_executor * executor)
	{	
		bool bRst1 = _provider_map.put(type,provider);
		bool bRst2 = _executor_map.put(type,executor);
		provider->_type = type;
		executor->_type = type;
		ASSERT(bRst1 && bRst2);
		if(!(bRst1 && bRst2))
		{
			__PRINTINFO("service_manager:添加功能服务失败\n");
			throw 100;
		}
	}
	
	service_provider * __GetProvider(int type)
	{
		service_provider ** pRst = _provider_map.nGet(type);
		if(pRst) 
			return *pRst;
		else
			return NULL;
	}

	service_executor * __GetExecutor(int type)
	{
		service_executor ** pRst = _executor_map.nGet(type);
		if(pRst) 
			return *pRst;
		else
			return NULL;
	}

	static service_manager & GetInstance()
	{
		static service_manager __instance;
		return __instance;
	}
	friend class service_inserter;
public:
	
	static inline service_provider * CreateProviderInstance(int type) 
	{
		service_provider * __p = GetInstance().GetProvider(type);
		if(!__p) 
		{
			ASSERT(false && "can not find provider");
			return NULL;
		}
		return __p->Clone();
	}

	static inline service_provider * GetProvider(int type) 
	{
		return GetInstance().__GetProvider(type);
	}
	
	static inline service_executor * GetExecutor(int type) 
	{
		return GetInstance().__GetExecutor(type);
	}

};

template <size_t capacity>
class provider_list
{
	typedef abase::static_map<int,service_provider *,capacity> LIST;
	LIST _list;
public:
	typedef typename LIST::iterator iterator;
public:

	~provider_list()
	{
		Clear();
	}
	
	bool AddProvider(service_provider * provider)
	{
		if(!(_list.insert(typename LIST::value_type(provider->GetProviderType(),provider)).second))
		{
			ASSERT(false && "异常情况");
			return false;
		}
		return true;
	}

	size_t size()
	{
		return _list.size();
	}

	iterator begin()
	{
		return _list.begin();
	}

	iterator end()
	{
		return _list.end();
	}

	void Clear()
	{
		typename LIST::iterator it = _list.begin();
		for(; it != _list.end(); ++it)
		{
			delete it->second;
		}
		return ;
	}

	service_provider * GetProvider(int type)
	{
		typename LIST::iterator it = _list.find(type);
		if(it == _list.end())
		{
			return NULL;
		}
		return it->second;
	}
};

//将服务提供商加入到管理器的辅助类
class service_inserter 
{
public:

	template <typename PROVIDER, typename EXECUTOR>
	service_inserter(PROVIDER *, EXECUTOR *,int type)
	{
		service_manager::GetInstance().InsertService(type,new PROVIDER, new EXECUTOR);
	}
};

#define SERVICE_INSERTER(PROVIDER,EXECUTOR,type) service_inserter((PROVIDER*)NULL,(EXECUTOR*)NULL,type);

namespace service_ns
{
	enum
	{
		SERVICE_ID_NPC_VENDOR	= 1,
		SERVICE_ID_NPC_PURCHASE = 2,
		SERVICE_ID_REPAIR	= 3,
		SERVICE_ID_HEAL		= 4,
		SERVICE_ID_TRANSMIT	= 5,
		SERVICE_ID_TASK_IN	= 6,
		SERVICE_ID_TASK_OUT	= 7,
		SERVICE_ID_TASK_MATTER	= 8,
		SERVICE_ID_SKILL	= 9,
		SERVICE_ID_INSTALL	= 10,
		SERVICE_ID_UNINSTALL	= 11,
		SERVICE_ID_PRODUCE	= 12,
		SERVICE_ID_DECOMPOSE	= 13,
		SERVICE_ID_TRASHBOX_PASS= 14,
		SERVICE_ID_TRASHBOX_OPEN= 15,
		SERVICE_ID_INSTANCE_SW	= 16,
		SERVICE_ID_IDENTIFY	= 17,
		SERVICE_ID_FACTION	= 18,
		SERVICE_ID_PLAYER_MARKET= 19,		//商店卖出
		SERVICE_ID_VEHICLE	= 20,
		SERVICE_ID_PLAYER_MARKET2= 21,		//商店买进
		SERVICE_ID_WAYPOINT	= 22,
		SERVICE_ID_UNLEARN_SKILL= 23,
		SERVICE_ID_COSMETIC	= 24,
		SERVICE_ID_MAIL		= 25,
		SERVICE_ID_AUCTION	= 26,
		SERVICE_ID_DOUBLE_EXP	= 27,
		SERVICE_ID_HATCH_PET	= 28,
		SERVICE_ID_RECOVER_PET	= 29,

	        SERVICE_ID_ADDON_CHANGE = 81,		// 魂石转化 
		SERVICE_ID_ADDON_REPLACE = 82,		// 魂石替换 	
	};
}
#endif

