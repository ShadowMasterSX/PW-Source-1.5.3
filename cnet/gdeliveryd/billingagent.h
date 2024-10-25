#ifndef __GNET_BILLINGAGENT_H
#define __GNET_BILLINGAGENT_H

#include <map>
#include <ext/hash_map>
#include "octets.h"
#include "hashstring.h"
namespace GNET
{
struct ShopItem
{
	int id;
	int number;
	int timeout;
	int amount;
	bool operator <  (const ShopItem& r)const
	{ 
		return (id<r.id || (id==r.id && number<r.number) || (id==r.id&&number==r.number&&timeout<r.timeout));
	}
	ShopItem () : id(0), number(0), timeout(0), amount(0) {}
	ShopItem (int i,int n, int t) : id(i), number(n), timeout(t), amount(0) {}
	ShopItem (const ShopItem &x) : id(x.id), number(x.number), timeout(x.timeout) {}
	ShopItem& operator = (const ShopItem&x)
	{
		id      = x.id;
		number  = x.number;
		timeout = x.timeout;
		return *this;
	}
};
struct MenuItem
{
	Octets menuid;
	Octets url;
	MenuItem (){}
	MenuItem (Octets& i,Octets& u) : menuid(i), url(u) {}
	MenuItem (const MenuItem &x) : menuid(x.menuid), url(x.url) {}
	MenuItem& operator = (const MenuItem&x)
	{
		menuid = x.menuid;
		url    = x.url;
		return *this;
	}
};
class BillingAgent
{
public:
	typedef std::map<ShopItem,MenuItem> MenuidMap;
	typedef __gnu_cxx::hash_map<Octets,ShopItem> ItemidMap;

	bool activated;

	MenuidMap menuids;
	ItemidMap itemids;

	BillingAgent() : activated(0) { }
	static BillingAgent& Instance() { static BillingAgent instance; return instance; }
	bool Initialize();
	bool Itemid2Menuid(int id, int number, int timeout, Octets& menuid, Octets& url);
	bool Menuid2Itemid(const Octets& menuid, int& id, int& number, int& timeout);
};

}
#endif
