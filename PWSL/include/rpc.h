/*
 *		RPC头文件代码
 *		作者：未知
 *		修改：杨延昭
 *		时间：2009-07
 *		公司：完美时空
 *
 */
#ifndef __RPC_H_
#define __RPC_H_

#include "protocol.h"

namespace GNET {

class Rpc : public TimerProtocol
{
public:
	struct Data : public Marshal
	{
		virtual ~Data() { }
		virtual Data *Clone() const = 0;
		virtual void Destroy() { delete this; }
		virtual Data& operator = (const Data &rhs) { return *this; }
	};
	union AuxData
	{
		int i;
		int save_sid;
		int id[2];
		uint32_t u32;
		uint64_t u64;
		void *ptr;
	};
	typedef unsigned int XID;
public:
	AuxData _auxdata;	//本地辅助数据，用于保存一些信息。当rpc调用返回时可以利用这些信息。该数据不会被marshal及传送。
	Data *argument;		//调用的参数
	Data *_result;		//结果
	unsigned int _xid;	//用以区分不同的Rpc实例, 最高一位表示是请求还是回应
public:
	inline static bool IsRequest(unsigned int xid) { return xid & 0x80000000; }
	inline static void SetRequest(unsigned int& xid) {xid |= 0x80000000;}
	inline static void ClrRequest(unsigned int& xid) {xid &= 0x7FFFFFFF;}

	Rpc(){}
	Rpc(PROTOCOL_TYPE type, ProtocolStubManager *stubman, Data* arg, Data *res):
		TimerProtocol(type, stubman),argument(arg), _result(res){}
	Rpc(PROTOCOL_TYPE type, Data* arg, Data *res):TimerProtocol(type, ProtocolStubManager::GetGlobalStub()),argument(arg), _result(res){}
	Rpc(Data *arg, Data *res):argument(arg), _result(res) { }
        Rpc(const Rpc &rhs) : TimerProtocol(rhs), argument(rhs.argument->Clone()), _result(rhs._result->Clone()),
				_xid(rhs._xid){}
	virtual ~Rpc()
	{
		argument->Destroy();
		_result->Destroy();
	}
	static Rpc *Call(PROTOCOL_TYPE type, const Data *arg, PManager *manager);
	static Rpc *Call(PROTOCOL_TYPE type, const Data &arg, PManager *manager) { return Call(type, &arg,manager); }

	static Rpc *Call(PROTOCOL_TYPE type, const Data *arg);						// 保持调用的兼容性
	static Rpc *Call(PROTOCOL_TYPE type, const Data &arg) { return Call(type, &arg); }		// 保持调用的兼容性
private:
	virtual OctetsStream& marshal(OctetsStream &os) const;
	virtual const OctetsStream& unmarshal(const OctetsStream &os);
	virtual void Process(PManager *manager, SESSION_ID sid);	//协议的处理函数，根据是否请求，来调用Server或Client
	virtual void Server(Data *argument, Data *result, PManager *manager, SESSION_ID sid) { Server(argument,result); } //接受方的处理
	virtual void Client(Data *argument, Data *result, PManager *manager, SESSION_ID sid) { Client(argument,result); } //收到请求的结果
	virtual void Server(Data *argument, Data *result) {  }
	virtual void Client(Data *argument, Data *result) {  }
	virtual void OnTimeout() { OnTimeout(argument); }
	virtual void OnTimeout(Data *argument) { }	//超时
};

}
#endif
