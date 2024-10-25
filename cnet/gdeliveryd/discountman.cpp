
#include "discountman.h"
#include "qpannouncediscount.hpp"
#include "gdeliveryserver.hpp"
#include "maplinkserver.h"
#include "merchantdiscount"

namespace GNET
{

DiscountMan DiscountMan::_instance;
	
void DiscountMan::UpdateDiscount(const MerchantDiscountVector& discount)
{
	discount_.clear();

	for (unsigned int i = 0; i < discount.size(); i++)
	{
		const MerchantDiscount& md = discount[i];
		QPDiscountInfo di;
		di.id = md.id;
		di.name = md.name;
		for (unsigned int j = 0; j < md.discount.size(); j++)
		{
			const DiscountGrade& dg = md.discount[j];
			QPDiscountLevel dl;
			dl.amount_begin = dg.amount_begin;
			dl.discount = dg.discount;

			di.discount.push_back(dl);
		}
		discount_.push_back(di);
	}
}

void DiscountMan::NotifyDiscount(int linksid, int localsid)
{
    QPAnnounceDiscount proto(localsid, discount_);

    GDeliveryServer::GetInstance()->Send(linksid, proto);
}

bool DiscountMan::CheckDiscount(int cash, int cash_after_discount, int merchant_id)
{
	int cash_level = -1;
	int cash_discount = 100;
	
	QPDiscountInfoVector::const_iterator it;
	for (it = discount_.begin(); it != discount_.end(); ++it)
	{
		if (it->id == merchant_id)
		{
			for (unsigned int i = 0; i < it->discount.size(); i++)
			{
				const QPDiscountLevel& grade = it->discount[i];
				if (cash_level < grade.amount_begin && cash >= grade.amount_begin)
				{
					cash_level = grade.amount_begin;
					cash_discount = grade.discount;
				}
			}
			break;
		}
	}

	return (cash * cash_discount / 100 == cash_after_discount);
}

} // end namespace GNET
