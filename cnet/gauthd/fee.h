#ifndef __GNET_FEE_H
#define __GNET_FEE_H
namespace GNET
{
class Fee
{
	int fee;
public:
	enum { FEE_TIME,FEE_OCTETS,FEE_MONTH,FEE_SEASON,FEE_BONUS }	;
	Fee():fee(0) {}
	~Fee() {}
	
};

};
#endif
