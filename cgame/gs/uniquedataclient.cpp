#include "uniquedataclient.h"
#include <spinlock.h>
#include <gsp_if.h>
#include "world.h"
#include <glog.h>
#include "player_imp.h"

#define UDC_LOCK()		spin_autolock keeper(_lock);
#define UDM_HEARTBEAT_INV   600   // 600tick = 30s
#define UDELEM_WRITE_TIMEOUT 30   // 30s

void UniqueDataClient::SUniqueDataElem::dump(char* str,size_t len)
{
	if(len < 128 || NULL == str) return;
	
	switch(vtype)
	{
		case UDT_INT:			
			{
				snprintf(str,len,"[type:INT][val=\"%d\"]",(int)syncval);
			}
			break;
		case UDT_DOUBLE:
			{
				snprintf(str,len,"[type:DOUBLE][val=\"%f\"]",(double)syncval);
			}
			break;
		case UDT_OCTETS:
			{
				snprintf(str,len,"[type:OCTETS][sz%d][val=\"%.*s\"]",syncval.size(),64,(char*)syncval.data());
			}
			break;
	}
}

void UniqueDataClient::SUniqueData::init(int worldtag)
{
	UniqueDataClient::SUniqueData::SGUMap::iterator iter = _elem_map.begin();
	UniqueDataClient::SUniqueData::SGUMap::iterator iend = _elem_map.end();

	for(;iter != iend; ++iter)
	{
		int key = iter->first;
		SUniqueDataElem& elem = *iter->second;
		
		if(elem.inittype && !elem.vtype ) // 未初始化的新值
		{
			elem.wlock = true;
			elem.wsync = true;
			elem.notifyrole	= false;
			elem.timeout = 0;
			__PRINTF("world:%d key%d first init to type%d\n",worldtag, key, elem.inittype);
		}
	}
}

void UniqueDataClient::SUniqueData::timeout(int worldtag)
{
	int now = g_timer.get_systime();

	UniqueDataClient::SUniqueData::SGUMap::iterator iter = _elem_map.begin();
	UniqueDataClient::SUniqueData::SGUMap::iterator iend = _elem_map.end();

	for(;iter != iend; ++iter)
	{
		int key = iter->first;
		SUniqueDataElem& elem = *iter->second;
		
		if(elem.wsync && elem.timever >= elem.clientver && elem.timeout < now )
		{
			GLog::log(GLOG_INFO,"UniqueData world:%d key%d write time out \n",worldtag, key);
			elem.timeout = now + UDELEM_WRITE_TIMEOUT;

			GMSV::ModifyUniqueData(key, elem.vtype ? elem.vtype : elem.inittype, 
						elem.stubval.data(), elem.stubval.size(),
						elem.localval.data(), elem.localval.size(), 
						worldtag, elem.wlock, elem.notifyrole, elem.timever, true);
		}
	}
}

void UniqueDataClient::SUniqueData::syncrole(gplayer* prole)
{
	UniqueDataClient::SUniqueData::SGUMap::iterator iter = _elem_map.begin();
	UniqueDataClient::SUniqueData::SGUMap::iterator iend = _elem_map.end();

	packet_wrapper h1(MAX_UDPACKET_LENTH),h2(MAX_UDPACKET_LENTH);
	unsigned int count = 0, lenth = 0;
	using namespace S2C;
	
	for(;iter != iend; ++iter)
	{
		int key = iter->first;
		SUniqueDataElem& elem = *iter->second;

		if(elem.notifyrole)
		{
			lenth += 12 + elem.syncval.size(); // 12 = sizeof(key type size)
	
			if(lenth > MAX_UDPACKET_LENTH - 6) //6 for head
			{
				h1.clear();
				CMD::Make<CMD::unique_data_notify>::From(h1,count);
				h1 << h2;
				send_ls_msg(prole,h1);
				h2.clear();
				count = 0;
				lenth = 12 + elem.syncval.size();
			}

			CMD::Make<CMD::unique_data_notify>::Add(h2,key,elem.syncval.type(),elem.syncval.data(),elem.syncval.size()); 
			++count;
		}
	}

	if(count)
	{
		h1.clear();
		CMD::Make<CMD::unique_data_notify>::From(h1,count);
		h1 << h2;
		send_ls_msg(prole,h1);
	}
}


void UniqueDataClient::OnLoadFinish() 
{ 
	UDC_LOCK();
	_state = UDC_STATE_OPEN;
	_data.init(_world_tag);
	_tickcount = UDM_HEARTBEAT_INV - 5; // 5 tick later init 

	// for debug
	class UniqueDataDebugTime : public UniqueDataClient::ModifyOperater	
	{
		public:	
			bool  OnInit(int key,const UDOctets& val) 
			{
				return true;
			}
			bool  OnModify(int key, const UDOctets& val, int retcode, bool localflag)
			{
				world_manager::GetUniqueDataMan().SetDebugTime(val);
				return true;			
			}
			bool  CheckModify(int key, const UDOctets& val, bool setflag)
			{
				return true;
			}
	};

	SUniqueDataElem& elem = _data[1000000];
	if(!elem.operater) elem.operater = new UniqueDataDebugTime();
	
	GLog::log(GLOG_INFO,"UniqueData world:%d uniquedata system load ok \n",_world_tag);
}

void UniqueDataClient::OnSystemClose()
{
	UDC_LOCK();
	_state = UDC_STATE_CLOSE;
	GLog::log(GLOG_INFO,"UniqueData world:%d uniquedata system close \n",_world_tag);
}



void UniqueDataClient::_DebugModifySync()
{
	if(_dbg_time)
	{
		--_dbg_time;	
		ModifyDataInCallback(999998,0.00001f,false);
		ModifyDataInCallback(999999,1,false);
	}
}

void UniqueDataClient::OnHeartbeat()
{
	UDC_LOCK();

	_DebugModifySync();

	if(++_tickcount < UDM_HEARTBEAT_INV)
		return;

	_tickcount = 0;

	if(UDC_STATE_UNINIT == _state)
	{
		GMSV::InitUniqueData(_world_tag,0);
		return;
	}

	_data.timeout(_world_tag);

}

void UniqueDataClient::OnDataLoad(int key,int type,int ver,const void* p,size_t sz)
{
	UDC_LOCK();

	if(0 == sz)
	{
		GLog::log(GLOG_ERR,"UniqueData world:%d key%d type%d version%d uniquedata load  fail, size 0 \n",_world_tag, key, type ,ver);
		return;
	}

	if(type <= UDT_INVALID || type >= UDT_COUNT)
	{
		GLog::log(GLOG_ERR,"UniqueData world:%d key%d type%d version%d uniquedata load  fail, wrong type \n",_world_tag, key, type ,ver);
		return;
	}

	SUniqueDataElem& elem = _data[key];	
	
	if(elem.vtype && key == 0) // 只由一个key负责记录reload log
	{
		GLog::log(GLOG_ERR,"UniqueData world:%d Reload!",_world_tag);
	}	

	UDOctets val(p,sz,type);
	elem.init(val,ver);
	
	if(elem.operater)
	{
		elem.operater->OnInit( key, elem.syncval );
	}

	__PRINTF("unique unique data load key %d type%d size%d\n", key, type, sz);
}
	
void UniqueDataClient::OnDataModify(int worldtag,int key,int type, const void* nval, size_t nsz,const void* oval, size_t osz, int retcode, int version)
{
	UDC_LOCK();
	
	SUniqueDataElem& elem = _data[key];

	if(elem.serverver > version)
	{
		GLog::log(GLOG_ERR,"UniqueData world:%d key%d modify version error old%d new%d \n",_world_tag, key, elem.serverver, version);
		return;
	}
	
	elem.serverver = version;
	UDOctets newval(nval,nsz,type);
	UDOctets oldval(oval,osz,type);
	bool init = (elem.vtype == UDT_INVALID);
	
	if(elem.vtype && elem.vtype != type) // 本地数据类型与delivery上不一致，可能出现在2个gs同时对key进行不同类型初始化
	{
		elem.wsync = false;
		elem.wlock = false;
		//输出log 按delivery上的结果进行本地同步
		GLog::log(GLOG_ERR,"UniqueData world:%d key%d type%d reset to octets-type \n",_world_tag, key, elem.vtype);
	}

	if(worldtag == _world_tag && elem.wsync 
		&& (init || elem.stubval == oldval) ) // 是本服修改
	{
		elem.wsync = false;

		if(retcode) // error
		{
			//log
			GLog::log(GLOG_ERR,"UniqueData world:%d key%d type%d uniquedata modify fail[%d] \n",_world_tag, key, type, retcode);

			if(retcode == ERR_UNIQUE_VERSION_FAIL && elem.timever >= elem.clientver) // 本地timeout修改因版本未提交成功,才重新提交
			{
				elem.wsync = true;
				elem.timever = version;
				elem.timeout = g_timer.get_systime() + UDELEM_WRITE_TIMEOUT;
	
				GMSV::ModifyUniqueData(key, elem.vtype ? elem.vtype : elem.inittype, 
						elem.stubval.data(), elem.stubval.size(),
						elem.localval.data(), elem.localval.size(), 
						worldtag, elem.wlock, elem.notifyrole, elem.timever, true);

				return;
			}
		}
		else // 尝试合并同步期修改
		{
			elem.clientver = version;
			bool merge = false;
			
			if(!init && !elem.wlock )  // 非独占写或同步 检查 同步期间是否发生了本地修改
			{
				switch(elem.vtype)
				{
					case UDT_INT:
						{
							int dsval = newval;
							int lsval = elem.syncval;
							int chgval = dsval - lsval;
							int llval = elem.localval;
							int stval = elem.stubval;
							int localchgval = llval - stval - chgval;
	
							if(localchgval != 0)
							{
								merge = true;
								elem.stubval = newval;
								elem.localval = dsval + localchgval;
							}
						}
						break;
					case UDT_DOUBLE:
						{
							double dsval = newval;
							double lsval = elem.syncval;
							double chgval = dsval - lsval;
							double llval = elem.localval;
							double stval = elem.stubval;
							double localchgval = llval - stval - chgval;
	
							if(fabs(localchgval) > 1e-8)
							{
								merge = true;
								elem.stubval = newval;
								elem.localval = dsval + localchgval;
							}
						}
						break;
					case UDT_OCTETS:
						{
							if(elem.localval != newval)
							{
								merge = true;
								elem.stubval = newval;
							}
						}
						break;
				}
			
			}
		
			if(merge)// 需要合并
			{
				elem.wsync = true;  // 继续向delivery请求修改
				
				elem.timeout = g_timer.get_systime() + UDELEM_WRITE_TIMEOUT;
				elem.timever = elem.serverver;
				
				GMSV::ModifyUniqueData(key, elem.vtype, elem.stubval.data(), elem.stubval.size(),
						elem.localval.data(), elem.localval.size(), _world_tag, false, elem.notifyrole,elem.serverver, false);

			}
			else// 移动桩数据，防止请求多次返回
			{
				elem.stubval = newval;
				elem.localval = newval;
			}
			
		}
		
		elem.wlock = false;
	}
	else if(worldtag == _world_tag)
	{
		GLog::log(GLOG_ERR,"UniqueData Modify Warning world:%d key%d type%d wlock%d sync%d old%d new%d syval%d loval%d stval%d \n",
				_world_tag, key, elem.vtype, elem.wlock?1:0, elem.wsync?1:0,
				(int)oldval,(int)newval,(int)elem.syncval,(int)elem.localval,(int)elem.stubval);
	}


	if(init)
	{
		elem.init(newval,version);

		if(elem.operater)
		{
			elem.operater->OnInit( key, newval );
		}
	}
	else
	{
		elem.update(newval);
		
		if(elem.operater)	
		{
			elem.operater->OnModify( key, newval, retcode, worldtag == _world_tag );
		}
	}

}


int	UniqueDataClient::ModifyData(int key, UDOctets val, bool setflag)
{
	UDC_LOCK();
	int ret = _ModifyDataNoLock(key,val,setflag);
	return ret;
}

int UniqueDataClient::ModifyDataInCallback(int key, UDOctets val, bool setflag)
{
	return _ModifyDataNoLock(key,val,setflag);
}

int UniqueDataClient::_ModifyDataNoLock(int key, UDOctets& val, bool setflag)
{
	if(UDC_STATE_OPEN != _state)
		return -1;
	
	SUniqueDataElem& elem = _data[key];
	int type = val.type();

	if(elem.operater && !elem.operater->CheckModify(key,val,setflag))
		return -2;
	
	if(elem.freeze)
		return -3;

	if(elem.vtype && elem.vtype != type)
		return -4;

	if(elem.wlock)  // 写锁下不能复写
		return -5;

	if(setflag || UDT_INVALID == elem.vtype)
	{
		if(elem.wsync) // 同步中不允许加独占写
			return -6;

		elem.stubval = elem.syncval;
		elem.localval = val;

		elem.wlock = true;
		elem.wsync = true;

		elem.timeout = g_timer.get_systime() + UDELEM_WRITE_TIMEOUT;
		elem.timever = elem.serverver;
	
		GMSV::ModifyUniqueData(key, type, elem.stubval.data(), elem.stubval.size(), 
					elem.localval.data(), elem.localval.size(), _world_tag, true, elem.notifyrole, elem.serverver, false);
	}
	else
	{
		if(elem.wsync) // 同步中只用更新本地值即可
		{
			elem.localval += val;
			return 1;
		}
		else
		{
			elem.stubval  = elem.syncval;
			elem.localval = elem.stubval;
			elem.localval += val;

			elem.wsync = true;

			elem.timeout = g_timer.get_systime() + UDELEM_WRITE_TIMEOUT;
			elem.timever = elem.serverver;

			GMSV::ModifyUniqueData(key, type, elem.stubval.data(), elem.stubval.size(), 
					elem.localval.data(), elem.localval.size(), _world_tag, false, elem.notifyrole, elem.serverver, false);
		}
	}


	return 0;
}

void UniqueDataClient::Register(int key,UDOctets initval, ModifyOperater* cb, bool noticeclient, bool freeze)
{
	UDC_LOCK();
	
	SUniqueDataElem& elem = _data[key];	

	if(UDC_STATE_OPEN != _state)
	{
		elem.inittype = initval.type();
		elem.localval = initval;
	}
	
	elem.notifyrole = noticeclient;
	elem.freeze = freeze;
	
	if(cb)
	{
		if(elem.operater) 
		{
			GLog::log(GLOG_ERR,"UniqueData world:%d key%d already have a modifyoperater\n",_world_tag, key);
			return;
		}

		elem.operater = cb;
		if (elem.vtype && elem.serverver)	cb->OnInit(key,elem.syncval);
		__PRINTF("unique data %d register a operater\n", key);
	}
}

bool UniqueDataClient::GetData(int key,UDOctets & val)
{
	UDC_LOCK();
	if(!_data.exist(key) || _data[key].vtype != val.type())
		return false;

	val = _data[key].syncval;
	return true;
}

void UniqueDataClient::OnRoleLogin(gplayer* prole)
{
	UDC_LOCK();
	if(!prole || UDC_STATE_OPEN != _state) return;
	_data.syncrole(prole);
}

void UniqueDataClient::OnRoleQuery(gplayer* prole, int count, int keys[])
{
	UDC_LOCK();

	if(UDC_STATE_OPEN != _state)
		return;

	if(count > 40)
		return;

	packet_wrapper h1(MAX_UDPACKET_LENTH);
	using namespace S2C;
	CMD::Make<CMD::unique_data_notify>::From(h1,count);
	
	for(int n = 0; n < count; ++n)
	{
		if( _data.exist(keys[n]) && prole)
		{
			SUniqueDataElem& elem = _data[keys[n]];	
			CMD::Make<CMD::unique_data_notify>::Add(h1,keys[n],elem.syncval.type(),elem.syncval.data(),elem.syncval.size()); 
		}
		else
		{
			//log
			CMD::Make<CMD::unique_data_notify>::Add(h1,keys[n],UDT_INVALID,0,0); 
		}
	}

	send_ls_msg(prole,h1);
}

int UniqueDataClient::OnDump(int key,char* str,size_t len)
{
	UDC_LOCK();

	if(str == NULL || len < 128)
		return -1;

	if( UDC_STATE_OPEN != _state || !_data.exist(key))
	{
		sprintf(str,"Not Exist!");
		return -1;
	}

	memset(str,0,len);

	_data[key].dump(str,len);

	return _data.next(key);
}

