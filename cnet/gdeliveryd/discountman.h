#ifndef __GNET_DISCOUNTMAN_H
#define __GNET_DISCOUNTMAN_H

#include "qpdiscountinfo"
#include "merchantdiscount"
#include <map>

namespace GNET
{

// 快捷支付的折扣管理
class DiscountMan
{
public:

	// 更新快捷支付的折扣信息
	void UpdateDiscount(const MerchantDiscountVector& discount);

	// 通知单个玩家折扣信息
	void NotifyDiscount(int linksid, int localsid);

	// 检测折扣是否正确，正确则返回true
	bool CheckDiscount(int cash, int cash_after_discount, int merchant_id);
	
	static DiscountMan* GetInstance() { return &_instance; }
private:
	static DiscountMan _instance; 
	QPDiscountInfoVector discount_;
};

} // end namespace GNET

#endif // __GNET_DISCOUNTMAN_H
