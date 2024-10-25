#ifndef __GNET_PSHOP_MARKET_H__
#define __GNET_PSHOP_MARKET_H__

#include <itimer.h>
#include <pshopbase>
#include <pshopdetail>
#include <pshopentry>
#include <pshopitementry>

namespace GNET
{

class PShopDetail;
class PShopObj
{
	bool _busy;//取消出售或收购,玩家买卖时设置
	PShopDetail _detail;

public:
	friend class PShopMarket;
	PShopObj(const PShopDetail &shop):_busy(false)				{ _detail = shop; }
	operator PShopDetail &() 									{ return _detail; }
	operator const PShopDetail &() const 						{ return _detail; }
	inline char GetType() const 								{ return _detail.shoptype; }
	inline int GetRoleID() const  								{ return _detail.roleid; }
	inline int GetCreateTime() const 							{ return _detail.createtime; }
	inline int GetExpireTime() const 							{ return _detail.expiretime; }
	inline unsigned int GetMoney() const						{ return _detail.money; }
	inline const PShopItemVector & GetListBuy() const			{ return _detail.blist; } 
	inline const PShopItemVector & GetListSale() const			{ return _detail.slist; } 
	inline const GRoleInventoryVector &GetStore() const			{ return _detail.store; }
	inline void SetType(char type)								{ _detail.shoptype = type; }
	inline void SetStatus(int status)							{ _detail.status = status; }
	inline void SetExpireTime(int time)							{ _detail.expiretime = time; }
	inline void SetMoney(int money)								{ if(money < 0) return; _detail.money = money; }
	inline void SetStore(const GRoleInventoryVector & store)	{ _detail.store = store; }
	inline void AddItemBuy(const PShopItem & item)				{ _detail.blist.push_back(item); }
	inline void AddItemSale(const PShopItem & item)				{ _detail.slist.push_back(item); }
	inline void AddItemStore(const GRoleInventory & item)		{ _detail.store.push_back(item); }
	inline void ClearBuyList()									{ _detail.blist.clear(); }
	inline void ClearSaleList()									{ _detail.slist.clear(); }
	inline void ClearStore()									{ _detail.store.clear(); }
	inline bool IsBusy() const									{ return _busy; }
	inline void SetBusy(bool b)									{ _busy = b; }
	inline void SetYinPiao(const GRoleInventoryVector & yp)		{ _detail.yinpiao = yp; }
	unsigned int GetYinPiao() const;
	PShopBase GetShopBase() const;
	uint64_t GetTotalMoney() const;
	uint64_t GetTotalMoneyCap() const;
	const PShopItem * GetItemBuy(int pos) const;
	const PShopItem * GetItemSale(int pos) const;
	const GRoleInventory * GetItemStore(int pos) const;
	void RemoveItemBuy(int pos);
	void RemoveItemSale(int pos);
	void RemoveItemStore(int pos);
	void UpdateItemStore(const GRoleInventory & item);
	void Destroy();
	void Trace() const;
};

class PShopMarket : public IntervalTimer::Observer
{
	/*物品索引项*/
	struct IndexEntry
	{
		int ref;//引用计数(标记本店铺有多少栏位在出售或收购该物品)
		const PShopObj *p;
		IndexEntry():ref(0),p(0){}
		explicit IndexEntry(const PShopObj *ptr):ref(1),p(ptr){}
	};

public:
	typedef unsigned int									ROLEID;
	typedef std::pair<int/*时间*/,ROLEID>					TIME_PAIR;
	typedef std::map<ROLEID, PShopObj*>						PSHOP_MAP;
	typedef std::vector<std::vector<PShopObj*> >			PSHOP_POOL;
	typedef std::vector<IndexEntry>							INDEX_LIST;
	typedef std::map<int/*物品id*/, INDEX_LIST>				ITEM_MAP;
	typedef std::multimap<int/*时间*/, ROLEID>				TIME_MAP;
	struct find_param_t
	{
		char type;//查询物品类型(0:出售1:收购)
		int itemid;//查询物品ID
		int page_num;//请求第几页(从0页开始)
		find_param_t(char t,int id,int pn):type(t),itemid(id),page_num(pn){}
	};

	PShopMarket():_init(false),_hb_count(0) {}
	~PShopMarket() {__OnDestroy();}
	static PShopMarket &GetInstance() {static PShopMarket instance; return instance;}
	static void MoneyToYinPiao(uint64_t money, unsigned int &yp, unsigned int &remains);
	bool Initialize();
	void LoadFromDB(const PShopDetailVector &, bool);
	void AddShop(const PShopDetail & shop);
	void RemoveShop(int roleid);
	void OnAddItemBuy(int roleid, const PShopItem &item);
	void OnAddItemSale(int roleid, const PShopItem &item);
	void OnRemoveItemBuy(int roleid, int pos);
	void OnRemoveItemSale(int roleid, int pos);
	void OnRemoveListBuy(PShopObj *obj);
	void OnRemoveListSale(PShopObj *obj);
	void OnModifyType(int roleid, int newtype);
	void OnModifyExpireTime(int roleid, int time);
	virtual bool Update();
	void Trace() const;
	bool IsLoadComplete() const { return _init;}

	PShopObj* GetShop(ROLEID roleid);
	const PShopObj* GetShop(ROLEID roleid) const {return GetShop(roleid);}
	bool GetFromTimeMap(ROLEID roleid) const;
	void ListShops(char type, PShopEntryVector &list) const;
	void ListItems(const find_param_t &param, PShopItemEntryVector &list, int &pagenum) const;

private:
	void __OnDestroy();
	void __OnAddItem(const PShopObj *obj, const PShopItem &, ITEM_MAP &);
	void __OnRemoveItem(const PShopObj *obj, const PShopItem &, ITEM_MAP &);
	void __RemoveShop(ROLEID roleid);
	void __RemoveFromTimeMap(ROLEID roleid);
	void __RemoveFromTimeMap(ROLEID roleid, int time);

private:
	PSHOP_MAP	_shopmap;		//店铺列表
	PSHOP_POOL	_shoppool;		//店铺类型索引表
	ITEM_MAP	_buymap;		//收购物品索引表
	ITEM_MAP	_salemap;		//出售物品索引表
	TIME_MAP	_timemap;		//过期时间索引表
	bool		_init;			//DB加载完成
	int			_hb_count;		//心跳计数
};

enum PLAYERSHOP_INDEX
{
	PSHOP_INDEX_MIN					= 0,
	PSHOP_INDEX_VEHICLE				= 0,//交通工具
	PSHOP_INDEX_TREASURECHESTS		= 1,//宝箱
	PSHOP_INDEX_MEDICAL				= 2,//药品
	PSHOP_INDEX_DRESS				= 3,//时装
	PSHOP_INDEX_EQUIPMENT			= 4,//装备
	PSHOP_INDEX_STRENGTHENITEM		= 5,//强化道具
	PSHOP_INDEX_MATERIAL			= 6,//材料
	PSHOP_INDEX_GROCERIES			= 7,//杂货
	PSHOP_INDEX_MAX,
};

enum PLAYERSHOP_MASK
{
	PSHOP_MASK_VEHICLE				= 0x01,
	PSHOP_MASK_TREASURECHESTS		= 0x02,
	PSHOP_MASK_MEDICAL				= 0x04,
	PSHOP_MASK_DRESS				= 0x08,
	PSHOP_MASK_EQUIPMENT			= 0x10,
	PSHOP_MASK_STRENGTHENITEM		= 0x20,
	PSHOP_MASK_MATERIAL			= 0x40,
	PSHOP_MASK_GROCERIES			= 0x80,
	PSHOP_MASK_ALL=0xFF,
};

};
#endif
