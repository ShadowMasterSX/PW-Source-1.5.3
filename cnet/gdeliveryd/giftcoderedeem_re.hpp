
#ifndef __GNET_GIFTCODEREDEEM_RE_HPP
#define __GNET_GIFTCODEREDEEM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GiftCodeRedeem_Re : public GNET::Protocol
{
	#include "giftcoderedeem_re"
	enum
	{
		GCR_SUCCESS,			// 成功
		GCR_UNACTIVE_CARD,		// 未激活
		GCR_PRE_LIMIT,			// 父类型重复
		GCR_COMPLETE,			// 已完成
		GCR_NOT_OWNER,			// 非拥有
		GCR_INVALID_CARD,		// 无效卡号
		GCR_OUT_DATE,			// 过期
		GCR_OTHER_USED,			// 其他角色已使用
		GCR_NET_FAIL,			// 重新尝试
		GCR_NOT_BIND,			// 未绑定账号
		GCR_UNMARSHAL_FAIL,		// 数据错误
		GCR_INVALID_USER,		// 无效账号
		GCR_TYPE_LIMIT,			// 类型重复
		GCR_INVALID_ROLE,		// 无效角色
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
