/*
 *		PROXYRPC头文件代码
 *		作者：未知
 *		修改：杨延昭
 *		时间：2009-07
 *		公司：完美时空
 *
 */	
#ifndef __PROXYRPC_H
#define __PROXYRPC_H

#include "marshal.h"
#include "rpc.h"

namespace GNET
{

class ProxyRpc : public TimerProtocol
{
protected:
	PManager *_proxy_manager;
	SESSION_ID _proxy_sid;
	unsigned int _proxy_xid;
	OctetsStream _proxy_data;

	unsigned int _xid;
public:
	~ProxyRpc () {}
	ProxyRpc(PROTOCOL_TYPE type, ProtocolStubManager *stubman):TimerProtocol(type, stubman), _proxy_manager(NULL) {}
	ProxyRpc(PROTOCOL_TYPE type, Rpc::Data* arg, Rpc::Data *res):TimerProtocol(type, ProtocolStubManager::GetGlobalStub()), _proxy_manager(NULL){if (arg) arg->Destroy(); if (res) res->Destroy();}
	ProxyRpc(PROTOCOL_TYPE type, ProtocolStubManager *stubman, Rpc::Data* arg, Rpc::Data *res):TimerProtocol(type, stubman), _proxy_manager(NULL){if (arg) arg->Destroy(); if (res) res->Destroy();}
	ProxyRpc(const ProxyRpc &rhs) : TimerProtocol(rhs),
				_proxy_manager(rhs._proxy_manager), _proxy_sid(rhs._proxy_sid), _proxy_xid(rhs._proxy_xid),
				/*_proxy_data(rhs._proxy_data),*/ _xid(rhs._xid)
	{
		_proxy_data.swap(REMOVE_CONST(rhs._proxy_data));
	}
	OctetsStream& marshal(OctetsStream &os) const;
	const OctetsStream& unmarshal(const OctetsStream &os);

	//重设Argument，只对当前和后续的处理方起作用，对发起方无效
	//只能在Delivery中使用
	//推荐在向其它manager转发本协议之前设
	void ResetArgument( const Rpc::Data * arg)
	{
		_proxy_data.clear();
		_proxy_data << _xid << *arg;
	}
	void ResetArgument( const Rpc::Data & arg) { ResetArgument(&arg); }

	void SetResult( const Rpc::Data * res )
	{
		_proxy_data.clear();
		_proxy_data << _proxy_xid << *res;
	}
	void SetResult( const Rpc::Data & res) { SetResult(&res); }

	void SendToSponsor( )
	{
		if( _proxy_manager )
		{
			_xid = _proxy_xid;
			_proxy_manager->Send( _proxy_sid, *this );
		}
	}
	void Process(PManager *manager, SESSION_ID sid);

	virtual bool Delivery(SESSION_ID _proxy_sid, const OctetsStream &osArg) { return false; }
	virtual bool Delivery(SESSION_ID _proxy_sid, OctetsStream &osArg) { return Delivery(_proxy_sid, (const OctetsStream &)osArg); }
	virtual void PostProcess(SESSION_ID _proxy_sid, const OctetsStream &osArg, const OctetsStream &osRes) { }
	virtual void OnTimeout() { _proxy_data>>_xid; OnTimeout(_proxy_data); }
	virtual void OnTimeout(const OctetsStream &osArg) { }
	virtual bool TimePolicy(int timeout) const { return timeout < 5; }
};

};

#endif

