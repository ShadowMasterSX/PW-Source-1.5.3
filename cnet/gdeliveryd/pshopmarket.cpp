#include <itimer.h>
#include <localmacro.h>
#include "pshopmarket.h"
#include "gamedbclient.hpp"
#include "dbpshoptimeout.hrp"

namespace GNET
{

/*******************************************************************************
***						PShopObj											****
********************************************************************************/
PShopBase PShopObj::GetShopBase() const
{
	PShopBase base;
	base.roleid = _detail.roleid;
	base.shoptype = _detail.shoptype;
	base.blist = _detail.blist;
	base.slist = _detail.slist;
	return base;
}

unsigned int PShopObj::GetYinPiao() const
{
	unsigned int yp=0;
	for(size_t i=0; i<_detail.yinpiao.size(); ++i)
	yp += _detail.yinpiao[i].count;
	return yp;
}

uint64_t PShopObj::GetTotalMoney() const
{
	uint64_t total_money = _detail.money;
	const GRoleInventoryVector &yp = _detail.yinpiao;
	for(size_t i=0; i<yp.size(); ++i)
		total_money += ((uint64_t)yp[i].count * (uint64_t)WANMEI_YINPIAO_PRICE);
	return total_money;
}

uint64_t PShopObj::GetTotalMoneyCap() const
{
	return /*(uint64_t)PSHOP_MONEY_CAP +*/ (uint64_t)PSHOP_YINPIAO_CAP * (uint64_t)WANMEI_YINPIAO_PRICE; // 由于银票转换原因，最终上限受制于银票上限
}

struct CmpPos
{
	int pos;
	CmpPos(int idx):pos(idx) {}
	bool operator()(const GRoleInventory &item) { return item.pos == pos; }
};
struct CmpPos2
{
	int pos;
	CmpPos2(int idx):pos(idx) {}
	bool operator()(const PShopItem &item) { return item.item.pos == pos; }
};
void PShopObj::RemoveItemBuy(int pos)
{
	_detail.blist.erase(std::remove_if(_detail.blist.begin(), _detail.blist.end(), CmpPos2(pos)), _detail.blist.end());
}
void PShopObj::RemoveItemSale(int pos)
{
	_detail.slist.erase(std::remove_if(_detail.slist.begin(), _detail.slist.end(), CmpPos2(pos)), _detail.slist.end());
}
void PShopObj::RemoveItemStore(int pos)
{
	_detail.store.erase(std::remove_if(_detail.store.begin(), _detail.store.end(), CmpPos(pos)), _detail.store.end());
}
const PShopItem * PShopObj::GetItemBuy(int pos) const
{
	PShopItemVector::const_iterator it = std::find_if(_detail.blist.begin(), _detail.blist.end(), CmpPos2(pos));
	return (it != _detail.blist.end()) ? &(*it) : NULL;
}
const PShopItem * PShopObj::GetItemSale(int pos) const
{
	PShopItemVector::const_iterator it = std::find_if(_detail.slist.begin(), _detail.slist.end(), CmpPos2(pos));
	return (it != _detail.slist.end()) ? &(*it) : NULL;
}
const GRoleInventory * PShopObj::GetItemStore(int pos) const
{
	GRoleInventoryVector::const_iterator it = std::find_if(_detail.store.begin(), _detail.store.end(), CmpPos(pos));
	return (it != _detail.store.end()) ? &(*it) : NULL;
}

void PShopObj::UpdateItemStore(const GRoleInventory & item)
{
	for(size_t i = 0; i < _detail.store.size(); ++i)
	{
		GRoleInventory& inv = _detail.store[i];
		if(inv.pos == item.pos)
		{
			inv = item;
			return;
		}
	}

	_detail.store.push_back(item);
}

void PShopObj::Destroy()
{
	_detail.money = 0;
	_detail.yinpiao.clear();
	_detail.blist.clear();
	_detail.slist.clear();
	_detail.store.clear();
	delete this;
}

void PShopObj::Trace() const
{
#ifdef USE_TRACE
	fprintf(stdout, "**************************************************************\n");
	fprintf(stdout, "roleid(%d),type(%d),status(%d),expiretime:%s\n", _detail.roleid,_detail.shoptype,_detail.status,ctime((time_t *)&(_detail.expiretime)));
	fprintf(stdout, "money(%d),yinpiao(%d)\n", _detail.money, GetYinPiao());
	fprintf(stdout, "收购栏:\n\tid\tpos\tcount\tmax_count\tprice\n");
	for(size_t i=0; i<_detail.blist.size(); ++i)
	{
		const GRoleInventory &item = _detail.blist[i].item;
		fprintf(stdout, "\t%d\t%d\t%d\t%d\t\t%d\n", item.id,item.pos,item.count,item.max_count,_detail.blist[i].price);
	}
	fprintf(stdout, "出售栏:\n\tid\tpos\tcount\tmax_count\tprice\n");
	for(size_t i=0; i<_detail.slist.size(); ++i)
	{
		const GRoleInventory &item = _detail.slist[i].item;
		fprintf(stdout, "\t%d\t%d\t%d\t%d\t\t%d\n", item.id,item.pos,item.count,item.max_count,_detail.slist[i].price);
	}
	fprintf(stdout, "店铺仓库:\n\tid\tpos\tcount\tmax_count\n");
	for(size_t i=0; i<_detail.store.size(); ++i)
	{
		const GRoleInventory &item = _detail.store[i];
		fprintf(stdout, "\t%d\t%d\t%d\t%d\n", item.id,item.pos,item.count,item.max_count);
	}
	fprintf(stdout, "**************************************************************\n");
#endif
}

/*******************************************************************************
***						PShopMarket::Public Function						****
********************************************************************************/
bool PShopMarket::Initialize()
{
	_shoppool.resize(PSHOP_INDEX_MAX);
	IntervalTimer::Attach(this,500000/IntervalTimer::Resolution());
	return true;
}

void PShopMarket::LoadFromDB(const PShopDetailVector & list, bool finish)
{
	/*
	 * DELIVERY启动时加载店铺列表
	 * 过滤过期店铺,过期店铺只加载过期时间
	 */

	if(_init) __OnDestroy();
	for(size_t i=0; i < list.size(); ++i)
	{
		const PShopDetail & shop = list[i];

		if(shop.status == PSHOP_STATUS_NORMAL)
			AddShop(shop); 
		else if(shop.status == PSHOP_STATUS_EXPIRED)
			_timemap.insert(TIME_MAP::value_type(shop.expiretime, shop.roleid));
		else
			Log::log(LOG_ERR,"LoadFromDB,Invalid status, roleid=%d,status=%d\n", shop.roleid,shop.status);
	}

	if(finish)
	{
		_init = true;
		Trace();
	}
}

void PShopMarket::AddShop(const PShopDetail & shop)
{
	if(shop.status != PSHOP_STATUS_NORMAL)
	{
		return;
	}

	//删除缓存数据
	int roleid = shop.roleid;
	if(GetShop(roleid))
	{
		__RemoveShop(roleid);
	}
	else if(GetFromTimeMap(roleid))
	{
		//有可能缓存了过期时间
		//所有在添加前删除过期时间
		__RemoveFromTimeMap(roleid);
	}

	//加入新店铺
	PShopObj * obj = new PShopObj(shop);
	_shopmap[roleid] = obj;
	_shoppool[shop.shoptype].push_back(obj);
	_timemap.insert(TIME_MAP::value_type(shop.expiretime, roleid));

	//同步物品索引表
	for(size_t i=0; i<shop.blist.size(); ++i)
		__OnAddItem(obj, shop.blist[i], _buymap);
	for(size_t i=0; i<shop.slist.size(); ++i)
		__OnAddItem(obj, shop.slist[i], _salemap);

	obj->Trace();
}

void PShopMarket::RemoveShop(int roleid)
{
	__RemoveShop(roleid);
}

void PShopMarket::OnAddItemBuy(int roleid, const PShopItem &item)
{
	if(item.item.count <= 0) return;
	PShopObj *obj = GetShop(roleid);
	if(obj)
	{
		__OnAddItem(obj, item, _buymap);
		obj->AddItemBuy(item);
	}
}

void PShopMarket::OnAddItemSale(int roleid, const PShopItem &item)
{
	if(item.item.count <= 0) return;
	PShopObj *obj = GetShop(roleid);
	if(obj)
	{
		__OnAddItem(obj, item, _salemap);
		obj->AddItemSale(item);
	}
}

void PShopMarket::OnRemoveItemBuy(int roleid, int pos)
{
	PShopObj *obj = GetShop(roleid);
	if(obj)
	{
		const PShopItem *pItem = obj->GetItemBuy(pos);
		if(pItem)
		{
			__OnRemoveItem(obj, *pItem, _buymap);
			obj->RemoveItemBuy(pos);
		}
	}
}

void PShopMarket::OnRemoveItemSale(int roleid, int pos)
{
	PShopObj *obj = GetShop(roleid);
	if(obj)
	{
		const PShopItem *pItem = obj->GetItemSale(pos);
		if(pItem)
		{
			__OnRemoveItem(obj, *pItem, _salemap);
			obj->RemoveItemSale(pos);
		}
	}
}

void PShopMarket::OnRemoveListBuy(PShopObj *obj)
{
	//店铺收购列表被删除
	//同步收购物品索引表
	const PShopItemVector & blist = obj->GetListBuy();
	PShopItemVector::const_iterator it = blist.begin();
	for(; it != blist.end(); ++it)
	{
		__OnRemoveItem(obj, *it, _buymap);
	}
	obj->ClearBuyList();
}

void PShopMarket::OnRemoveListSale(PShopObj *obj)
{
	//店铺待售列表被删除
	//同步待售物品索引表
	const PShopItemVector & slist = obj->GetListSale();
	PShopItemVector::const_iterator it = slist.begin();
	for(; it != slist.end(); ++it)
	{
		__OnRemoveItem(obj, *it, _salemap);
	}
	obj->ClearSaleList();
}

void PShopMarket::OnModifyType(int roleid, int newtype)
{
	PShopObj * obj = GetShop(roleid);
	if(obj)
	{
		int oldtype = obj->GetType();
		std::vector<PShopObj*> &list = _shoppool[oldtype];
		list.erase(std::remove(list.begin(), list.end(), obj), list.end());

		_shoppool[newtype].push_back(obj);
		obj->SetType(newtype);
	}
}

void PShopMarket::OnModifyExpireTime(int roleid, int expiretime)
{
	PShopObj * obj = GetShop(roleid);
	if(obj)
	{
		__RemoveFromTimeMap(roleid, obj->GetExpireTime());
		_timemap.insert(TIME_MAP::value_type(expiretime,roleid));

		obj->SetStatus(PSHOP_STATUS_NORMAL);
		obj->SetExpireTime(expiretime);
	}
}

PShopObj* PShopMarket::GetShop(ROLEID id)
{
	if(id <= 0) return NULL;
	
	PSHOP_MAP::iterator it = _shopmap.find(id);
	return it != _shopmap.end() ? it->second : NULL;
}

bool PShopMarket::GetFromTimeMap(ROLEID roleid) const
{
	if(!_init) return false;

	TIME_MAP::const_iterator it = _timemap.begin();
	for(; it != _timemap.end(); ++it)
	{
		if(it->second == (unsigned int)roleid)
		{
			return true;
		}
	}
	return false;
}

void PShopMarket::ListShops(char type, PShopEntryVector &list) const
{
	if(!_init) return;
	if(type < PSHOP_INDEX_MIN || type >= PSHOP_INDEX_MAX) return;
	const std::vector<PShopObj*> &shops = _shoppool[type];
	for(size_t i=0; i<shops.size(); ++i)
	{
		const PShopObj *obj = shops[i];
		int invstate = 0;//栏位状态(标记栏位空闲情况)
		if(obj->GetListSale().size() > 0)
			invstate |= 0x01;
		if(obj->GetListBuy().size() > 0)
			invstate |= 0x02;

		//过滤出售和收购栏均为空的店铺
		if(invstate) list.push_back(PShopEntry(obj->GetRoleID(), obj->GetType(), obj->GetCreateTime(), invstate));
	}
}

void PShopMarket::ListItems(const find_param_t &param, PShopItemEntryVector &list, int &pagenum) const
{
	pagenum = 0;
	list.clear();
	if(!_init) return;
	if((param.type != 0 && param.type != 1) || param.itemid <= 0 || param.page_num < 0) return;

	const ITEM_MAP &map = (param.type==0) ? _salemap : _buymap;
	ITEM_MAP::const_iterator it = map.find(param.itemid);
	if(it == map.end()) return;

	unsigned int total = 0;//匹配物品在架总数
	const INDEX_LIST &idx_list = it->second;
	for(size_t i=0; i < idx_list.size(); ++i)
		total += idx_list[i].ref;

	if(total < 1) return;

	//验证请求页面有效性
	//无效则重置请求页面为最后一页
	//请求页面从0开始编号
	if(total <= param.page_num * PSHOP_PAGE_SIZE)
		pagenum = (total - 1) / PSHOP_PAGE_SIZE; //整号退1
	else
		pagenum = param.page_num;
	
	//将pagenum页面物品放入list中
	unsigned int pass  = 0;	//跳过计数
	unsigned int count = 0;	//pagenum页面物品计数
	const unsigned int total_pass = pagenum * PSHOP_PAGE_SIZE ; //到达页面所需跳过数
	for(size_t i=0; i < idx_list.size() && count < PSHOP_PAGE_SIZE; ++i)
	{
		if((pass + idx_list[i].ref) <= total_pass) // 加快跳过不需要的页
		{
			pass += idx_list[i].ref;
			continue;
		}
		
		const PShopObj *obj = idx_list[i].p;
		const PShopItemVector &items = (param.type==0) ? obj->GetListSale(): obj->GetListBuy();
		for(size_t j=0; j < items.size() && count < PSHOP_PAGE_SIZE; ++j)
		{
			if(items[j].item.id == (unsigned int)param.itemid && ++pass > total_pass)
			{
				++count;
				list.push_back(PShopItemEntry(obj->GetRoleID(), items[j]));
			}
		}
	}
}

bool PShopMarket::Update()
{
	if(!_init)
		return true;
	
	if(++_hb_count < 20)
		return true;

	struct timeval now;
	IntervalTimer::GetTime(&now);

	//扫描已过期的店铺
	std::vector<TIME_PAIR> listdel;
	TIME_MAP::iterator it = _timemap.begin();
	for(; it != _timemap.end(); ++it)
	{
		if(it->first > now.tv_sec) break;
		listdel.push_back(TIME_PAIR(*it));
	}

	//统一删除过期店铺
	for(size_t i=0; i<listdel.size(); ++i)
	{
		int time = listdel[i].first;
		int roleid = listdel[i].second;
		char delflag = 0;
		PShopObj *obj = GetShop(roleid);
		if(obj)
		{
			//店铺首次过期
			__RemoveShop(roleid);
			_timemap.insert(TIME_MAP::value_type(time+PSHOP_DELETE_DELAY, roleid));
		}
		else
		{
			//店铺真正过期
			__RemoveFromTimeMap(roleid, time);
			delflag = 1;
		}

		DBPShopTimeout *rpc = (DBPShopTimeout *)Rpc::Call(RPC_DBPSHOPTIMEOUT, DBPShopTimeoutArg(roleid,delflag));
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	_hb_count = 0;
	return true;
}

void PShopMarket::Trace() const
{
#ifdef USE_TRACE
	fprintf(stdout, "**************************************************************\n");
	fprintf(stdout, "当前有效店铺个数: %d\n", _shopmap.size());
	fprintf(stdout, "店主ROLEID分别为:");
	for(PSHOP_MAP::const_iterator it=_shopmap.begin(); it!=_shopmap.end(); ++it)
		fprintf(stdout, "%d\t", it->first);

	fprintf(stdout, "\n店铺类型索引表:\n");
	for(size_t i=0; i<PSHOP_INDEX_MAX; ++i)
	{
		fprintf(stdout, "\t\t类型%d的店铺: ", i);
		for(size_t j=0; j<_shoppool[i].size(); ++j)
			fprintf(stdout, "%d,", _shoppool[i][j]->GetRoleID());
		fprintf(stdout, "\n");
	}

	fprintf(stdout, "\n收购物品索引表:物品种类(%d)\n", _buymap.size());
	for(ITEM_MAP::const_iterator it=_buymap.begin(); it!=_buymap.end(); ++it)
	{
		fprintf(stdout, "\t\t物品ID(%d): %d人在收购.\n", it->first, it->second.size());
		for(size_t i=0; i<it->second.size(); ++i)
			fprintf(stdout, "\t\t\t收购人%d: %d个栏位在收购.\n", it->second[i].p->GetRoleID(), it->second[i].ref);
		fprintf(stdout, "\n");
	}

	fprintf(stdout, "\n出售物品索引表:物品种类(%d)\n", _salemap.size());
	for(ITEM_MAP::const_iterator it=_salemap.begin(); it!=_salemap.end(); ++it)
	{
		fprintf(stdout, "\t\t物品ID(%d): %d人在出售.\n", it->first, it->second.size());
		for(size_t i=0; i<it->second.size(); ++i)
			fprintf(stdout, "\t\t\t出售人%d: %d个栏位在出售.\n", it->second[i].p->GetRoleID(), it->second[i].ref);
		fprintf(stdout, "\n");
	}

	fprintf(stdout, "**************************************************************\n");
#endif
}

/*******************************************************************************
***						PShopMarket::Private Function						****
********************************************************************************/
void PShopMarket::__OnDestroy()
{
	for(size_t i=0; i<_shoppool.size(); ++i)
	{
		_shoppool[i].clear();
	}
	PSHOP_MAP::iterator itb = _shopmap.begin();
	PSHOP_MAP::iterator ite = _shopmap.end();
	while(itb != ite)
	{
		itb->second->Destroy();
		++itb;
	}

	_buymap.clear();
	_salemap.clear();
	_timemap.clear();
	_shopmap.clear();
	_init = 0;
	_hb_count = 0;
}

void PShopMarket::__OnAddItem(const PShopObj *obj, const PShopItem &item, ITEM_MAP &map)
{
	//添加物品,同步物品索引表
	//本函数只同步索引表,不添加物品到OBJ中
	if(!obj) return;
	if(item.item.id <= 0 && item.item.pos < 0 && item.item.count <= 0) return;
	INDEX_LIST &list = map[item.item.id];
	size_t i;
	for(i=0; i<list.size(); ++i)
	{
		if(list[i].p == obj)
		{
			list[i].ref++;
			break;
		}
	}
	if(i == list.size())
	{
		list.push_back(IndexEntry(obj));
	}
}

void PShopMarket::__OnRemoveItem(const PShopObj *obj, const PShopItem &item, ITEM_MAP &map)
{
	//删除物品,同步物品索引表
	//本函数只同步索引表,不从OBJ中删除物品
	if(!obj) return;
	if(item.item.id <= 0 && item.item.pos < 0 && item.item.count <= 0) return;
	bool delsucc = false;//标记删除成功
	INDEX_LIST &list = map[item.item.id];
	for(size_t i=0; i<list.size(); ++i)
	{
		if(list[i].p == obj)
		{
			if(--list[i].ref == 0)
			{
				list.erase(list.begin() + i);
				//无人出售或收购该物品,删除索引项
				if(list.empty()) map.erase(item.item.id);
			}
			delsucc = true;
			break;
		}
	}
	if(!delsucc)
	{
		Log::log(LOG_ERR,"PShopMarket::__OnRemoveItem: 同步物品索引表失败.roleid=%d,itemid=%d.\n", obj->GetRoleID(), item.item.id);
	}
}

void PShopMarket::__RemoveShop(ROLEID roleid)
{
	PShopObj *obj = GetShop(roleid);
	if(obj)
	{
		_shopmap.erase(roleid);
		std::vector<PShopObj*> &list = _shoppool[obj->GetType()];
		list.erase(std::remove(list.begin(), list.end(), obj), list.end());
		OnRemoveListBuy(obj);
		OnRemoveListSale(obj);
		__RemoveFromTimeMap(roleid, obj->GetExpireTime());
		obj->Destroy();
	}
}

void PShopMarket::__RemoveFromTimeMap(ROLEID roleid)
{
	//删除指定玩家的店铺
	//效率较低,逐个遍历
	TIME_MAP::iterator it = _timemap.begin();
	for(; it != _timemap.end(); ++it)
	{
		if(it->second == (unsigned int)roleid)
		{
			_timemap.erase(it);
			break;
		}
	}

	if(it == _timemap.end())
	{
		Log::log(LOG_ERR,"__RemoveFromTimeMap: 删除过期时间失败.roleid=%d.\n", roleid);
	}
}

void PShopMarket::__RemoveFromTimeMap(ROLEID roleid, int time)
{
	//删除指定店铺和时间的过期时间
	//相对第一种删除方式高效很多
	std::pair<TIME_MAP::iterator, TIME_MAP::iterator> range;
	range = _timemap.equal_range(time);
	TIME_MAP::iterator it = range.first;
	TIME_MAP::iterator ie = range.second;
	for(; it!=ie; ++it)
	{   
		if(it->second == roleid)
		{   
			_timemap.erase(it);
			break;
		}   
	}

	if(it == ie)
	{
		Log::log(LOG_ERR,"__RemoveFromTimeMap: 删除过期时间失败.roleid=%d,time=%d.\n", roleid,time);
	}
}

/*
 * 函数功能: 金钱转换为银票
 * @param: money: 需要兑换的金钱数
 * @param: yinpiao: 兑换得到的银票
 * @param: remains: 剩余金钱数
 */
void PShopMarket::MoneyToYinPiao(uint64_t money, unsigned int &yinpiao, unsigned int &remains)
{
	yinpiao = 0;
	remains = 0;
	while(money >= WANMEI_YINPIAO_PRICE)
	{
		yinpiao++;
		money -= WANMEI_YINPIAO_PRICE;
	}
	remains = money;
}

};

