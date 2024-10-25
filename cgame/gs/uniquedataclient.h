#ifndef __ONLINEGAME_GS_UNIQUE_DATA_MAN_H__
#define __ONLINEGAME_GS_UNIQUE_DATA_MAN_H__

#include "amemobj.h"
#include <map>
#include <octets.h>

enum UNIQUE_DATA_ENUM   // 各系统独占变量
{
	UDI_HISTORY_VERSION = 0,
};

enum UNIQUE_DATA_TYPE // 同localmacro.h
{
	UDT_INVALID,		// 未使用
	UDT_INT,			// 整数
	UDT_DOUBLE,			// 浮点数
	UDT_OCTETS,			// 变长

	UDT_COUNT	
};

enum UNIQUE_MODIFY_ERR // localmacro.h
{
	ERR_UNIQUE_CLASH = 436,         // 唯一值设置冲突 
	ERR_UNIQUE_TYPE_INVALID = 437,  // 唯一值类型错误
	ERR_UNIQUE_VERSION_FAIL = 438,  // 唯一值版本错误
};

// -----------  abase::octets 壳，负责类型安全转化 ---------------
struct UDOctets
{
	UDOctets() : _type(UDT_INVALID) {}
	UDOctets(int n) : _type(UDT_INT), _data(&n,sizeof(int)) {}
	UDOctets(double n) : _type(UDT_DOUBLE), _data(&n,sizeof(double)) {}
	UDOctets(const void* p,size_t sz,int type = UDT_OCTETS) : _type(type), _data(p,sz) {}
	UDOctets(const UDOctets& rhs) : _type(rhs._type), _data(rhs._data) {}

	operator const int&() const { ASSERT(_type == UDT_INT && _data.size() == sizeof(int) && "UDOctets operator int fail!");  return *((const int*)data()); }
	operator const double&() const { ASSERT(_type == UDT_DOUBLE && _data.size() == sizeof(double) && "UDOctets operator double fail!"); return *((const double*)data());}

	UDOctets& operator +=(const UDOctets& rhs)
	{
		ASSERT( _type == rhs._type && "UDOctets operator += in UDOctets type fail!");
		
		_type = rhs._type;

		switch(_type)
		{
			case UDT_INT:
				{
					reserve(sizeof(int));
					int nval = *((int*)data());
					*((int*)data()) = nval + *((int*)rhs.data());
				}
				break;
			case UDT_DOUBLE:
				{
					reserve(sizeof(double));
					double dval = *((double*)data());
					*((double*)data()) = dval + *((double*)rhs.data());
				}
				break;
			case UDT_OCTETS:
				{
					_data = rhs._data;			
				}
				break;
		}
		
		return *this;
	}
	UDOctets& operator =(const UDOctets& rhs)
	{
		ASSERT(!(_type && _type != rhs._type) && "UDOctets operator = in UDOctets type fail!");
		
		_type = rhs._type;
		_data = rhs._data;
		
		return *this;
	}
	UDOctets& operator =(int irhs)
	{
		ASSERT(!(_type && _type != UDT_INT) && "UDOctets operator = in INT type fail!");
			
		_type = UDT_INT;
		reserve(sizeof(int));
		*((int*)data()) = irhs;
		
		return *this;
	}
	UDOctets& operator =(double drhs)
	{
		ASSERT(!(_type && _type != UDT_DOUBLE) && "UDOctets operator = in DOUBLE type fail!");
			
		_type = UDT_DOUBLE;
		reserve(sizeof(double));
		*((double*)data()) = drhs;
		
		return *this;
	}
	bool operator ==(const UDOctets& rhs)
	{
		return _type == rhs._type && _data == rhs._data;
	}
	bool operator !=(const UDOctets& rhs)
	{
		return !(*this == rhs);
	}

	int 	type() 	const { return _type;}
	size_t 	size() 	const { return _data.size();}
	void 	reserve(size_t n) { _data.reserve(n);}
	void*	data()	{ return _data.begin();}
	const void* data() const { return _data.begin();}
	
	int 		  _type;
	abase::octets _data;
};

class gplayer;
class UniqueDataClient
{
	enum 
	{
		UDC_STATE_UNINIT,
		UDC_STATE_OPEN,
		UDC_STATE_CLOSE,
	};
	
	friend class world_manager;
	UniqueDataClient(): _world_tag(-1),_modify_version(0), _state(UDC_STATE_UNINIT), _tickcount(0), _lock(0), _dbg_time(0) {}
	void  SetWorldTag(int wt) { _world_tag = wt ;}

	static const size_t MAX_UDPACKET_LENTH = 1024;  // 消息报长度
public:

	// -----------  全局变量值操作函数 ---------------
	struct ModifyOperater
	{
		// -----------  值初始化回调函数 ---------------
		// key 全局变量键值
		// val 全局变量值
		virtual bool  OnInit(int key, const UDOctets& val) 		{return false;}
		// -----------  值改动后回调函数 ---------------
		// key 全局变量键值
		// val 全局变量值
		// retcode   |0 成功 |436 设置冲突 |437 设置类型不符 |438 版本错误
		// localflag |true 本服修改  | false 异服修改
		virtual bool  OnModify(int key, const UDOctets& val, int retcode, bool localflag) 	{ return false;}
		// -----------  值改动前预判函数 --------------
		// key 全局变量键值
		// val 全局变量值
		// setflag 改变或设置
		virtual bool  CheckModify(int key,const UDOctets& val, bool setflag)		{ return false;}
	};

//	------------------------- 数据注册 ---------------------------------
//  initval 数据初始化 在db未存在此值时会进行唯一次初始化
//  noticeclient 角色上线或修改后通知客户端 
//  freeze 本服是否禁止修改 
//  callback 数据改动操作
	void 	Register(int key,UDOctets initval, ModifyOperater* callback = NULL, bool noticeclient = false,bool freeze = false);	

/// 查询  取得最新同步后的值 返回失败
	bool	GetData(int key,UDOctets & val);

/// 修改
	int		ModifyData(int key, UDOctets val, bool setflag);
/// ModifyOperater 专用modify接口，防止死锁
	int 	ModifyDataInCallback(int key, UDOctets val, bool setflag);
/// --------------------------- 非使用接口 ------------------------------------------------
public:	
/// tick
	void 	OnHeartbeat();
/// 初始化	
	void  	OnDataLoad(int key,int type,int version, const void* p,size_t sz);
	void 	OnLoadFinish();
	void 	OnSystemClose();
/// 修改回调
	void  	OnDataModify(int worldtag,int key, int type, const void* val, size_t sz,const void* oldval, size_t osz, int retcode, int version);
/// 玩家登陆
	void 	OnRoleLogin(gplayer* prole);
/// 玩家查询
	void 	OnRoleQuery(gplayer* prole, int count,int keys[]);
/// 调试
	int 	OnDump(int key,char* str,size_t len);
	void 	SetDebugTime(int dt) { _dbg_time = dt; }
private:
	int 	_ModifyDataNoLock(int key,UDOctets& val,bool setflag);
	void	_DebugModifySync();
private :
	struct SUniqueDataElem
	{
		SUniqueDataElem() : wlock(false), wsync(false), freeze(false), vtype(0),inittype(0), operater(NULL), 
						timeout(0), notifyrole(false) ,timever(0),clientver(0), serverver(0)
		{
		}
		~SUniqueDataElem() 
		{
			if(operater) delete operater;
		}
		

		void init(const UDOctets& val,int ver)
		{
			vtype = val._type;
			syncval = val;
			stubval = val;
			localval = val;
			timever = clientver = serverver = ver;
		}
		void update(const UDOctets& val)
		{
			syncval = val;
		}

		void dump(char* str,size_t len);

		bool wlock;         // 独占写
		bool wsync;			// 写同步中
		bool freeze;		// 本地修改禁止
		int  vtype;			// 数据类型	
		int  inittype;		// 待初始化数据类型

		UDOctets syncval;  // sync已同步值
		UDOctets stubval;  // stub提交改变时刻本地已同步值，
		UDOctets localval; // local同步过程中本地改变到的值
		
		ModifyOperater* operater;
		int	 timeout;
		bool notifyrole;
		
		int	 timever;		// timeout用版本
		int  clientver;		// 本地修改版本
		int  serverver;		// 服务器版本
	};

	class SUniqueData
	{
	public:
		~SUniqueData()
		{
			SGUMap::iterator iter = _elem_map.begin();
			SGUMap::iterator iend = _elem_map.end();

			for(;iter != iend; ++iter)
			{
				delete iter->second;
			}

			_elem_map.clear();
		}
		
		SUniqueDataElem& operator[](int key) 
		{ 
			SGUMap::iterator iter = _elem_map.find(key);
			if(iter == _elem_map.end()) 
				iter = _elem_map.insert(_elem_map.begin(), std::make_pair(key,new SUniqueDataElem()));
			return *iter->second;
		}
		
		void init(int worldtag);
		void timeout(int worldtag);
		void syncrole(gplayer* prole);
		bool exist(int key) { return _elem_map.find(key) != _elem_map.end();}
		int  next(int key) 
		{ 
			SGUMap::iterator iter = _elem_map.find(key); 
			if(iter == _elem_map.end())
				return -1;
			if(++iter == _elem_map.end())
				return -1;
			return iter->first;
		}	
	private:
		typedef std::map<int,SUniqueDataElem* const> SGUMap;
		SGUMap _elem_map;
	} _data;

	int	 _world_tag;
	int  _modify_version;	
	int  _state;
	int  _tickcount;
	int  _lock;	// ud 数据操作锁

	//// for debug
	int  _dbg_time;
};

#endif
