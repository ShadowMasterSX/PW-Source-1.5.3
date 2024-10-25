#ifndef __UTIL_FUNCTION_H
#define __UTIL_FUNCTION_H

#include "gmailheader"
#include "gmail"
#include "gmailbox"
#include "gfactioninfo"
#include "gamedbmanager.h"
#include "pshopdetail"
#include "greincarnationdata"

namespace GNET
{
void PutSyslog(StorageEnv::Storage *plog,StorageEnv::Transaction& txn,int roleid,int ip, GRoleInventory& inv);
void PutSyslog(StorageEnv::Storage *plog,StorageEnv::Transaction& txn,int roleid,int ip, int money, GRoleInventoryVector& invs);

class MailSender
{
public:
	static unsigned char NextId(GMailBox& box)
	{
		int size = box.mails.size();
		unsigned char id = 0;
		if(size) 
			id = box.mails[size-1].header.id+1;
		for(GMailVector::iterator it=box.mails.begin(),ie=box.mails.end();it!=ie;it++)
		{
			if(it->header.id==id)
			{
				id++;
				it = box.mails.begin();
			}
		}
		return id;
	}

	static int Send(StorageEnv::Storage * pmailbox,GMailHeader& header,GRoleInventory& inv, unsigned int money, 
		StorageEnv::Transaction& txn)
	{
		Marshal::OctetsStream key, value;
		GMailBox box;
		time_t now = Timer::GetTime();
		GMail mail;
		mail.attach_money = 0;
		mail.attach_obj = inv;
		key << header.receiver;
		if(pmailbox->find(key,value,txn))
			value >> box;
		else
			box.timestamp = now;
		if(box.mails.size()>100)
		{
			Log::log( LOG_ERR, 
				"Send mail failed:src=%d:dst=%d:mid=%d:size=%d:money=%d:item=%d:count=%d:pos=%d",
			   	header.sender, header.receiver, header.id, 0, 0, mail.attach_obj.id, 
				mail.attach_obj.count, 0);
			return -1;
		}
		header.id = NextId(box);
		header.attribute = (1<<_MA_UNREAD); 
		header.send_time = now;
		if(inv.count>0)
		{
			mail.attach_obj = inv;
			header.attribute ^= (1<<_MA_ATTACH_OBJ);
		}
		if(money>0)
		{
			mail.attach_money = money;
			header.attribute ^= (1<<_MA_ATTACH_MONEY);
		}
						
		Log::formatlog("mail","type=send:src=%d:dst=%d:mid=%d:size=%d:money=%d:item=%d:count=%d:pos=%d",
			   	header.sender, header.receiver, header.id, 0, 0, mail.attach_obj.id, 
				mail.attach_obj.count, 0);

		mail.header = header;
		mail.header.receiver = now;
		box.mails.push_back(mail);
		pmailbox->insert( key, Marshal::OctetsStream()<<box, txn );
		return 0;
	}

	static int Send(StorageEnv::Storage * pmailbox, GMailHeader& header,unsigned int money, StorageEnv::Transaction& txn)
	{
		Marshal::OctetsStream key, value;
		GMailBox box;
		time_t now = Timer::GetTime();
		GMail mail;
		mail.attach_money = money;
		mail.attach_obj.id = 0;
		mail.attach_obj.count = 0;
		mail.attach_obj.pos = -1;
		key << header.receiver;
		if(pmailbox->find(key,value,txn))
			value >> box;
		else
			box.timestamp = now;
		if(box.mails.size()>100)
		{
			Log::log( LOG_ERR,
				"Send mail failed:src=%d:dst=%d:mid=%d:size=%d:money=%d:item=%d:count=%d:pos=%d",
			   	header.sender, header.receiver, header.id, 0, money, 0, 0, -1);
			return -1;
		}
		header.id = NextId(box);
		header.attribute = (1<<_MA_UNREAD); 
		header.send_time = now;
		if(money>0)
		{
			mail.attach_money = money;
			header.attribute ^= (1<<_MA_ATTACH_MONEY);
		}
		Log::formatlog("mail","type=send:src=%d:dst=%d:mid=%d:size=%d:money=%d:item=%d:count=%d:pos=%d",
			   	header.sender, header.receiver, header.id, 0, money, 0, 0, -1);
		mail.header = header;
		mail.header.receiver = now;
		box.mails.push_back(mail);
		pmailbox->insert( key, Marshal::OctetsStream()<<box, txn );
		return 0;
	}

	static int SendMaster(StorageEnv::Storage * pmailbox, StorageEnv::Storage * pinfo, GMailHeader& header,
			unsigned int fid,unsigned int money,StorageEnv::Transaction& txn)
	{
		Marshal::OctetsStream key, value;
		GMailBox box;
		time_t now = Timer::GetTime();
		GMail mail;
		GFactionInfo info;
		if( pinfo->find( Marshal::OctetsStream()<<fid, value, txn ) )
			value >> info;
		else 
			return -1;
		int roleid = info.master.rid;

		if(!roleid)
			return -1;

		header.receiver = roleid;
		mail.attach_money = 0;
		key << header.receiver;
		value.clear();
		if(pmailbox->find(key,value,txn))
			value >> box;
		else
			box.timestamp = now;
		if(box.mails.size()>100)
		{
			Log::log( LOG_ERR, "Send mail failed:src=%d:dst=%d:mid=%d:size=%d:money=%d:item=%d:count=%d:pos=%d",
				fid, header.sender, header.receiver, header.id, 0, money, mail.attach_obj.id, 
				mail.attach_obj.count, 0);
			return -1;
		}
		header.id = NextId(box);
		header.attribute = (1<<_MA_UNREAD); 
		header.send_time = now;
		if(money>0)
		{
			mail.attach_money = money;
			header.attribute ^= (1<<_MA_ATTACH_MONEY);
		}
						
		Log::formatlog("battlemail", "faction=%d:src=%d:dst=%d:mid=%d:size=%d:money=%d:item=%d:count=%d:pos=%d",
				fid, header.sender, header.receiver, header.id, 0, money, mail.attach_obj.id, 
				mail.attach_obj.count, 0);

		mail.header = header;
		mail.header.receiver = now;
		box.mails.push_back(mail);
		pmailbox->insert( key, Marshal::OctetsStream()<<box, txn );
		return 0;
	}
	static int SendMaster(StorageEnv::Storage * pmailbox, StorageEnv::Storage * pinfo, GMailHeader& header,
			unsigned int fid,GRoleInventory& inv,unsigned int money,StorageEnv::Transaction& txn)
	{
		Marshal::OctetsStream key, value;
		GMailBox box;
		time_t now = Timer::GetTime();
		GMail mail;
		GFactionInfo info;
		if( pinfo->find( Marshal::OctetsStream()<<fid, value, txn ) )
			value >> info;
		else 
			return -1;
		int roleid = info.master.rid;

		if(!roleid)
			return -1;

		header.receiver = roleid;
		mail.attach_money = 0;
		mail.attach_obj=inv;
		key << header.receiver;
		value.clear();
		if(pmailbox->find(key,value,txn))
			value >> box;
		else
			box.timestamp = now;
		if(box.mails.size()>100)
		{
			Log::log( LOG_ERR, "Send mail failed:src=%d:dst=%d:mid=%d:size=%d:money=%d:item=%d:count=%d:pos=%d",
				fid, header.sender, header.receiver, header.id, 0, money, mail.attach_obj.id, 
				mail.attach_obj.count, 0);
			return -1;
		}
		header.id = NextId(box);
		header.attribute = (1<<_MA_UNREAD); 
		header.send_time = now;
		if(inv.count>0)
		{
			mail.attach_obj=inv;
			header.attribute ^= (1<<_MA_ATTACH_OBJ);
		}
		if(money>0)
		{
			mail.attach_money = money;
			header.attribute ^= (1<<_MA_ATTACH_MONEY);
		}
						
		Log::formatlog("battlemail", "faction=%d:src=%d:dst=%d:mid=%d:size=%d:money=%d:item=%d:count=%d:pos=%d",
				fid, header.sender, header.receiver, header.id, 0, money, mail.attach_obj.id, 
				mail.attach_obj.count, 0);

		mail.header = header;
		mail.header.receiver = now;
		box.mails.push_back(mail);
		pmailbox->insert( key, Marshal::OctetsStream()<<box, txn );
		return 0;
	}
};

int WriteRestSyncData(StorageEnv::Storage *pstore,StorageEnv::Storage *plog,int roleid,GMailSyncData& data,int money_delta,StorageEnv::Transaction& txn);

class FactionFunc
{
public:
	template <typename V>
		static void Add(V& v, int type, int fid, int end_time)
		{
			v.push_back(typename V::value_type(type,fid,end_time));
		}
	template <typename V>
		static void Remove(V& v, int type, int fid)
		{
			for(typename V::iterator it = v.begin(); it != v.end(); ++it)
				if(it->type == type && it->fid == fid)
				{
					v.erase(it);
					break;
				}
		}
	template <typename V>
		static void Add(V& v, int fid, int end_time)
		{
			v.push_back(typename V::value_type(fid,end_time));
		}
	template <typename V>
		static void Remove(V& v, int fid)
		{
			for(typename V::iterator it = v.begin(); it != v.end(); ++it)
				if(it->fid == fid)
				{
					v.erase(it);
					break;
				}
		}
	template <typename V>	
		static bool Find(V& v, int fid)
		{
			for(typename V::iterator it = v.begin(); it != v.end(); ++it)
				if(it->fid == fid) return true;
			return false;
		}
	template <typename V>	
		static bool Find(V& v, int type, int fid)
		{
			for(typename V::iterator it = v.begin(); it != v.end(); ++it)
				if(it->type == type && it->fid == fid) return true;
			return false;
		}
};

class PShopFunc
{
public:
	enum PSHOP_OP
	{
		PSHOP_OP_MIN,
		PSHOP_OP_CREATE,
		PSHOP_OP_TRADE,
		PSHOP_OP_CANCEL_TRADE,
		PSHOP_OP_ACTIVATE,
		PSHOP_OP_SET_TYPE,
		PSHOP_OP_SAVE_MONEY,
		PSHOP_OP_DRAW_MONEY,
		PSHOP_OP_DRAW_ITEM,
		PSHOP_OP_SELF_GET,
		PSHOP_OP_CLEAR,
		PSHOP_OP_RENAME,
		PSHOP_OP_MAX,
	};

	//����״̬�����ƵĲ���
	static std::set<PSHOP_OP> _limits_normal;
	static std::set<PSHOP_OP> _limits_expired;

	/*
	 * ��������: ����Ȩ����֤
	 * @param: optype: ��������
	 * @ret: �Ƿ���������Ͳ���
	 */
	static bool OptionPolicy(const PShopDetail &detail, char optype)
	{
		PSHOP_OP op = static_cast<PSHOP_OP>(optype);
		if(detail.status == PSHOP_STATUS_NORMAL)
			return _limits_normal.find(op) == _limits_normal.end();
		else if(detail.status == PSHOP_STATUS_EXPIRED)
			return _limits_expired.find(op) == _limits_expired.end();
		return false;
	}

	/*
	 * ��������: ��Ǯת��Ϊ��Ʊ
	 * @param: money: ��Ҫ�һ��Ľ�Ǯ��
	 * @param: yinpiao: �һ��õ�����Ʊ
	 * @param: remains: ʣ���Ǯ��
	 */
	static void MoneyToYinPiao(uint64_t money, unsigned int &yinpiao, unsigned int &remains)
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

	/*
	 * ��������: ͳ�ƴ����ۻ��չ���Ʒ���ܼ�ֵ
	 * @list: ���������չ���
	 * @ret: �ܼ�ֵ
	 */
	static uint64_t CalItemValue(const PShopItemVector &list)
	{
		uint64_t itemvalue = 0;
		for(size_t i=0; i<list.size(); ++i)
			itemvalue += (uint64_t)list[i].price * (uint64_t)list[i].item.count;
		return itemvalue;
	}

	static unsigned int GetShopLifeByItem(int type)
	{
		//TODO
		//��Ҫ������ƷID��ʱ���ӳ��
		switch(type)
		{
			case PSHOP_CONSUME_ITEM_1:
				return 7*24*3600;
			case PSHOP_CONSUME_ITEM_2:
				return 30*24*3600;
			case PSHOP_CONSUME_ITEM_3:
				return 15*24*3600;
			default:
				// Ӧ��gamed �������ε��Ĵ���
				Log::log( LOG_ERR,"pshop consume item failed:item=%d",type);

				return 0;
		}
	}
};

/*
 * GRoleInventory�ṹ��Ʒ�б����
 * ����ϵͳ����Ұ����͵�����Ʊʹ�ô���
 */
class InvMan
{
	GRoleInventoryVector &_list;
	size_t _capacity;

public:
	InvMan(GRoleInventoryVector &l, int cap):_list(l),_capacity(cap){}

	/*
	 * ֻ�м���Ʒ�ſ��Ե��ô˽ӿ�,������ҪΪ��Ʊ��������ṩ
	 * ������Ʒ,���ȿ��Ƕѵ�,ʣ��ķ��������λ
	 */
	bool Push(const GRoleInventory &item, GRoleInventoryVector &listchange)
	{
		GRoleInventory inv = item;
		if(inv.id <= 0 || inv.count <= 0 || inv.count > inv.max_count)
			return false;
		for(size_t i=0; i<_list.size() && inv.count > 0; ++i)
		{
			GRoleInventory &it = _list[i];
			if(it.id == inv.id)
			{
				if(it.max_count - it.count >= inv.count)
				{
					it.count += inv.count;
					inv.count = 0;
				}
				else
				{
					inv.count -= (it.max_count - it.count);
					it.count = it.max_count;
				}
				listchange.push_back(it);
			}
		}
		if(inv.count > 0)
		{
			//�ѵ���,���������λ
			if(PushInEmpty(inv))
			{
				listchange.push_back(inv);
				return true;
			}
			return false;
		}
		return true;
	}
	/*�ŵ�ָ������λ��*/
	bool Push(int pos, GRoleInventory &item)
	{
		GRoleInventory &inv = item;
		if(pos < 0 || inv.id <= 0 || inv.count <= 0 || inv.count > inv.max_count)
			return false;
		if(!__IsEmptySlot(pos))
			return false;
		inv.pos = pos;
		_list.push_back(inv);
		return true;
	}
	/*�ŵ��������λ��*/
	bool PushInEmpty(GRoleInventory &item)
	{
		GRoleInventory &inv = item;
		if(inv.id <= 0 || inv.count <= 0 || inv.count > inv.max_count)
			return false;
		if(_list.size() >= _capacity)
			return false;
		inv.pos = __AllocSlot();
		_list.push_back(inv);
		return true;
	}

	/*���ȳ����ڶѵ������*/
	/*����ֶѵ�*/
	bool PushInHeap(GRoleInventory &item)
	{
		if(item.id <= 0 || item.count <= 0 || item.count > item.max_count)
			return false;

		for(size_t i=0; i<_list.size() ; ++i)
		{
			GRoleInventory &inv = _list[i];
			if(inv.id == item.id)
			{
				if(inv.max_count - inv.count >= item.count)
				{
					inv.count += item.count;
					item = inv;			
					return true;
				}
			}
		}
		
		return PushInEmpty(item);	
	}

	/*ɾ��ָ��λ�õ���Ʒ*/
	bool Remove(int pos, GRoleInventory &itemdel)
	{
		if(pos < 0) return false;
		if(__IsEmptySlot(pos)) return false;
		bool ret = false;
		for(size_t idx=0; idx<_list.size(); ++idx)
		{
			if(_list[idx].pos == pos)
			{
				ret = true;
				itemdel = _list[idx];
				_list.erase(_list.begin()+idx);
				break;
			}
		}
		return ret;
	}
	/*ɾ��ָ��λ��,ID,��������Ʒ*/
	bool Remove(int id, int pos, int count, GRoleInventory &itemchange)
	{
		if(id <= 0 || pos < 0 || count <= 0) return false;
		unsigned int idx=0;
		for(idx=0; idx<_list.size(); idx++)
		{
			if(_list[idx].pos == pos)
			{
				if(_list[idx].id != (unsigned int)id || _list[idx].count < count)
					return false;
				break;
			}
		}
		if(idx >= _list.size())
			return false;
		if(_list[idx].count == count)
		{
			_list[idx].count = 0;
			itemchange = _list[idx];
			_list.erase(_list.begin()+idx);
		}
		else
		{
			_list[idx].count -= count;
			itemchange = _list[idx];
		}
		return true;
	}
	/*ɾ��ָ��ID�͸�������Ʒ*/
	bool Remove(int id, int count, GRoleInventoryVector &listchange)
	{
		if(id <= 0 || count <= 0) return false;
		int total = 0;
		for(size_t idx=0; idx<_list.size(); ++idx)
		{
			if(_list[idx].id == (unsigned int)id)
				total += _list[idx].count;
		}
		if(total < count) return false;

		total = count;
		GRoleInventoryVector::iterator it = _list.begin();
		while(it != _list.end())
		{
			GRoleInventory &inv = *it;
			if(inv.id != (unsigned int)id)
			{
				++it;
				continue;
			}
			if(inv.count <= total)
			{
				total -= inv.count;
				inv.count = 0;
				listchange.push_back(inv);
				it = _list.erase(it);
			}
			else
			{
				inv.count -= total;
				listchange.push_back(inv);
				total = 0;
				++it;
			}
			if(total == 0)
				break;
		}
		return true;
	}
	bool IsItemExist(int id, int pos, int count) const
	{
		if(id <= 0) return false;
		if(pos < 0 || pos >= (int)_capacity) return false;
		for(size_t idx=0; idx<_list.size(); ++idx)
		{
			if(_list[idx].pos == pos) return (((int)(_list[idx].id) == id) && (_list[idx].count >= count));
		}
		return false;
	}
	bool IsItemExist(int id, int count) const
	{
		if(id <= 0) return false;
		int total = 0;
		for(size_t idx=0; idx<_list.size(); ++idx)
		{
			if(_list[idx].id == (unsigned int)id)
				total += _list[idx].count;
		}
		return total >= count;
	}
	const GRoleInventory * GetItem(int pos) const
	{   
		for(size_t idx=0; idx<_list.size(); ++idx)
			if(_list[idx].pos == pos)
				return &(_list[idx]);
		return NULL;
	} 
	bool HasEmptySlot() const
	{
		return _list.size() < _capacity;
	}
	unsigned int GetEmptySlotCount() const
	{
		return _capacity - _list.size();
	}
	void Trace() const
	{
		fprintf(stdout, "*************************************************************************\n");
		fprintf(stdout, "id\t\tpos\t\tcount\t\tmax_count\n");
		for(size_t i=0; i<_list.size(); ++i)
		{
			const GRoleInventory &inv = _list[i];
			fprintf(stdout, "%d\t\t%d\t\t%d\t\t%d\n", inv.id, inv.pos, inv.count, inv.max_count);
		}
		fprintf(stdout, "*************************************************************************\n");
	}
private:
	int __AllocSlot() const
	{
		std::set<int> slots;
		for(size_t i=0; i<_capacity; ++i) slots.insert(i);
		for(size_t i=0; i<_list.size(); ++i) slots.erase(_list[i].pos);
		if(slots.begin() != slots.end()) return *slots.begin();
		else return -1;
	}
	bool __IsEmptySlot(unsigned int pos) const
	{
		if(pos < 0 || pos >= _capacity) return false;
		size_t i;
		for(i=0; i<_list.size(); ++i)
			if((unsigned int)_list[i].pos == pos)
				break;
		return i >= _list.size();
	}
};

/*
 * PShopItem�ṹ��Ʒ�б������
 * ����ϵͳ�ĳ��������չ���ʹ�ô���
 */
class ShopInvMan
{
	PShopItemVector &_list;
	size_t _capacity;

public:
	ShopInvMan(PShopItemVector &l, int cap):_list(l),_capacity(cap){}
	/*�ŵ�ָ������λ��*/
	bool Push(int pos, PShopItem &item)
	{
		GRoleInventory &inv = item.item;
		if(pos < 0 || inv.id <= 0 || inv.count <= 0 || inv.count > inv.max_count)
			return false;
		if(!__IsEmptySlot(pos))
			return false;
		inv.pos = pos;
		_list.push_back(item);
		return true;
	}
	/*�ŵ��������λ��*/
	bool PushInEmpty(PShopItem &item)
	{
		GRoleInventory &inv = item.item;
		if(inv.id <= 0 || inv.count <= 0 || inv.count > inv.max_count)
			return false;
		if(_list.size() >= _capacity)
			return false;
		inv.pos = __AllocSlot();
		_list.push_back(item);
		return true;
	}
	/*ɾ��ָ��λ����Ʒ*/
	bool Remove(int pos, PShopItem &itemdel)
	{
		if(pos < 0) return false;
		if(__IsEmptySlot(pos)) return false;
		bool ret =false;
		for(size_t idx=0; idx<_list.size(); ++idx)
		{
			if(_list[idx].item.pos == pos)
			{
				ret = true;
				itemdel = _list[idx];
				_list.erase(_list.begin()+idx);
				break;
			}
		}
		return ret;
	}
	/*ɾ��ָ��λ����Ʒ*/
	bool Remove(int id, int pos, int count, PShopItem &itemchange)
	{
		if(id <= 0 || pos < 0 || count <= 0) return false;
		unsigned int idx=0;
		for(idx=0; idx<_list.size(); idx++)
		{
			GRoleInventory item = _list[idx].item;
			if(item.pos == pos)
			{
				if(item.id != (unsigned int)id || item.count < count)
					return false;
				break;
			}
		}
		if(idx >= _list.size())
			return false;
		if(_list[idx].item.count == count)
		{
			_list[idx].item.count = 0;
			itemchange = _list[idx];
			_list.erase(_list.begin()+idx);
		}
		else
		{
			_list[idx].item.count -= count;
			itemchange = _list[idx];
		}
		return true;
	}
	bool IsItemExist(int id, int pos, int count) const
	{
		if(id <= 0) return false;
		if(pos < 0 || pos >= (int)_capacity) return false;
		for(size_t idx=0; idx<_list.size(); ++idx)
		{
			if(_list[idx].item.pos == pos) return (((int)(_list[idx].item.id) == id) && (_list[idx].item.count >= count));
		}
		return false;
	}
	const PShopItem * GetItem(int pos) const
	{   
		for(size_t idx=0; idx<_list.size(); ++idx)
			if(_list[idx].item.pos == pos)
				return &(_list[idx]);
		return NULL;
	}
	bool HasEmptySlot() const
	{
		return _list.size() < _capacity;
	}
	bool IsEmptySlot(unsigned int pos) const
	{
		return __IsEmptySlot(pos);
	}
	void Trace() const
	{
		fprintf(stdout, "*************************************************************************\n");
		fprintf(stdout, "id\t\tpos\t\tcount\t\tmax_count\t\tprice\n");
		for(size_t i=0; i<_list.size(); ++i)
		{
			const GRoleInventory &inv = _list[i].item;
			fprintf(stdout, "%d\t\t%d\t\t%d\t\t%d\t\t%d\n", inv.id, inv.pos, inv.count, inv.max_count, _list[i].price);
		}
		fprintf(stdout, "*************************************************************************\n");
	}
private:
	int __AllocSlot() const
	{
		std::set<int> slots;
		for(size_t i=0; i<_capacity; ++i) slots.insert(i);
		for(size_t i=0; i<_list.size(); ++i) slots.erase(_list[i].item.pos);
		if(slots.begin() != slots.end()) return *slots.begin();
		else return -1;
	}
	bool __IsEmptySlot(unsigned int pos) const
	{
		if(pos < 0 || pos >= _capacity) return false;
		size_t i;
		for(i=0; i<_list.size(); ++i)
			if((unsigned int)_list[i].item.pos == pos)
				break;
		return i >= _list.size();
	}
};

void GetRoleRealmDetail(const Octets & odata,int& level);
int  GetRoleReincarnationTimes(const Octets & odata);
void GetRoleReincarnationDetail(const Octets & odata,int& times,int& maxlevel,GReincarnationData& data);

};
#endif
