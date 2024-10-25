#ifndef __ONLINE_QGAME_SHOPPING_MALL_H__
#define __ONLINE_QGAME_SHOPPING_MALL_H__

#include <hashmap.h>
#include <amemory.h>
#include <amemobj.h>

class itemdataman;
typedef struct _MALL_ITEM_SERV MALL_ITEM_SERV;
namespace netgame{

class mall
{
public:
	enum
	{
		MAX_ENTRY = 4,
		MAX_OWNER = 8,
	};

	class sale_time	//lgc
	{
	public:
		enum{
			TYPE_NOLIMIT = 0,	//����ʱ������:����
			TYPE_INTERZONE,		//���䣬����1:��ʼ(0��[-inf,p2])����2:����(0��[p1,inf])����������ȫΪ0
			TYPE_WEEK,			//ÿ�ܣ�����1 bit:0-6 ����-���� ����2��ʹ��
			TYPE_MONTH,			//ÿ�£�����1 bit:1-31 1-31�� ����2��ʹ��
		};
		sale_time():type(0),param1(0),param2(0){}
		void SetParam(int t, int p1, int p2){type = t, param1 = p1, param2 = p2;}
		bool CheckAvailable(time_t t) const;
		int GetType() const{return type;}
		int GetParam1() const{return param1;}
		int GetParam2()const{return param2;}
		//���sale_time�����Ƿ���Ч
		static bool CheckParam(int type, int param1, int param2);
	private:
		int type;
		int param1;
		int param2;
	};

	struct node_t
	{
		int goods_id;
		int goods_count;

		bool group_active;		//�Ƿ���ڷ�Ĭ����id
		bool sale_time_active;	//�Ƿ���ڷ���������ʱ��

		struct
		{
			int group_id;		//���ڶ���Чʱ���������ƣ�lgc
			sale_time _sale_time;	//����ʱ��
			int status;			//��Ʒ����,��Ʒ...
			int expire_time;
			int expire_type;	//���expire_time��Ϊ0�� ����Ŀ��Ч
			int cash_need;
		} entry[MAX_ENTRY];
		
		int gift_id;
		int gift_count;
		int gift_expire_time;
		int gift_log_price;
		int spec_owner[MAX_OWNER];
		
		
		bool check_owner(int tid)
		{
			if(!spec_owner[0]) return true; // no spec
			if(tid == 0) return false; // no owner
			for(int n = 0; n < MAX_OWNER; ++n)
			{
				if(spec_owner[n] == tid) return true;
			}
			return false;
		}
	};
	
	struct index_node_t				//lgc�����ڱ����̳��п��ܷ����仯��Ʒ�����ݽṹ
	{
		int _index;		
		node_t _node;
		index_node_t(const node_t& node, int index):_index(index),_node(node){}
	};
	
private:
	//typedef abase::hash_map<int, node_t>  MAP;
	typedef abase::vector<node_t>  MAP;
	MAP _map;
	int _lock;

	typedef abase::vector<index_node_t, abase::fast_alloc<> > LIMIT_GOODS;	//lgc
	LIMIT_GOODS _limit_goods;
	
	int _group_id;
	int _group_id_lock;	
public:
	enum
	{
		STATUS_NONE = 0,	
		STATUS_NEWPRODUCT,
		STATUS_PROMOTION,
		STATUS_RECOMMENDED,
		STATUS_10_PERCENT_PRICE,
		STATUS_20_PERCENT_PRICE,
		STATUS_30_PERCENT_PRICE,
		STATUS_40_PERCENT_PRICE,
		STATUS_50_PERCENT_PRICE,
		STATUS_60_PERCENT_PRICE,
		STATUS_70_PERCENT_PRICE,
		STATUS_80_PERCENT_PRICE,
		STATUS_90_PERCENT_PRICE,
	};

	enum
	{
		COIN_RESERVED_MONEY 		= -1123,
		COIN_RESERVED_RANK_POINT 	= -1124,
	};

	enum
	{
		EXPIRE_TYPE_TIME	= 0,	//��ʱ��������
		EXPIRE_TYPE_DATE	= 1,	//��ʱ��������
		
	};

	bool CheckReservedCoin(int coin)
	{
		return coin != COIN_RESERVED_MONEY && coin != COIN_RESERVED_RANK_POINT;
	}
	
public:
	mall()
	{
		_lock = 0;
		_group_id = 0;
		_group_id_lock = 0;
	}
	
	bool AddGoods(const node_t & node);
	bool QueryGoods(size_t index, node_t & n);
	//size_t QueryGoods(const size_t * index, node_t * n, size_t count);
	size_t GetGoodsCount();

	int GetGroupId();	//lgc
	void SetGroupId(int id);
	LIMIT_GOODS & GetLimitGoods(){return _limit_goods;}
	bool AddLimitGoods(const node_t & node, int index);
};

class mall_order : public abase::ASmallObject
{
	struct entry_t
	{
		int  item_id;
		int  item_count;
		int  cash_need;
		int  expire_time;
		int expire_type;
		int gift_id;
		int gift_count;
		int gift_expire_time;
		int gift_log_price;
		entry_t(int id, int count, int cash, int expire_time,int expire_type,int _gift_id, int _gift_count, int _gift_expire_time, int _gift_log_price)
			:item_id(id),item_count(count),cash_need(cash),expire_time(expire_time),expire_type(expire_type),gift_id(_gift_id),gift_count(_gift_count),gift_expire_time(_gift_expire_time),gift_log_price(_gift_log_price)
		{}
	};

	typedef abase::vector<entry_t,abase::fast_alloc<> > LIST;

	LIST _list;
	int _total_point;
	int _order_id;
public:

	mall_order():_total_point(0),_order_id(-1) {}
	mall_order(int order_id):_total_point(0),_order_id(order_id) {}

	void AddGoods(int id, int count,int point, int expire_time,int expire_type, int _gift_id, int _gift_count, int _gift_expire_time, int _gift_log_price)
	{
		_list.push_back(entry_t(id,count,point,expire_time,expire_type,_gift_id,_gift_count,_gift_expire_time,_gift_log_price));
		_total_point += point;
	}

	int GetPointRequire()
	{
		return _total_point;
	}

	bool GetGoods(size_t index, int & id, int & count, int & point, int &expire_time, int &expire_type, int& _gift_id, int& _gift_count, int& _gift_expire_time, int & _gift_log_price)
	{
		if(index >= _list.size()) return false;
		
		id = _list[index].item_id;
		count = _list[index].item_count;
		point = _list[index].cash_need;
		expire_time = _list[index].expire_time;
		expire_type = _list[index].expire_type;
		_gift_id = _list[index].gift_id;
		_gift_count = _list[index].gift_count;
		_gift_expire_time = _list[index].gift_expire_time;
		_gift_log_price = _list[index].gift_log_price;
		return true;
	}
	
};

bool InitMall(netgame::mall & __mall, itemdataman & dataman, const abase::vector<MALL_ITEM_SERV> & __list);

struct mall_invoice
{
	int order_id;           //���ν��׵���ˮ��
	int item_id;            //�������ƷID
	int item_count;         //ÿ����Ʒ������
	int price;              //����
	int expire_date;        //����ʱ��
	int guid1;		//������Ʒ��GUID
	int guid2;		//������Ʒ��GUID
	int timestamp;		

	mall_invoice(int order_id, int item_id, int item_count, int price, int expire_date,int ts,int guid1,int guid2)
		:order_id(order_id),item_id(item_id), item_count(item_count), price(price), 
			expire_date(expire_date),guid1(guid1),guid2(guid2),timestamp(ts)
		{
		}
};

class touchshop
{
	struct entry_t
	{
		int id;
		unsigned int num;
		unsigned int price;
		int expire;

		entry_t(int _id = 0, unsigned int _num = 0, unsigned int _price = 0, int _expire = 0)
			: id (_id), num(_num), price(_price), expire(_expire) {}
	};
	typedef abase::vector<entry_t,abase::fast_alloc<> > LIST;
public:
	void AddGoods(int id,unsigned int num, unsigned int price, int expire)
	{
		_list.push_back(entry_t(id,num,price,expire));
	}	
	bool GetGoods(size_t index,int & id,unsigned int & num,unsigned int & price,int& expire)
	{
		if(index >= _list.size()) return false;
		
		entry_t& node = _list[index]; 

		id = node.id;
		num = node.num;
		price = node.price;
		expire = node.expire;
	
		return true;
	}

	bool CheckGoods(size_t index,int id ,unsigned int num, unsigned int price,int expire)
	{
		if(index >= _list.size()) return false;
	
		entry_t& node = _list[index]; 
		
		return id == node.id && num == node.num && price == node.price && expire == node.expire;		
	}

private:
	LIST _list;

};
bool InitTouchShop(netgame::touchshop & __shop, itemdataman & dataman);

}
#endif

