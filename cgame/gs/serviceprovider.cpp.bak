#include "world.h"
#include "serviceprovider.h"
#include "player_imp.h"
#include "item.h"
#include <common/protocol_imp.h>
#include "usermsg.h"
#include "gsp_if.h"

#include "clstab.h"
#include "item.h"
#include "item/equip_item.h"
#include "template/itemdataman.h"
#include "skillwrapper.h"
#include "npcgenerator.h"
#include "actsession.h"
#include "task/taskman.h"
#include <factionlib.h>
#include <mailsyslib.h>
#include <auctionsyslib.h>
#include "playerstall.h"
#include "travel_filter.h"
#include "servicenpc.h"
#include "cooldowncfg.h"
#include "sfilterdef.h"
#include <sellpointlib.h>
#include <stocklib.h>
#include <webtradesyslib.h>
#include <kingelectionsyslib.h>
#include <pshopsyslib.h>

#include "global_controller.h"


namespace NG_ELEMNET_SERVICE
{

class general_provider: public service_provider
{
protected:	
	inline void SendServiceContent(int id, int cs_index, int sid, const void *buf, size_t size)
	{
		packet_wrapper  h1(size + 32);
		using namespace S2C;
		gobject * pObj = _imp->_parent;
		CMD::Make<CMD::npc_service_content>::From(h1,pObj->ID, _type, buf,size);
		send_ls_msg(cs_index,id,sid,h1);
	}

	inline void SendMessage(int message, const XID & target,int param, const void * buf, size_t size)
	{
		MSG msg;
		BuildMessage(msg,message,target,_imp->_parent->ID,_imp->_parent->pos,
				param,buf,size);
		_imp->_plane->PostLazyMessage(msg);
	}
	
};

class feedback_provider : public general_provider
{
	virtual feedback_provider * Clone()
	{
		ASSERT(!_is_init);
		return new feedback_provider(*this);
	}

	virtual bool Save(archive & ar)
	{
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		return true;
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		ASSERT(size == 0);
		return true;
	}
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		//将数据发回,表示有这个服务
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class vendor_provider : public general_provider
{
	float _tax_rate;
//	abase::vector<int> 		_left_list;	//物品剩余数量的列表
//	abase::vector<int> 		_amount_list;	//物品数量的列表
	abase::vector<const item_data *, abase::fast_alloc<> >   	_item_list;	//物品的数据列表
	abase::vector<int , abase::fast_alloc<> >   	_price_list;			//物品的价格列表
	abase::vector<int , abase::fast_alloc<> >   	_consume_contrib_cost_list;		//物品的帮派消费贡献度花费列表
	abase::vector<int , abase::fast_alloc<> >   	_cumulate_contrib_limit_list;	//物品的帮派累积贡献度购买限制列表
	abase::vector<int , abase::fast_alloc<> >		_force_contrib_cost_list;		//物品的势力战功花费列表
	abase::vector<int , abase::fast_alloc<> >		_force_limit_list;				//物品的势力购买限制列表
	abase::vector<int , abase::fast_alloc<> >		_force_reputation_limit_list;	//物品的势力威望购买限制列表
	
	virtual bool Save(archive & ar)
	{
		ar << _tax_rate;
		ar << (int)_item_list.size();
		for(size_t i =0; i < _item_list.size(); i ++)
		{
			if(!_item_list[i])
			{
				ar << 0 << 0 << 0 << 0 << 0 << 0;
			}
			else
			{
				ar << _item_list[i]->type << _consume_contrib_cost_list[i] << _cumulate_contrib_limit_list[i]
					<< _force_contrib_cost_list[i] << _force_limit_list[i] << _force_reputation_limit_list[i];
			}
		}
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate;
		size_t size;
		ar >> size;
		if(size > 1024) 
		{
			ASSERT(false);
			size = 1024;
		}
		abase::vector<int, abase::fast_alloc<> > tmpList;
		tmpList.reserve(size*6);
		for(size_t i =0; i < size; i ++)
		{
			int item_type, consume_contrib_cost, cumulate_contrib_limit, force_contrib_cost, force_limit, force_reputation_limit;
			ar >> item_type >> consume_contrib_cost >> cumulate_contrib_limit >> force_contrib_cost >> force_limit >> force_reputation_limit;
			tmpList.push_back(item_type);
			tmpList.push_back(consume_contrib_cost);
			tmpList.push_back(cumulate_contrib_limit);
			tmpList.push_back(force_contrib_cost);
			tmpList.push_back(force_limit);
			tmpList.push_back(force_reputation_limit);
		}
		OnInit(tmpList.begin(),tmpList.size()*sizeof(int));
		return true;
	}
public:
	virtual ~vendor_provider()
	{
		_item_list.clear();
	}

	struct bag 
	{
		int item_type;
		size_t index;
		size_t count;
	};

	struct request
	{
		size_t money;
		int consume_contrib;
		int cumulate_contrib;
		int force_id;
		int force_repu;
		int force_contrib;
		size_t count;
		bag item_list[];
	};

	vendor_provider():_tax_rate(1.05f)
	{
	}
	void AdjustVendorFee(int & fee)
	{
		if(fee < 100) return ;
		if(fee < 1000)
		{
			int r = fee % 10;
			if(r) fee += 10 - r;
		}
		else //if(fee < 10000)
		{
			int r = fee % 100;
			if(r) fee += 100 - r;
		}
	}

private:
	
	
	virtual vendor_provider * Clone()
	{
		ASSERT(!_is_init);
		return new vendor_provider(*this);
	}
	
	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size % (sizeof(int)*6)) 
		{
			ASSERT(false);
			return false;
		}
		int * list = (int*)buf;
		size_t count = size / (sizeof(int)*6);

		float tax_rate = ((service_npc*)_imp)->GetTaxRate() + 1.f;
		_item_list.clear();_item_list.reserve(count);
		for(size_t i = 0; i < count; i ++)
		{
			if(list[6*i] == 0)
			{
				_item_list.push_back(NULL);
				_price_list.push_back(0);
				_consume_contrib_cost_list.push_back(0);
				_cumulate_contrib_limit_list.push_back(0);
				_force_contrib_cost_list.push_back(0);
				_force_limit_list.push_back(0);
				_force_reputation_limit_list.push_back(0);
				continue;
			}

			const void *pBuf = world_manager::GetDataMan().get_item_for_sell(list[6*i]);
			if(pBuf)
			{
				const item_data * data = (const item_data*)pBuf;
				_item_list.push_back(data);	
				int shop_price = world_manager::GetDataMan().get_item_shop_price(list[6*i]);
				if(shop_price < (int)data->price)
				{
					__PRINTF("商店卖出价小于卖店价格 %d\n",list[6*i]);
					ASSERT(false);
					shop_price = data->price;
				}
				float fp = shop_price * _tax_rate * tax_rate + 0.5f;
				if(fp > 2e8) fp = 2e8;
				int price = (int)(fp);
				if(price <= 0) price = 1;
				AdjustVendorFee(price); 	//计算税后进一取整后的钱数 
				_price_list.push_back(price);
				int consume_contrib_cost = list[6*i+1];
				int cumulate_contrib_limit = list[6*i+2];
				int force_contrib_cost 		= list[6*i+3];
				int force_limit 			= list[6*i+4];
				int force_reputation_limit 	= list[6*i+5];
				if(consume_contrib_cost < 0) consume_contrib_cost = 0;
				if(cumulate_contrib_limit < 0) cumulate_contrib_limit = 0;
				if(force_contrib_cost < 0) force_contrib_cost = 0;
				if(force_limit < 0) force_limit = 0;
				if(force_reputation_limit < 0) force_reputation_limit = 0;
				_consume_contrib_cost_list.push_back(consume_contrib_cost);
				_cumulate_contrib_limit_list.push_back(cumulate_contrib_limit);
				_force_contrib_cost_list.push_back(force_contrib_cost);
				_force_limit_list.push_back(force_limit);
				_force_reputation_limit_list.push_back(force_reputation_limit);
			}
			else
			{
				__PRINTF("can not init vendor goods %d\n", list[6*i]);
				_item_list.push_back(NULL);
				_price_list.push_back(0);
				_consume_contrib_cost_list.push_back(0);
				_cumulate_contrib_limit_list.push_back(0);
				_force_contrib_cost_list.push_back(0);
				_force_limit_list.push_back(0);
				_force_reputation_limit_list.push_back(0);
			}
		}
		return true;
	}
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		//param 是 索引 buf 代表数量和自己的钱数(初步判断,未必准确)
		request * param = (request *)buf;
		if(size != sizeof(request) + param->count*sizeof(bag)) return;
		
		size_t money_need = 0;
		int consume_contrib_need = 0;
		int cumulate_contrib_limit = 0;
		int force_contrib_need = 0;
		int force_limit = 0;
		int force_reputation_limit = 0;
		for(size_t i=0; i < param->count; ++i)
		{
			size_t index = param->item_list[i].index;
			if(index >=_item_list.size()) 
			{
				//客户端传来的错误的数据
				SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
				return ;
			}
			const item_data * pItem = _item_list[index];
			int price = _price_list[index];
			int contrib_cost = _consume_contrib_cost_list[index];
			int contrib_limit = _cumulate_contrib_limit_list[index];
			int fcontrib_cost = _force_contrib_cost_list[index];
			int flimit = _force_limit_list[index];
			int frepu_limit = _force_reputation_limit_list[index];
			if(!pItem)
			{
				//没有找到正确的数据
				SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
				return ;
			}
			ASSERT(pItem->item_content == ((const char * )pItem) + sizeof(item_data));
			size_t count = param->item_list[i].count; 
			if(!count || count > pItem->pile_limit || param->item_list[i].item_type != pItem->type)
			{
				SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
				return ;
			}
			float p = (float)money_need + (float)(price) * (float)count;
			if(p > 2e8)
			{
				//钱数太多
				SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
				return ;
			}
			money_need += price* count;
			if(money_need > 0x7FFFFFFF)
			{
				//没有找到正确的数据
				SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
				return ;
			}
			//
			float c = (float)consume_contrib_need + (float)contrib_cost * (float)count;
			if(c > 2e9)
			{
				SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_NOT_ENOUGH_FACTION_CONTRIB,NULL,0);
				return;
			}
			consume_contrib_need += contrib_cost*count;
			//
			if(contrib_limit > cumulate_contrib_limit)
				cumulate_contrib_limit = contrib_limit;
			//势力战功消耗
			float fc = (float)force_contrib_need + (float)fcontrib_cost * (float)count;
			if(fc > 2e9)
			{
				SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_NOT_ENOUGH_FORCE_CONTRIB,NULL,0);
				return;
			}
			force_contrib_need += fcontrib_cost*count;
			//势力限制，购买列表中存在势力限制的物品势力限制必须相同
			if(flimit != 0)
			{
				if(force_limit == 0)
				{
					force_limit = flimit;
				}
				else if(force_limit != flimit)
				{
					SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
					return;
				}
			}
			//势力威望限制
			if(frepu_limit > force_reputation_limit)
				force_reputation_limit = frepu_limit;
		}

		if(money_need > param->money)
		{
			//钱数不够，返回错误信息
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
			return;
		}

		if(consume_contrib_need > param->consume_contrib || cumulate_contrib_limit > param->cumulate_contrib)
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_NOT_ENOUGH_FACTION_CONTRIB,NULL,0);
			return;
		}
		
		if(force_limit != 0 && force_limit != param->force_id)
		{
			//势力不匹配
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
			return;
		}
		
		if(force_contrib_need > param->force_contrib)
		{
			//势力战功不足
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_NOT_ENOUGH_FORCE_CONTRIB,NULL,0);
			return;
		}

		if(force_reputation_limit > param->force_repu)
		{
			//势力威望不足
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_NOT_ENOUGH_FORCE_REPU,NULL,0);
			return;
		}
		param->money = money_need;
		param->consume_contrib = consume_contrib_need;
		param->cumulate_contrib = cumulate_contrib_limit;
		param->force_id = force_limit;
		param->force_repu = force_reputation_limit;
		param->force_contrib = force_contrib_need;

		//发出物品数据和要求的钱数
		//发出回馈数据
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}

};

class vendor_executor : public service_executor
{
public:
	typedef vendor_provider::request player_request;

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		player_request * req= (player_request*)buf;
		if(req->count <= 0 || req->count > 12 ||		//一次最多购买12个商品
			size != sizeof(player_request)+req->count*sizeof(vendor_provider::bag )) return false;
		
		req->money = pImp->GetMoney();
		req->consume_contrib = pImp->GetFactionConsumeContrib();
		req->cumulate_contrib = pImp->GetFactionCumulateContrib();
		req->force_id = pImp->_player_force.GetForce();
		req->force_repu = pImp->_player_force.GetReputation();
		req->force_contrib = pImp->_player_force.GetContribution();
		
		if(req->money == 0 && req->consume_contrib == 0 && req->force_contrib == 0) return false;	
		if(!pImp->InventoryHasSlot(req->count)) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		player_request * req= (player_request*)buf;
		if(req->count <= 0 || 
				size != sizeof(player_request)+req->count*sizeof(vendor_provider::bag ))
		{
			ASSERT(false);
			return false;
		}
		
		if(!pImp->InventoryHasSlot(req->count)) 
		{
			pImp->_runner->error_message(S2C::ERR_INVENTORY_IS_FULL);
			return false;
		}
		
		//检查钱数
		if(pImp->GetMoney() < req->money)
		{
			//钱数不够
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}

		if(pImp->GetFactionConsumeContrib() < req->consume_contrib || pImp->GetFactionCumulateContrib() < req->cumulate_contrib)
		{
			 pImp->_runner->error_message(S2C::ERR_NOT_ENOUGH_FACTION_CONTRIB);
			 return false;
		}

		if(req->force_id != 0 && pImp->_player_force.GetForce() != req->force_id)
		{
			//势力不匹配
			pImp->_runner->error_message(S2C::ERR_SERVICE_ERR_REQUEST);
			return false;
		}
		
		if(pImp->_player_force.GetContribution() < req->force_contrib)
		{
			//势力战功不足
			pImp->_runner->error_message(S2C::ERR_NOT_ENOUGH_FORCE_CONTRIB);
			return false;
		}

		if(pImp->_player_force.GetReputation() < req->force_repu)
		{
			//势力威望不足
			pImp->_runner->error_message(S2C::ERR_NOT_ENOUGH_FORCE_REPU);
			return false;
		}
		//给player添加物品
		abase::vector<abase::pair<const item_data*,int> ,abase::fast_alloc<> > list;
		list.reserve(req->count);
		for(size_t i = 0; i < req->count; i ++)
		{
			int item_id = req->item_list[i].item_type;
			const item_data * pItem = (const item_data*)world_manager::GetDataMan().get_item_for_sell(item_id);
			ASSERT(pItem);
			if(pItem) list.push_back(abase::pair<const item_data*,int>(pItem,req->item_list[i].count));
		}
		pImp->PurchaseItem(list.begin(),list.size(), req->money, req->consume_contrib, req->force_contrib);
		return true;
	}
};

typedef feedback_provider purchase_provider;

class purchase_executor : public service_executor
{
public:
	struct  player_request
	{
		size_t item_count;
		struct item_data
		{
			int type;		//物品类型
			size_t inv_index; 	//在包裹栏里的号码
			size_t count;		//数量
			int price;		//由于引入了卖店价会实时变化的物品,所以客户端卖店时须确认价格,价格一致时方可出售
		} list[1];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		if(req->item_count > pImp->GetInventory().Size() || size != sizeof(player_request) + (req->item_count-1)*sizeof(player_request::item_data) ) return false;

		for(size_t i = 0; i < req->item_count; i ++)
		{
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY,req->list[i].inv_index);
		}
		/*
		//校验该物品是否存在
		if(!pImp->IsItemCanSell(req->inv_index, req->type,req->count))
		{
			return false;
		}
		由于是批量卖出操作，不再进行物品是否存在的检查
		*/
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size < sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;
		ASSERT(req->item_count  <= 0x7FFF&&size==sizeof(player_request)+(req->item_count-1)*sizeof(player_request::item_data));
		for(size_t i = 0; i < req->item_count;i ++)
		{
			if(!pImp->ItemToMoney(req->list[i].inv_index,req->list[i].type,req->list[i].count,req->list[i].price))
			{
				return true;
			}
		}
		return true;
	}
};

typedef feedback_provider repair_provider;
class repair_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{
		int type;		//物品类型 如果是-1则全部修理,后面的两个值无效
		unsigned char where;		// 修理哪个位置
		unsigned char index;		// 位置索引
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		if(req->where != gplayer_imp::IL_EQUIPMENT && req->where != gplayer_imp::IL_INVENTORY) return false;
		if(req->type != -1 && !pImp->IsItemNeedRepair(req->where,req->index, req->type))
		{
			//校验该物品是否存在
			return false;
		}

		//不考虑钱数
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;

		GLog::log(GLOG_INFO,"用户%d修理装备",pImp->_parent->ID.id);

		ASSERT(req->where==gplayer_imp::IL_EQUIPMENT || req->where==gplayer_imp::IL_INVENTORY);
		if(req->type == -1)
		{
			pImp->RepairAllEquipment();
		}
		else
		{
			pImp->RepairEquipment(req->where,req->index);
		}
		return true;
	}
};

typedef feedback_provider heal_provider;
class heal_executor : public service_executor
{
public:
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != 0) return false;
		if(!pImp->IsBled())
		{
			return true;
		}
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(!pImp->IsCombatState()) 
			pImp->Renew();
		else
			pImp->_runner->error_message(S2C::ERR_CANNOT_HEAL_IN_COMBAT);
		return true;
	}
};

class transmit_provider : public general_provider
{
public:
	typedef npc_template::npc_statement::__st_ent transmit_entry;
/*	struct transmit_entry 
	{
		int 		world_tag
		A3DVECTOR 	target;
		size_t		fee;
		int		require_level;
		int		target_waypoint;
	};*/

private:
	float _tax_rate;
	abase::vector<transmit_entry,abase::fast_alloc<> > _target_list;	//传送的目标列表

	virtual bool Save(archive & ar)
	{
		ar << _tax_rate;
		size_t size = _target_list.size();
		ar << (int)size;
		ar.push_back(_target_list.begin(),sizeof(transmit_entry)*size);
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate;
		size_t size;
		ar >> size;
		for(size_t i = 0; i < size; i ++)
		{
			transmit_entry  entry;
			ar.pop_back(&entry,sizeof(entry));
			_target_list.push_back(entry);
		}
		return true;
	}
public:
	struct request
	{
		size_t index;	//目标点索引
		size_t money;	//player当前的钱数
	};

	transmit_provider():_tax_rate(1.f)
	{}
private:
	
	
	virtual transmit_provider * Clone()
	{
		ASSERT(!_is_init);
		return new transmit_provider(*this);
	}
	
	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size % sizeof(transmit_entry) || size == 0) 
		{
			ASSERT(false);
			return false;
		}
		transmit_entry * list = (transmit_entry *)buf;
		size_t count = size / sizeof(transmit_entry);

		_target_list.clear();_target_list.reserve(count);
		for(size_t i = 0; i < count; i ++)
		{
			if(_imp->_plane->GetGrid().IsOutsideGrid(list[i].target.x,list[i].target.z))
			{
				//ASSERT(false);
				__PRINTF("建立传送服务失败， 目标点不在网格内\n");
				return false;
			}
			//测试结束
			_target_list.push_back(list[i]);
		}
		return true;
	}

	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request) ) return;
		request * param = (request *)buf;
		size_t index = param->index;
		if(index >=_target_list.size()) 
		{
			//客户端传来的错误的数据
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
			return ;
		}
		size_t money = param->money;
		if(money < _target_list[index].fee) 
		{
			//通知该玩家钱不够
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
			return ;
		}
		//发出数据
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,&(_target_list[index]),sizeof(transmit_entry));
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class transmit_executor : public service_executor
{
public:
	struct  player_request
	{
		size_t index;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		//检查状态是否正确
		if(pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL) return false;

		player_request * req= (player_request*)buf;
		transmit_provider::request data;
		data.index = req->index;
		data.money = pImp->GetMoney();
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&data,sizeof(data));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(transmit_provider::transmit_entry));
		transmit_provider::transmit_entry * entry = (transmit_provider::transmit_entry*) buf;

		//检查状态是否正确
		if(pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL) return false;

		//价差是否可以进行传送
		if(pImp->_basic.level < entry->require_level)
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}

		if(!pImp->IsWaypointActived( entry->target_waypoint & 0xFFFF)) 
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		
		//检查钱数
		size_t fee = entry->fee;
		if(pImp->GetMoney() < fee)
		{
			//钱数不够
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}
		gplayer * pParent = pImp->GetParent();
		GLog::log(GLOG_INFO,"用户%d使用传送服务从(%f,%f,%f)到(%f,%f,%f)",pParent->ID.id,pParent->pos.x,pParent->pos.y,pParent->pos.z,entry->target.x,entry->target.y,entry->target.z);

		//检查是否可以进行传送（检查目标点是否在本机，如果是远程的则检查远程目标点是否能够到达）
		//GetGlobalServer只能在同样Tag的世界中进行传送，所以如果世界不同，则无法进行传送操作
		if(!pImp->_plane->PosInWorld(entry->target))	
		{
			int dest = pImp->_plane->GetGlobalServer(entry->target);
			if(dest <0)
			{
				pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
				return false;
			}
		}

		//减少钱...
		if(fee)
		{
			pImp->SpendMoney(fee);
			pImp->_runner->spend_money(fee);
		}

		//让player进行传送
		pImp->LongJump(entry->target, entry->world_tag);
		return true;
	}
};

class general_id_provider : public general_provider
{
protected:
	abase::vector<int, abase::fast_alloc<> > _list;

	virtual bool Save(archive & ar)
	{
		size_t size = _list.size();
		ar << (int)size;
		ar.push_back(_list.begin(),sizeof(int)*size);
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		size_t size;
		ar >> size;
		for(size_t i = 0; i < size; i ++)
		{
			int entry;
			ar.pop_back(&entry,sizeof(entry));
			_list.push_back(entry);
		}
		return true;
	}
protected:
	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size % sizeof(int) || !size) 
		{
			ASSERT(false);
			return false;
		}
		int * list = (int*)buf;
		size_t count = size / sizeof(int);
		_list.reserve(count);
		for(size_t i = 0; i < count; i ++)
		{
			_list.push_back(list[i]);
		}
		std::sort(_list.begin(),_list.end());
		return true;
	}
};

class task_provider : public general_id_provider
{

public:
	struct request
	{
		//int npc_id;
		int task_id;
	};

private:
	task_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new task_provider(*this);
	}
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		int id = req->task_id;
		//if(req->npc_id==((gnpc*)(_imp->_parent))->tid && std::binary_search(_list.begin(),_list.end(),id))
		if(std::binary_search(_list.begin(),_list.end(),id))
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_TASK_NOT_AVAILABLE,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class task_executor : public service_executor
{

public:
	typedef task_provider::request player_request;
	task_executor(){}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	
};

class task_in_provider : public task_provider
{
public:
	struct request
	{
		int task_id;
		int choice;
	};
private:
	task_in_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new task_in_provider(*this);
	}
	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		int id = req->task_id;
		//if(req->npc_id==((gnpc*)(_imp->_parent))->tid && std::binary_search(_list.begin(),_list.end(),id))
		if(std::binary_search(_list.begin(),_list.end(),id))
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_TASK_NOT_AVAILABLE,NULL,0);
		}
	}

};

class task_in_executor : public service_executor
{

	typedef task_in_provider::request player_request;
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}

	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		PlayerTaskInterface  task_if(pImp);
		player_request * req = (player_request*)buf;
		OnTaskCheckAward(&task_if,req->task_id,req->choice);
		return true;
	}
};

class task_out_provider : public task_provider
{
public:
	struct request
	{
		int task_id;
		int task_set_id;	//库任务id，如为0则为普通任务
		int refresh_item;	//库任务的刷新物品
	};
	int _task_set_id;
	int _refresh_item;
private:
	task_out_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new task_out_provider(*this);
	}
	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size % sizeof(int) || !size) 
		{
			ASSERT(false);
			return false;
		}

		_task_set_id = *(int*)buf;
		_refresh_item = *((int*)buf+1);
		return general_id_provider::OnInit( ((const char *)buf) + 2*sizeof(int),size - 2*sizeof(int));
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		int id = req->task_id;

		if(id == 0 && _task_set_id != 0 && req->task_set_id == _task_set_id && req->refresh_item == _refresh_item)
		{
			//请求刷新库任务
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		
		}
		else if(id != 0 && req->task_set_id == _task_set_id && std::binary_search(_list.begin(),_list.end(),id))
		{
			//接普通/库任务
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_TASK_NOT_AVAILABLE,NULL,0);
		}
	}
};

class task_out_executor : public task_executor
{
	typedef task_out_provider::request player_request;
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}

	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*) buf;
		
		PlayerTaskInterface  task_if(pImp);
		if(req->task_id == 0)
		{
			//刷新任务
			if(!pImp->IsItemExist(req->refresh_item))
			{
				pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
				return false;
			}
			if(!task_if.RefreshTaskStorage(req->task_set_id))
			{
				pImp->_runner->error_message(S2C::ERR_TASK_NOT_AVAILABLE);
				return false;			
			}
			pImp->TakeOutItem(req->refresh_item);
		}
		else
		{
			//接任务
			if(OnTaskCheckDeliver(&task_if, req->task_id, req->task_set_id))
			{
				//接到任务了
				__PRINTF("接到任务了...........\n");
			}
			else
			{
				pImp->_runner->error_message(S2C::ERR_TASK_NOT_AVAILABLE);
			}
		}
		return true;
	}
};

class task_matter_provider : public task_provider
{
public:
	struct feed_back
	{
		int task_id;
		int npc_tid;
	};
private:
	task_matter_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new task_matter_provider(*this);
	}
	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		int id = req->task_id;
		//if(req->npc_id==((gnpc*)(_imp->_parent))->tid && std::binary_search(_list.begin(),_list.end(),id))
		if(std::binary_search(_list.begin(),_list.end(),id))
		{
			//找到回应
			gnpc * pNPC = (gnpc*)_imp->_parent;
			feed_back fb = { id, pNPC->tid };
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,&fb,sizeof(fb));
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_TASK_NOT_AVAILABLE,NULL,0);
		}
	}

};

class task_matter_executor : public task_executor
{

private:
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(task_matter_provider::feed_back));
		task_matter_provider::feed_back & fb = *(task_matter_provider::feed_back*)buf;
		PlayerTaskInterface  task_if(pImp);
		OnNPCDeliverTaskItem(&task_if,fb.npc_tid,fb.task_id);
		return true;
	}
};

class skill_provider : public general_id_provider
{

public:
	struct request
	{
		int skill_id;
	};

private:
	skill_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new skill_provider(*this);
	}
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		int id = req->skill_id;
		if(std::binary_search(_list.begin(),_list.end(),id))
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class skill_executor : public service_executor
{

public:
	typedef skill_provider::request player_request;
	skill_executor(){}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*) buf;
		if(pImp->IsCombatState())
		{
			pImp->_runner->error_message(S2C::ERR_INVALID_OPERATION_IN_COMBAT);
			return false;
		}
		if(pImp->_session_state == gactive_imp::STATE_SESSION_USE_SKILL)
		{
			pImp->_runner->error_message(S2C::ERR_CANNOT_LEARN_SKILL);
			return false;
		}
		int rlevel;
		if(( rlevel = pImp->_skill.Learn((unsigned int)req->skill_id,object_interface(pImp))) <= 0)
		{
			//不能学习技能
			pImp->_runner->error_message(S2C::ERR_CANNOT_LEARN_SKILL);
			return false;
		}
		GLog::log(GLOG_INFO, "用户%d技能%d达到%d级",pImp->_parent->ID.id, req->skill_id, rlevel);
		pImp->_runner->learn_skill(req->skill_id, rlevel);
		return true;
	}
};

typedef feedback_provider install_provider;
class install_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{
		unsigned short chip_idx;
		unsigned short equip_idx;
		int chip_type;
		int equip_type;
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		item_list & inv = pImp->GetInventory();
		size_t inv_size = inv.Size();
		if(req->chip_idx >= inv_size || req->equip_idx >= inv_size 
				|| inv[req->chip_idx].type != req->chip_type 
				|| inv[req->equip_idx].type != req->equip_type)
		{
			//校验该物品是否存在
			return false;
		}

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		//收费按照宝石来计算
		player_request * req = (player_request*)buf;
		item_list & inv = pImp->GetInventory();
		int chip_id = inv[req->chip_idx].type;
		DATA_TYPE dt;
		STONE_ESSENCE * ess = (STONE_ESSENCE*) world_manager::GetDataMan().get_data_ptr(chip_id, ID_SPACE_ESSENCE, dt);
		if(!ess || dt != DT_STONE_ESSENCE) 
		{
			pImp->_runner->error_message(S2C::ERR_CANNOT_EMBED);
			return false;
		}
		
		if(pImp->GetMoney() < (size_t)ess->install_price)
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}

		if(pImp->EmbedChipToEquipment(req->chip_idx,req->equip_idx))
		{
			int item_id = inv[req->equip_idx].type;
			GLog::log(GLOG_INFO, "用户%d在装备%d上镶嵌了%d",pImp->_parent->ID.id,item_id,chip_id);
			pImp->SpendMoney(ess->install_price);
			pImp->_runner->spend_money(ess->install_price);
			return true;
		}
		return true;
	}
};

typedef feedback_provider uninstall_provider;
class uninstall_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{
		size_t equip_idx;
		int equip_type;
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		item_list & inv = pImp->GetInventory();
		size_t inv_size = inv.Size();
		if(req->equip_idx >= inv_size || inv[req->equip_idx].type != req->equip_type)
		{
			//校验该物品是否存在
			return false;
		}

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		//收费按照宝石来计算 
		player_request * req = (player_request*)buf;
		item_list & inv = pImp->GetInventory();
		size_t money = pImp->GetMoney();
		size_t usemoney = 0;
		if(!inv.ClearEmbed(req->equip_idx, money,usemoney))
		{
			return false;
		}
		GLog::log(GLOG_INFO, "用户%d在装备%d去除了镶嵌物",pImp->_parent->ID.id,inv[req->equip_idx].type);
		pImp->SpendMoney(usemoney);
		pImp->_runner->clear_embedded_chip(req->equip_idx,usemoney);
		return true;
	}
};

class produce_provider : public general_provider
{
public:
	struct request
	{
		int skill;
		int id;
		size_t count;
	};

private:
	float _tax_rate;
	int _skill_id;
	abase::vector<int> _target_list;	//可生产的目标列表

	virtual bool Save(archive & ar)
	{
		ar << _tax_rate << _skill_id;
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate >> _skill_id;
		return true;
	}

public:
	produce_provider():_tax_rate(1.f),_skill_id(-1)
	{}
private:
	virtual produce_provider * Clone()
	{
		ASSERT(!_is_init);
		return new produce_provider(*this);
	}
	virtual bool OnInit(const void * buf, size_t size)
	{
		typedef npc_template::npc_statement::__service_produce  DATA;
		if(size != sizeof(DATA))
		{
			ASSERT(false);
			return false;
		}
		DATA * t = (DATA*)buf;
		_skill_id = t->produce_skill;
		_target_list.reserve(t->produce_num);
		for(int i  = 0; i < t->produce_num; i ++)
		{
			_target_list.push_back(t->produce_items[i]);
			ASSERT(t->produce_items[i]);
		}
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->skill == _skill_id)
		{
			//检查是否能生产
			for(size_t i = 0; i < _target_list.size(); i ++)
			{
				if(_target_list[i] == req->id)
				{
					//找到了合适的物品
					SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
					return;
				}
			}
		}

		SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class produce_executor : public service_executor
{

public:
	typedef produce_provider::request inner_request;
	typedef struct 
	{
		int skill;
		int id;
		size_t count;
	} player_request;
	produce_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//校验物品配方是否存在
		const recipe_template *rt = recipe_manager::GetRecipe(req->id);
		if(!rt) return false;
		if(rt->equipment_need_upgrade > 0) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		//校验配方技能和级别是否匹配
		if(rt->produce_skill != req->skill || 
				rt->produce_skill > 0 && pImp->GetSkillLevel(rt->produce_skill) < rt->require_level) return false;

		//检查金钱是否足够
		if(pImp->GetMoney() < rt->fee) 
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return true;	//只报告金钱不够的错误
		}
		inner_request xreq;
		memset(&xreq,0,sizeof(xreq));
		
		xreq.skill = req->skill;
		xreq.id = req->id;
		xreq.count = req->count;

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&xreq,sizeof(xreq));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return false;
		}
		//	加入制造的session
		const recipe_template *rt = recipe_manager::GetRecipe(req->id);
		if(!rt) 
		{
			__PRINTF("配方%d不存在在分解服务\n",req->id);
			return false;
		}

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		pImp->AddSession(new session_produce(pImp,rt,req->count));
		pImp->StartSession();
		return true;
	}
	
};

class decompose_provider : public general_provider
{
public:
	struct request
	{
		int skill;
		size_t index;
		int id;
	};

private:
	float _tax_rate;
//	abase::vector<int,abase::fast_alloc<> > _target_list;	//传送的目标列表
	int _skill_id;
	virtual bool Save(archive & ar)
	{
		ar << _tax_rate << _skill_id;
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate >> _skill_id;
		return true;
	}
public:
	decompose_provider():_tax_rate(1.f),_skill_id(-1)
	{}

private:
	virtual decompose_provider * Clone()
	{
		ASSERT(!_is_init);
		return new decompose_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size != sizeof(int)) 
		{
			ASSERT(false);
			return false;
		}
		_skill_id = *(int*)buf;
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->skill == _skill_id)
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class decompose_executor : public service_executor
{

public:
	typedef decompose_provider::request player_request;
	decompose_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		size_t index = req->index;
		if(index >= pImp->GetInventory().Size()
				||pImp->GetInventory()[index].type != req->id)  return false;
		
		//损毁的物品不能分解
		if(pImp->GetInventory()[index].proc_type & item::ITEM_PROC_TYPE_DAMAGED) return false;
	
		//校验物品配方是否存在
		const decompose_recipe_template *rt = recipe_manager::GetDecomposeRecipe(req->id);
		if(!rt) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->id)) return false;

		//校验配方技能和级别是否匹配
		if(rt->produce_skill != req->skill || 
				rt->produce_skill > 0 && pImp->GetSkillLevel(rt->produce_skill) < rt->require_level) return false;

		//察看是否能够进行拆分
		if(rt->element_id == 0 || rt->element_num == 0) return false;

		//检查金钱是否足够
		if(pImp->GetMoney() < rt->decompose_fee) 
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return true;	//只报告金钱不够的错误
		}

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;

		//	加入制造的session
		const decompose_recipe_template *rt = recipe_manager::GetDecomposeRecipe(req->id);
		if(!rt) 
		{
			__PRINTF("配方%d不存在在分解服务\n",req->id);
			return false;
		}

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->id)) return false;

		pImp->AddSession(new session_decompose(pImp,req->index,rt));
		pImp->StartSession();
		return true;
	}
	
};

class produce2_provider : public general_provider
{
public:
	struct request
	{
		int skill;
		int recipe_id;
		int materials_id[16];
		int idxs[16];
	};

private:
	float _tax_rate;
	int _skill_id;
	abase::vector<int> _target_list;	//可生产的目标列表

	virtual bool Save(archive & ar)
	{
		ar << _tax_rate << _skill_id;
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate >> _skill_id;
		return true;
	}

public:
	produce2_provider():_tax_rate(1.f),_skill_id(-1)
	{}
private:
	virtual produce2_provider * Clone()
	{
		ASSERT(!_is_init);
		return new produce2_provider(*this);
	}
	virtual bool OnInit(const void * buf, size_t size)
	{
		typedef npc_template::npc_statement::__service_produce  DATA;
		if(size != sizeof(DATA))
		{
			ASSERT(false);
			return false;
		}
		DATA * t = (DATA*)buf;
		_skill_id = t->produce_skill;
		_target_list.reserve(t->produce_num);
		for(int i  = 0; i < t->produce_num; i ++)
		{
			_target_list.push_back(t->produce_items[i]);
			ASSERT(t->produce_items[i]);
		}
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->skill == _skill_id)
		{
			//检查是否能生产
			for(size_t i = 0; i < _target_list.size(); i ++)
			{
				if(_target_list[i] == req->recipe_id)
				{
					//找到了合适的物品
					SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
					return;
				}
			}
		}

		SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class produce2_executor : public service_executor
{

public:
	typedef produce2_provider::request player_request;
	produce2_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//校验物品配方是否存在
		const recipe_template *rt = recipe_manager::GetRecipe(req->recipe_id);
		if(!rt || rt->bind_type == 0&&rt->proc_type == 0 ) return false;
		if(rt->equipment_need_upgrade > 0) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		//校验配方技能和级别是否匹配
		if(rt->produce_skill != req->skill || 
				rt->produce_skill > 0 && pImp->GetSkillLevel(rt->produce_skill) < rt->require_level) return false;

		//检查金钱是否足够
		if(pImp->GetMoney() < rt->fee) 
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return true;	//只报告金钱不够的错误
		}

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return false;
		}
		//	加入制造的session
		const recipe_template *rt = recipe_manager::GetRecipe(req->recipe_id);
		if(!rt) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		pImp->AddSession(new session_produce2(pImp,rt,req->materials_id, req->idxs));
		pImp->StartSession();
		return true;
	}
	
};

typedef feedback_provider trashbox_passwd_provider;
class trashbox_passwd_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{
		unsigned short origin_size;
		unsigned short new_size;
		char origin_passwd[1];
		char new_passwd[1];
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(unsigned short) * 2) return false;
		player_request * req = (player_request*)buf;
		if(size != sizeof(unsigned short) * 2 + req->origin_size + req->new_size) return false;

		//检查原始密码的正确与否

		const char * origin_passwd = req->origin_passwd;
		const char * new_passwd = req->origin_passwd + req->origin_size;

		if(!pImp->_trashbox.IsPasswordValid(new_passwd,req->new_size))
		{
			pImp->_runner->error_message(S2C::ERR_INVALID_PASSWD_FORMAT);
			return true;
		}

		if(!pImp->_trashbox.CheckPassword(origin_passwd,req->origin_size))
		{
			pImp->_runner->error_message(S2C::ERR_PASSWD_NOT_MATCH);
			return true;
		}
		
		pImp->SetSecurityPasswdChecked(true);	//同时安全密码通过
		
		//不考虑钱数
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,new_passwd,req->new_size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		GLog::log(GLOG_INFO,"用户%d修改了仓库密码，新密码长度%d",pImp->_parent->ID.id,size);
		pImp->_trashbox.SetPassword((const char *)buf,size);
		pImp->_runner->trashbox_passwd_changed(pImp->_trashbox.HasPassword());
		return true;
	}
};

typedef feedback_provider trashbox_open_provider;
class trashbox_open_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{
		unsigned int passwd_size;
		char passwd[];
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(unsigned int)) return false;
		player_request * req = (player_request*)buf;
		if(size != req->passwd_size + sizeof(unsigned int)) return false;

		if(pImp->HasSession())
		{
			pImp->_runner->error_message(S2C::ERR_OTHER_SESSION_IN_EXECUTE);
			return true;
		}

		//考虑添加重试计数策略 

		//检查密码
		const char * passwd = req->passwd;
		if(!pImp->_trashbox.CheckPassword(passwd,req->passwd_size))
		{
			pImp->_runner->error_message(S2C::ERR_PASSWD_NOT_MATCH);
			return true;
		}

		pImp->SetSecurityPasswdChecked(true);	//同时安全密码通过
		
		//不考虑钱数
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,0,0);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		//开启钱箱
		//加入当前的
		if(!pImp->HasSession())
		{	
			__PRINTF("芝麻开门啦、\n");
			session_use_trashbox *pSession = new session_use_trashbox(pImp);
			pSession->SetViewOnly(false);
			pImp->AddSession(pSession);
			pImp->StartSession();
		}
		return true;
	}
};

class plane_switch_provider : public general_provider
{
public:
	typedef npc_template::npc_statement::__pw_ent transmit_entry;
	/*struct transmit_entry 
	{
		A3DVECTOR 	target_pos;
		int 		target_tag;
		int		require_level;
		size_t		fee;
	};*/

private:
	float _tax_rate;
	abase::vector<transmit_entry,abase::fast_alloc<> > _target_list;	//传送的目标列表
	
	virtual bool Save(archive & ar)
	{
		ar << _tax_rate;
		size_t size = _target_list.size();
		ar << (int)size;
		ar.push_back(_target_list.begin(),sizeof(transmit_entry)*size);
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate;
		size_t size;
		ar >> size;
		for(size_t i = 0; i < size; i ++)
		{
			transmit_entry  entry;
			ar.pop_back(&entry,sizeof(entry));
			_target_list.push_back(entry);
		}
		return true;
	}
public:
	struct request
	{
		size_t index;	//目标点索引
		size_t money;	//player当前的钱数
		int    level;	//玩家的级别
	};

	plane_switch_provider():_tax_rate(1.f)
	{}
private:
	
	
	virtual plane_switch_provider * Clone()
	{
		ASSERT(!_is_init);
		return new plane_switch_provider(*this);
	}
	
	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size % sizeof(transmit_entry) || size == 0) 
		{
			ASSERT(false);
			return false;
		}
		transmit_entry * list = (transmit_entry *)buf;
		size_t count = size / sizeof(transmit_entry);

		_target_list.clear();_target_list.reserve(count);
		for(size_t i = 0; i < count; i ++)
		{
			//测试结束
			_target_list.push_back(list[i]);
		}
		return true;
	}

	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request) ) return;
		request * param = (request *)buf;
		size_t index = param->index;
		if(index >=_target_list.size()) 
		{
			//客户端传来的错误的数据
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
			return ;
		}
		size_t money = param->money;
		if(money < _target_list[index].fee) 
		{
			//通知该玩家钱不够
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
			return ;
		}

		if(param->level < _target_list[index].require_level)
		{
			//通知该玩家级别不够
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_LEVEL_NOT_MATCH,NULL,0);
			return;
		}
		//发出数据
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,&(_target_list[index]),sizeof(transmit_entry));
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

//此服务并未真正启用 但后面的注释很有意义， 
class plane_switch_executor : public service_executor
{
public:
	struct  player_request
	{
		size_t index;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;

		player_request * req= (player_request*)buf;
		plane_switch_provider::request data;
		data.index = req->index;
		data.money = pImp->GetMoney();
		data.level = pImp->_basic.level;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&data,sizeof(data));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(transmit_provider::transmit_entry));
		plane_switch_provider::transmit_entry * entry = (plane_switch_provider::transmit_entry*) buf;
		
		//检查钱数
		size_t fee = entry->fee;
		if(pImp->GetMoney() < fee)
		{
			//钱数不够
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}


		//如果是副本  取得传送所需要的key
		//看起来需要进行几次交互，首先通知对方需要进入，对方回应说需要何种Key(包含是否有剩余空间 )
		//然后进行传送并且将key告知，或者一种办法是直接传送自己所有的key给对方，并要求传送
		//目前已知的key有多种 但是对于帮派基地是个特例，所以很麻烦。 帮派基地副本可能是一个特例，
		//所以帮派传送传送服务可能也需要单独完成。
		instance_key key;
		memset(&key,0,sizeof(key));
		pImp->GetInstanceKey(entry->target_tag,key);
		key.target = key.essence;
		
		//目前考虑直接传送几个Key过去1:自己的ID(level0)，2.队长的ID和TeamSeq(level1) 3.帮派id(level2) 
		//同时考虑对所有远程跳转和传送都基于同样的模式，只是非位面传送所附加的ID都应该是0


		//让Player进行副本传送 
		if(world_manager::GetInstance()->PlaneSwitch(pImp,entry->target_pos,entry->target_tag,key,fee) < 0)
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
		}
		return true;
	}
};


class identify_provider : public general_provider
{
public:
	struct request
	{
		unsigned short index;
		size_t money;
	};

private:
	size_t _fee;
	virtual bool Save(archive & ar)
	{
		ar << _fee;
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _fee;
		return true;
	}
public:
	identify_provider():_fee(0)
	{}

private:
	virtual identify_provider * Clone()
	{
		ASSERT(!_is_init);
		return new identify_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size != sizeof(size_t)) 
		{
			ASSERT(false);
			return false;
		}
		_fee = *(size_t*)buf;
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->money < _fee)
		{
			//发送金钱不够
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
		}
		else
		{
			//设置金钱数目
			req->money = _fee;
			//发送回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class identify_executor : public service_executor
{

	struct player_request
	{
		unsigned int index;
		int type;
	};
public:
	identify_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		size_t index = req->index;
		item_list & inv = pImp->GetInventory();
		if(index >= inv.Size() || inv[index].type != req->type || req->type == -1) return false;
		
		identify_provider::request request;
		request.index = req->index;
		request.money = pImp->GetMoney();

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&request,sizeof(request));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		identify_provider::request * req = (identify_provider::request*)buf;
		if(size != sizeof(*req))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}

		//	加入制造的session
		if(pImp->GetMoney() < req->money)
		{
			return false;
		}

		GLog::log(GLOG_INFO,"用户%d鉴定了物品",pImp->_parent->ID.id);
		pImp->IdentifyItemAddon(req->index, req->money);
		return true;
	}
	
};

typedef feedback_provider faction_service_provider;

class faction_service_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;
		
		//检查是否可以开始
		if(world_manager::GetWorldParam().forbid_faction)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		if(world_manager::GetWorldParam().forbid_faction)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		player_request * req = (player_request*)buf;
		//发送请求数据
		if(!GNET::ForwardFactionOP(req->service_id,pImp->_parent->ID.id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了帮派操作",pImp->_parent->ID.id);
		return true;
	}
};

//这个provider并没有用处，只是空实现
class player_market_provider :public general_provider
{

	virtual bool Save(archive & ar)
	{
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		return true;
	}
private:
	player_market_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new player_market_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_UNAVILABLE,NULL,0);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class player_market_executor : public service_executor
{
public:
	typedef player_stall::trade_request player_request;
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;

		player_request * req = (player_request*)buf;
		if(req->count == 0 || req->count > PLAYER_MARKET_MAX_SELL_SLOT+PLAYER_MARKET_MAX_BUY_SLOT) return false;
		if(req->count * sizeof(player_request::entry_t) + sizeof(player_request) != size) return false;
		req->money = pImp->GetMoney();
		if(pImp->GetInventory().GetEmptySlotCount() < req->count)
		{
			pImp->_runner->error_message(S2C::ERR_INVENTORY_IS_FULL);
			return true;
		}
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(false);
		//执行不是如此进行地
		return true;
	}
};

class vehicle_service_provider  :public general_provider
{
public:
	typedef npc_template::npc_statement::vehicle_path_entry entry_t;
	abase::vector<entry_t, abase::fast_alloc<> > _list;

	struct request
	{
		int index;
		int line_no;
		int waypoint;
		size_t money;
	};
	
	virtual bool Save(archive & ar)
	{
		size_t size = _list.size();
		ar << (int)size;
		ar.push_back(_list.begin(),sizeof(entry_t)*size);
		return true;
	}

	virtual bool Load(archive & ar) 
	{
		size_t size;
		ar >> size;
		for(size_t i = 0; i < size; i ++)
		{
			entry_t entry;
			ar.pop_back(&entry,sizeof(entry));
			_list.push_back(entry);
		}
		return true;
	}
private:
	vehicle_service_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new vehicle_service_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size % sizeof(entry_t)) return false;
		size_t 	count = size / sizeof(entry_t);
		entry_t *pEntry = (entry_t*)buf;
		for(size_t i = 0; i < count; i ++)
		{
			_list.push_back(*pEntry);
		}
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		size_t idx = req->index;
		if(idx >= _list.size())
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_UNAVILABLE,NULL,0);
			return ;
		}
		if(req->line_no == _list[idx].line_no && req->waypoint == _list[idx].target_waypoint)
		{
			if(req->money >= _list[idx].fee)
			{
				SendMessage(GM_MSG_SERVICE_DATA,player,_type,&(_list[idx]),sizeof(entry_t));
				return;
			}
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
			return ;
		}
		SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_UNAVILABLE,NULL,0);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class vehicle_service_executor : public service_executor
{
public:

	typedef vehicle_service_provider::request request;
	struct player_request
	{
		int index;
		int line_no;
		int target_waypoint;
	};
	
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;

		player_request * req = (player_request*)buf;
		if(req->target_waypoint < 0) return false;
		if(!pImp->IsWaypointActived(req->target_waypoint & 0xFFFF)) return false;

		request req2;
		req2.index = req->index;
		req2.line_no = req->line_no;
		req2.money = pImp->GetMoney();
		req2.waypoint = req->target_waypoint;

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&req2,sizeof(req2));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(vehicle_service_provider::entry_t))
		{
			ASSERT(false);
			return false;
		}
		const vehicle_service_provider::entry_t & ent = *(vehicle_service_provider::entry_t *)buf;
		if(ent.fee > pImp->GetMoney())
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}

		if(pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL || pImp->_parent->IsZombie()
				|| pImp->HasSession())
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}

		//取消可能的飞行效果
		pImp->_filters.RemoveFilter(FILTER_FLY_EFFECT);
		
		//减少金钱
		pImp->SpendMoney(ent.fee);
		pImp->_runner->spend_money(ent.fee);

		//清除所有DEBUFF
		pImp->_filters.ClearSpecFilter(filter::FILTER_MASK_DEBUFF);

		//加入旅行的filter attach的时候会转入旅行状态
		travel_filter *pf;
		pf = new travel_filter((gactive_imp*)pImp,FILTER_INDEX_TRAVEL,
				ent.min_time,ent.max_time,ent.dest_pos,ent.speed, ent.vehicle,ent.line_no);
		pImp->_filters.AddFilter(pf);
		return true;
	}
};

class player_market_executor2 : public service_executor
{
public:
	typedef player_stall::trade_request player_request;
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;

		player_request * req = (player_request*)buf;
		if(req->count > PLAYER_MARKET_MAX_SELL_SLOT+PLAYER_MARKET_MAX_BUY_SLOT) return false;
		if(req->count * sizeof(player_request::entry_t) + sizeof(player_request) != size) return false;

		for(size_t i = 0; i < req->count; i ++)
		{
			pImp->_runner->unlock_inventory_slot(gplayer_imp::IL_INVENTORY,req->list[i].inv_index);
		}

		//检查是否有这些物品
		req->money = pImp->GetMoney();

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(false);
		//执行不是如此进行地
		return true;
	}
};

class waypoint_service_provider  :public general_provider
{
public:
	int _waypoint;
	struct request
	{
	};
	
	virtual bool Save(archive & ar) { ar << _waypoint; return true; } 
	virtual bool Load(archive & ar) { ar >> _waypoint; return true; }
private:
	waypoint_service_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new waypoint_service_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size != sizeof(int) ) return false;
		_waypoint = *(int*)buf;
		ASSERT(_waypoint >= 0);
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,&_waypoint,sizeof(_waypoint));
	}

	virtual void OnHeartbeat()
	{} 

	virtual void OnControl(int param ,const void * buf, size_t size)
	{}
};

class waypoint_service_executor : public service_executor
{
public:
	
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,"",0);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(int ))
		{
			ASSERT(false);
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		int waypoint = *(int*)buf;
		ASSERT(waypoint >= 0);
		
		GLog::log(GLOG_INFO,"用户%d激活了传送点%d" ,pImp->_parent->ID.id,waypoint & 0xFFFF);
		pImp->ActivateWaypoint(waypoint & 0xFFFF);
		return true;
	}
};

typedef feedback_provider unlearn_skill_provider;
class unlearn_skill_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{	
		int skill_id;
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		int skillid = req->skill_id;
		if(pImp->GetSkillLevel(skillid) <= 0)
		{
			return false;
		}

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;
		if(pImp->RemoveSkill(req->skill_id))
		{
			pImp->_runner->error_message(S2C::ERR_CAN_NOT_UNLEARN_SKILL);
		}
		else
		{
			
			PlayerTaskInterface  task_if(pImp);
			OnForgetLivingSkill(&task_if);
			GLog::log(GLOG_INFO,"用户%d遗忘了技能%d",pImp->_parent->ID.id,req->skill_id);
			pImp->_runner->learn_skill(req->skill_id, 0);
		}
		return true;
	}
};


typedef feedback_provider cosmetic_provider;
class cosmetic_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{	
		size_t inv_index;
		int    item_type;
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		//检查状态是否合适
		if(pImp->_filters.IsFilterExist(FILTER_INDEX_FACEPILL)) return false;
		if(pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL) return false;
		if(pImp->HasNextSession()) return false;
		
		if(!pImp->IsItemExist(req->inv_index,req->item_type, 1)) return false;

		//检查冷却时间
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_FACETICKET)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}
		
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}

		//检查状态是否合适
		if(pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL) return false;
		if(pImp->HasSession()) return false;

		player_request * req = (player_request*)buf;
		if(!pImp->IsItemExist(req->inv_index,req->item_type, 1)) 
		{
			return false;
		}

		//取数据并检查
		const FACETICKET_ESSENCE * pess;
		DATA_TYPE dt;
		pess = (const FACETICKET_ESSENCE *)world_manager::GetDataMan().get_data_ptr(req->item_type,ID_SPACE_ESSENCE,dt);
		if(!pess || dt != DT_FACETICKET_ESSENCE)
		{
			return false;
		}
		
		if(pImp->_basic.level < pess->require_level)
		{
			return false;
		}

		//发送开始整容的消息， 并进入整容状态 
		pImp->AddSession(new session_cosmetic(pImp, req->inv_index, req->item_type));
		pImp->StartSession();
		return true;
	}
};

typedef feedback_provider mail_service_provider;

class mail_service_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;
		
		//检查是否可以开始
		if(world_manager::GetWorldParam().forbid_mail)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		if(world_manager::GetWorldParam().forbid_mail)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		player_request * req = (player_request*)buf;
		//发送请求数据
		if(!GNET::ForwardMailSysOP(req->service_id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了邮件操作",pImp->_parent->ID.id);
		return true;
	}
};

typedef feedback_provider double_exp_provider;
class double_exp_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{
		size_t bonus_index;
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		return false;	//这个版本中领取双倍时间的服务已经禁止
		static int bonus_time[] = {3600,3600*2,3600*3,3600*4};
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		if(req->bonus_index >= 4) return false;
		if(!pImp->TestRestTime(bonus_time[req->bonus_index]))
		{
			//没有足够的双倍时间
			pImp->_runner->error_message(S2C::ERR_NOT_ENOUGH_REST_TIME);
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		static int bonus_time[] = {3600,3600*2,3600*3,3600*4};
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}

		player_request * req = (player_request*)buf;
		if(!pImp->TestRestTime(bonus_time[req->bonus_index]))
		{
			//没有足够的双倍时间
			pImp->_runner->error_message(S2C::ERR_NOT_ENOUGH_REST_TIME);
			return false;
		}
		int t = bonus_time[req->bonus_index];

		//完成操作，增加双倍时间，增加双倍时间消耗
		pImp->ActiveDoubleExpTime(t);
		return true;
	}
};

typedef feedback_provider auction_service_provider;

class auction_service_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;
		
		//检查是否可以开始
		if(world_manager::GetWorldParam().forbid_auction)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		if(world_manager::GetWorldParam().forbid_auction)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		player_request * req = (player_request*)buf;
		//发送请求数据
		if(!GNET::ForwardAuctionSysOP(req->service_id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了拍卖操作",pImp->_parent->ID.id);
		return true;
	}
};

typedef feedback_provider hatch_pet_service_provider;

class hatch_pet_service_executor : public service_executor
{
public:
	struct player_request
	{
		int egg_index;
		int egg_id;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;

		//检查孵化的宠物蛋是否存在
		item_list & inv = pImp->GetInventory();
		if(!inv.IsItemExist(req->egg_index, req->egg_id, 1))
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		//检查是否正确的物品
		item & it = inv[req->egg_index];
		DATA_TYPE dt;
		PET_EGG_ESSENCE * ess;
		ess = (PET_EGG_ESSENCE*) world_manager::GetDataMan().get_data_ptr(it.type, ID_SPACE_ESSENCE, dt);
		if(!it.body || !ess || dt != DT_PET_EGG_ESSENCE)
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}

		if(pImp->GetMoney() < (size_t)ess->money_hatched)
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}

		int rst = it.Use(item::INVENTORY,req->egg_index,pImp,1);
		//进行孵化操作
		if(rst > 0)
		{
			//孵化成功
			inv.DecAmount(req->egg_index,rst);
			pImp->_runner->use_item(gplayer_imp::IL_INVENTORY,req->egg_index, req->egg_id,rst);

			//减少金钱
			pImp->SpendMoney(ess->money_hatched);
			pImp->_runner->spend_money(ess->money_hatched);
			
			GLog::log(GLOG_INFO,"用户%d孵化了宠物蛋%d",pImp->_parent->ID.id, req->egg_id);
			return true;
		}
		else
		{
			pImp->_runner->error_message(S2C::ERR_PET_CAN_NOT_BE_HATCHED);
			return false;
		}
	}
};

typedef feedback_provider restore_pet_service_provider;

class restore_pet_service_executor : public service_executor
{
public:
	struct player_request
	{
		size_t pet_index;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;
		
		
		//这个操作由Player内部完成
		int rst = pImp->ServiceConvertPetToEgg(req->pet_index);
		if(rst)
		{
			pImp->_runner->error_message(rst);
		}
		//日志在里面记录
		return true;
	}
};

typedef feedback_provider battle_service_provider;

class battle_service_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		//发送请求数据 
		if(!GNET::ForwardBattleOP(req->service_id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了战场操作",pImp->_parent->ID.id);
		return true;
	}
};

class towerbuild_provider : public general_provider
{
public:
	struct request
	{
		size_t index;
		size_t money;
		int item_id;
	};

	typedef npc_template::npc_statement::__npc_tower_build  entry_t;
	entry_t _build_list[4];
	int _time_counter;

private:
	virtual bool Save(archive & ar)
	{
		ar.push_back(_build_list,sizeof(_build_list));
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar.pop_back(_build_list,sizeof(_build_list));
		return true;
	}
public:

	towerbuild_provider()
	{
		_time_counter = 0;
	}

private:
	virtual towerbuild_provider * Clone()
	{
		ASSERT(!_is_init);
		return new towerbuild_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size != sizeof(_build_list)) 
		{
			ASSERT(false);
			return false;
		}
		memcpy(_build_list, buf, size);
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		size_t index = req->index;
		if(index >= 4)
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_UNAVILABLE,NULL,0);
			return;
		}

		if(!_build_list[index].id_in_build || !_build_list[index].id_buildup)
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_UNAVILABLE,NULL,0);
			return;
		}

		if(_time_counter)
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_NPC_SERVICE_IS_BUSY,NULL,0);
			return;
		}
		
		if(req->money < (size_t)_build_list[index].fee || req->item_id != _build_list[index].id_object_need)
		{
			//发送金钱不够
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
			return ;
		}

		_time_counter = 2;
		
		//发回回应数据
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,_build_list + index,sizeof(_build_list[index]));
	}

	virtual void OnHeartbeat()
	{
		if(_time_counter)
		{
			_time_counter--;
			if(_time_counter <=0)
			{
				_time_counter = 0;
			}
		}
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class towerbuild_executor : public service_executor
{
public:
	struct player_request
	{
		size_t index;		//服务索引
		size_t item_id;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;

		player_request *preq = (player_request*)buf;
		
		//检查物品是否存在
		if(preq->item_id != 0 && !pImp->IsItemExist(preq->item_id))
		{
			return false;
		}
		
		towerbuild_provider::request req = {preq->index,pImp->GetMoney(),preq->item_id};

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&req,sizeof(req));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(towerbuild_provider::entry_t));
		towerbuild_provider::entry_t * pEnt = (towerbuild_provider::entry_t *)buf;
		if(pImp->GetMoney() < (size_t)pEnt->fee)
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}

		item_list & inv = pImp->GetInventory();
		int item_index = -1;
		if(pEnt->id_object_need && (item_index = inv.Find(0,pEnt->id_object_need)) < 0)
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}
		
		//可以进行服务，减少金钱和物品
		if(pEnt->fee)
		{
			pImp->SpendMoney(pEnt->fee);
			pImp->_runner->spend_money(pEnt->fee);
		}

		if(item_index >= 0)
		{
			item& it = inv[item_index];
			pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
			
			inv.DecAmount(item_index,1);
			pImp->_runner->use_item(gplayer_imp::IL_INVENTORY,item_index, pEnt->id_object_need,1);
		}

		//发送一个激活消息通知对方
		int flag = msg_npc_transform::FLAG_DOUBLE_DMG_IN_BUILD;
		msg_npc_transform data = {pEnt->id_in_build, pEnt->time_use , flag, pEnt->id_buildup};
		pImp->SendTo<0>(GM_MSG_NPC_TRANSFORM,provider,0,&data,sizeof(data));
		return true;
	}
};

typedef feedback_provider battle_leave_provider;
class battle_leave_executor : public service_executor
{
public:
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != 0) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		pImp->_filters.ModifyFilter(FILTER_CHECK_INSTANCE_KEY,FMID_CLEAR_AEBF,NULL,0);
		return true;
	}
};

class resetprop_provider : public general_provider
{
public:
	struct request
	{
		size_t index;
		int item_id;
	};

	typedef npc_template::npc_statement::__reset_prop  entry_t;
	abase::vector<entry_t> _reset_list;

private:
	virtual bool Save(archive & ar)
	{
		//未做
		ASSERT(false);
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		//未做
		ASSERT(false);
		return true;
	}
public:

	resetprop_provider()
	{
	}

private:
	virtual resetprop_provider * Clone()
	{
		ASSERT(!_is_init);
		return new resetprop_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size % sizeof(entry_t))
		{
			ASSERT(false);
			return false;
		}
		size_t t = size / sizeof(entry_t);
		if(t > 15)
		{
			ASSERT(false);
			return false;
		}
		
		entry_t * pEnt = (entry_t *)buf;
		_reset_list.reserve(t);
		for(size_t i = 0 ;i < t ; i++)
		{
			_reset_list.push_back(pEnt[i]);
		}

		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		size_t index = req->index;
		if(index >= _reset_list.size())
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_UNAVILABLE,NULL,0);
			return;
		}
		
		if(req->item_id != _reset_list[index].object_need)
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_ITEM_NOT_IN_INVENTORY,NULL,0);
			return ;
		}

		
		//发回回应数据
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,&(_reset_list[index]),sizeof(_reset_list[index]));
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class resetprop_executor : public service_executor
{
public:
	typedef resetprop_provider::request player_request;

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;

		player_request *preq = (player_request*)buf;
		
		//检查物品是否存在
		if(preq->item_id == 0 || !pImp->IsItemExist(preq->item_id))
		{
			return false;
		}
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(resetprop_provider::entry_t));
		resetprop_provider::entry_t * pEnt = (resetprop_provider::entry_t *)buf;

		item_list & inv = pImp->GetInventory();
		int item_index = -1;
		if(!pEnt->object_need || (item_index = inv.Find(0,pEnt->object_need)) < 0)
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}
		
		//发送一个激活消息通知对方
		//洗点
		if(!pImp->RegroupPropPoint(pEnt->str_delta, pEnt->agi_delta, pEnt->vit_delta, pEnt->eng_delta))
		{
			pImp->_runner->error_message(S2C::ERR_CAN_NOT_RESET_PP);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了洗点操作(%d,%d,%d,%d)",
				pImp->_parent->ID.id,
				pEnt->str_delta, pEnt->agi_delta, pEnt->vit_delta, pEnt->eng_delta);
		
		if(item_index >= 0)
		{
			item& it = inv[item_index];
			pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
			
			inv.DecAmount(item_index,1);
			pImp->_runner->use_item(gplayer_imp::IL_INVENTORY,item_index, pEnt->object_need,1);
		}

		return true;
	}
};

typedef feedback_provider spec_trade_provider;

class spec_trade_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;

		if(world_manager::GetWorldParam().forbid_cash_trade)
		{
			return false;
		}

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		//发送请求数据 
		if(!GNET::ForwardSellPointSysOP(req->service_id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了点卡操作",pImp->_parent->ID.id);
		return true;
	}
};

typedef feedback_provider refine_service_provider;

class refine_service_executor : public service_executor
{
public:
	struct player_request
	{
		int inv_index;
		int item_type;
		int rt_index;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		//价格冷却限制一下
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_REFINE)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		if(!pImp->IsItemExist(req->inv_index,req->item_type, 1)) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		if(!pImp->IsItemExist(req->inv_index,req->item_type, 1)) return false;

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_REFINE,REFINE_COOLDOWN_TIME);
		if(!pImp->RefineItemAddon(req->inv_index, req->item_type, req->rt_index))
		{
			//无法进行精炼
			pImp->_runner->error_message(S2C::ERR_REFINE_CAN_NOT_REFINE);
		}

		return true;
	}
};

class change_pet_name_provider : public general_provider
{
public:
	struct request
	{
		int money_need;
		int item_need;
		unsigned short pet_index;
		unsigned short  name_len;
		char name[pet_data::MAX_NAME_LEN];
	};

private:
	virtual bool Save(archive & ar)
	{
		//未做
		ASSERT(false);
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		//未做
		ASSERT(false);
		return true;
	}
	int _money_need;
	int _item_need;
public:

	change_pet_name_provider()
	{
	}

private:
	virtual change_pet_name_provider * Clone()
	{
		ASSERT(!_is_init);
		return new change_pet_name_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size != sizeof(int) + sizeof(int))
		{
			ASSERT(false);
			return false;
		}
		_money_need = ((int *)buf)[0];
		_item_need = ((int*)buf)[1];
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->money_need <_money_need)
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
			return;
		}

		request reply = *req;
		reply.money_need = _money_need;
		reply.item_need = _item_need;

		//发回回应数据
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,&reply,sizeof(reply));
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class change_pet_name_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned short pet_index;
		unsigned short name_len;
		char name[];
	};
#pragma pack()
	typedef change_pet_name_provider::request request;

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		player_request *preq = (player_request*)buf;
		size_t name_len = preq->name_len;
		if(name_len == 0 || 
			name_len > pet_data::MAX_NAME_LEN ||
			(name_len & 0x01) || name_len + sizeof(player_request) != size) return false;


		request req;
		req.pet_index= preq->pet_index;
		req.name_len = name_len;
		memcpy(req.name, preq->name, name_len);

		req.item_need = 0;
		req.money_need = pImp->GetMoney();
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&req, sizeof(req));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(request));
		request * req = (request*) buf;

		if(req->money_need > 0)
		{
			if(pImp->GetMoney() < (size_t)req->money_need)
			{
				pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
				return false;
			}
		}

		item_list & inv = pImp->GetInventory();
		int item_index = -1;
		if(req->item_need && (item_index = inv.Find(0,req->item_need)) < 0)
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		//更改宠物名称 
		if(!pImp->ChangePetName(req->pet_index, req->name, req->name_len))
		{
			return false;
		}

		//改名代价很低，不做日志了
		
		//删除物品
		if(item_index >= 0)
		{
			item& it = inv[item_index];
			pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
			
			inv.DecAmount(item_index,1);
			pImp->_runner->use_item(gplayer_imp::IL_INVENTORY,item_index, req->item_need,1);
		}

		//删除金钱
		if(req->money_need > 0)
		{
			pImp->SpendMoney(req->money_need);
			pImp->_runner->spend_money(req->money_need);
		}

		return true;
	}
};

class forget_pet_skill_provider : public general_provider
{
public:
	struct request
	{
		int money_need;
		int item_need;
		int skill_id;
	};

private:
	virtual bool Save(archive & ar)
	{
		//未做
		ASSERT(false);
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		//未做
		ASSERT(false);
		return true;
	}
	int _money_need;
	int _item_need;
public:

	forget_pet_skill_provider()
	{
	}

private:
	virtual forget_pet_skill_provider * Clone()
	{
		ASSERT(!_is_init);
		return new forget_pet_skill_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size != sizeof(int) + sizeof(int))
		{
			ASSERT(false);
			return false;
		}
		_money_need = ((int *)buf)[0];
		_item_need = ((int*)buf)[1];
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->money_need <_money_need)
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
			return;
		}

		request reply = *req;
		reply.money_need = _money_need;
		reply.item_need = _item_need;

		//发回回应数据
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,&reply,sizeof(reply));
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class forget_pet_skill_executor : public service_executor
{
public:
	struct player_request
	{
		int skill_id;
	};
	typedef forget_pet_skill_provider::request request;

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		if(size != sizeof(player_request)) return false;
		player_request *preq = (player_request*)buf;

		request req;
		req.skill_id = preq->skill_id;
		req.item_need = 0;
		req.money_need = pImp->GetMoney();
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&req, sizeof(req));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(request));
		request * req = (request*) buf;

		if(req->money_need > 0)
		{
			if(pImp->GetMoney() < (size_t)req->money_need)
			{
				pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
				return false;
			}
		}

		item_list & inv = pImp->GetInventory();
		int item_index = -1;
		if(req->item_need && (item_index = inv.Find(0,req->item_need)) < 0)
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		//遗忘技能
		if(!pImp->ForgetPetSkill(req->skill_id))
		{
			return false;
		}

		GLog::log(GLOG_INFO,"用户%d让宠物遗忘了技能%d",pImp->_parent->ID.id, req->skill_id);
		
		//删除物品
		if(item_index >= 0)
		{
			item& it = inv[item_index];
			pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);

			inv.DecAmount(item_index,1);
			pImp->_runner->use_item(gplayer_imp::IL_INVENTORY,item_index, req->item_need,1);
		}

		//删除金钱
		if(req->money_need > 0)
		{
			pImp->SpendMoney(req->money_need);
			pImp->_runner->spend_money(req->money_need);
		}

		return true;
	}
};

class pet_skill_provider : public general_id_provider
{

public:
	struct request
	{
		int skill_id;
	};

private:
	pet_skill_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new pet_skill_provider(*this);
	}
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		int id = req->skill_id;
		if(std::binary_search(_list.begin(),_list.end(),id))
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class pet_skill_executor : public service_executor
{

public:
	typedef pet_skill_provider::request player_request;
	pet_skill_executor(){}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*) buf;
		int rst = pImp->LearnPetSkill(req->skill_id);
		if(rst <= 0)
		{
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d让宠物学习了技能%d(rst:%d)",pImp->_parent->ID.id, req->skill_id,rst);
		return true;
	}
};

//注意:绑定 销毁 解除销毁使用同一个service_provider
class bind_item_provider : public general_provider
{
public:
	struct request
	{
		int money_need;
		int item_need;
		int can_webtrade;

		int item_index;
		int item_id;
	};

private:
	virtual bool Save(archive & ar)
	{
		//未做
		ASSERT(false);
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		//未做
		ASSERT(false);
		return true;
	}
	int _money_need;
	int _can_webtrade;
	abase::vector<int, abase::fast_alloc<> > _item_need_list;
public:

	bind_item_provider()
	{
	}

private:
	virtual bind_item_provider * Clone()
	{
		ASSERT(!_is_init);
		return new bind_item_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size % sizeof(int) != 0 || size <= sizeof(int) + sizeof(int))
		{
			ASSERT(false);
			return false;
		}
		_money_need = ((int *)buf)[0];
		_can_webtrade = ((int *)buf)[1];
		for(size_t i = 2 ; i < size/sizeof(int); i ++)
		{
			int item_id = ((int *)buf)[i];
			if(item_id) _item_need_list.push_back(item_id);	
		}
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->money_need <_money_need)
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_OUT_OF_FUND,NULL,0);
			return;
		}

		//如果list为空，表示此服务不需要消耗物品
		if(_item_need_list.size() && std::find(_item_need_list.begin(), _item_need_list.end(), req->item_need) == _item_need_list.end())
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
			return;
		}

		request reply = *req;
		reply.money_need = _money_need;
		reply.can_webtrade = _can_webtrade;

		//发回回应数据
		SendMessage(GM_MSG_SERVICE_DATA,player,_type,&reply,sizeof(reply));
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class bind_item_executor : public service_executor
{
public:
	struct player_request
	{
		int item_index;
		int item_id;
		int item_need;
	};
	typedef bind_item_provider::request request;

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request *preq = (player_request*)buf;
		
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_REFINE))
		{       
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		request req;
		req.item_need = preq->item_need;
		req.can_webtrade = 0;
		req.money_need = pImp->GetMoney();
		req.item_index = preq->item_index;
		req.item_id = preq->item_id;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&req, sizeof(req));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(request));
		request * req = (request*) buf;

		if(req->money_need > 0)
		{
			if(pImp->GetMoney() < (size_t)req->money_need)
			{
				pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
				return false;
			}
		}

		if(!pImp->CheckItemBindCondition(req->item_index, req->item_id,req->can_webtrade))
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		item_list & inv = pImp->GetInventory();
		int item_index = -1;
		if(req->item_need && (item_index = inv.Find(0,req->item_need)) < 0)
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_REFINE,REFINE_COOLDOWN_TIME);

		//进行绑定操作
		if(!pImp->BindItem(req->item_index, req->item_id, req->can_webtrade))
		{
			//这个错误一般都不会出现的
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d对物品%d执行了绑定操作,寻宝网交易标志%d",pImp->_parent->ID.id, req->item_id, req->can_webtrade);

		
		//删除需求物品
		if(item_index >= 0)
		{
			item& it = inv[item_index];
			pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);

			inv.DecAmount(item_index,1);
			pImp->_runner->use_item(gplayer_imp::IL_INVENTORY,item_index, req->item_need,1);
		}

		//删除金钱
		if(req->money_need > 0)
		{
			pImp->SpendMoney(req->money_need);
			pImp->_runner->spend_money(req->money_need);
		}

		return true;
	}
};

typedef bind_item_provider destory_bind_item_provider;
class destory_bind_item_executor : public service_executor
{
public:
	struct player_request
	{
		int item_index;
		int item_id;
	};
	typedef destory_bind_item_provider::request request;

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request *preq = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//检查物品是否能够进行绑定销毁
		if(!pImp->CheckBindItemDestory(preq->item_index, preq->item_id))
		{
			return false;
		}

		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_REFINE))
		{       
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		request req;
		req.item_need = 0;
		req.money_need = pImp->GetMoney();
		req.item_index = preq->item_index;
		req.item_id = preq->item_id;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&req, sizeof(req));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(request));
		request * req = (request*) buf;

		if(req->money_need > 0)
		{
			if(pImp->GetMoney() < (size_t)req->money_need)
			{
				pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
				return false;
			}
		}

		if(!pImp->CheckBindItemDestory(req->item_index, req->item_id))
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		item_list & inv = pImp->GetInventory();
		int item_index = -1;
		if(req->item_need && (item_index = inv.Find(0,req->item_need)) < 0)
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}


		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_REFINE,REFINE_COOLDOWN_TIME);

		//进行摧毁操作
		if(!pImp->DestoryBindItem(req->item_index, req->item_id))
		{
			//这个错误一般都不会出现的
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d对绑定物品%d执行了销毁操作",pImp->_parent->ID.id, req->item_id);

		
		//删除需求物品
		if(item_index >= 0)
		{
			item& it = inv[item_index];
			pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
			
			inv.DecAmount(item_index,1);
			pImp->_runner->use_item(gplayer_imp::IL_INVENTORY,item_index, req->item_need,1);
		}

		//删除金钱
		if(req->money_need > 0)
		{
			pImp->SpendMoney(req->money_need);
			pImp->_runner->spend_money(req->money_need);
		}

		return true;
	}
};

typedef bind_item_provider destory_item_restore_provider;
class destory_item_restore_executor : public service_executor
{
public:
	struct player_request
	{
		int item_index;
		int item_id;
	};
	typedef destory_item_restore_provider::request request;

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request *preq = (player_request*)buf;

		//检查物品是否能够进行绑定销毁
		if(!pImp->CheckRestoreDestoryItem(preq->item_index, preq->item_id))
		{
			return false;
		}

		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_REFINE))
		{       
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		request req;
		req.item_need = 0;
		req.money_need = pImp->GetMoney();
		req.item_index = preq->item_index;
		req.item_id = preq->item_id;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,&req, sizeof(req));
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(request));
		request * req = (request*) buf;

		if(req->money_need > 0)
		{
			if(pImp->GetMoney() < (size_t)req->money_need)
			{
				pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
				return false;
			}
		}

		if(!pImp->CheckRestoreDestoryItem(req->item_index, req->item_id))
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		item_list & inv = pImp->GetInventory();
		int item_index = -1;
		if(req->item_need && (item_index = inv.Find(0,req->item_need)) < 0)
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_REFINE,REFINE_COOLDOWN_TIME);


		//进行恢复操作
		if(!pImp->RestoreDestoryItem(req->item_index, req->item_id))
		{
			//这个错误一般不会出现
			pImp->_runner->error_message(S2C::ERR_CAN_NOT_STOP_DESTROY_BIND_ITEM);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d对待销毁物品执行了还原操作，还原出%d",pImp->_parent->ID.id, inv[req->item_index].type);

		
		//删除需求物品
		if(item_index >= 0)
		{
			item& it = inv[item_index];
			pImp->UpdateMallConsumptionDestroying(it.type, it.proc_type, 1);
			
			inv.DecAmount(item_index,1);
			pImp->_runner->use_item(gplayer_imp::IL_INVENTORY,item_index, req->item_need,1);
		}

		//删除金钱
		if(req->money_need > 0)
		{
			pImp->SpendMoney(req->money_need);
			pImp->_runner->spend_money(req->money_need);
		}

		return true;
	}
};

typedef feedback_provider stock_service_provider;

class stock_service_executor2 : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:

	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;

		//检查是否可以开始
		if(world_manager::GetWorldParam().forbid_cash_trade)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		if(world_manager::GetWorldParam().forbid_cash_trade)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		player_request * req = (player_request*)buf;
		//发送请求数据
		if(!GNET::ForwardStockCmd(req->service_id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		return true;
	}
};


class stock_service_executor1 : public service_executor
{
public:
	struct player_request
	{
		int withdraw;
		int cash;
		int money;
	};
private:

	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;

		//检查是否可以开始
		if(world_manager::GetWorldParam().forbid_cash_trade)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		if(world_manager::GetWorldParam().forbid_cash_trade)
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		player_request * req = (player_request*)buf;
		//发送请求数据
		if(!GNET::SendStockTransaction(object_interface(pImp),req->withdraw, req->cash, req->money))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		return true;
	}
};

typedef feedback_provider dye_service_provider;

class dye_service_executor : public service_executor
{
public:
	struct player_request
	{
		int inv_index;		//要染色的时装在包裹中的位置 
		int item_type;		//时装的id
		int dye_index;		//染色材料的位置 
		int dye_type;		//染色剂的id
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//价格冷却限制一下
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_DYE)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		if(!pImp->IsItemExist(req->inv_index,req->item_type, 1)) return false;
		if(!pImp->IsItemExist(req->dye_index,req->dye_type, 1)) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		if(!pImp->IsItemExist(req->inv_index,req->item_type, 1)) return false;
		if(!pImp->IsItemExist(req->dye_index,req->dye_type, 1)) return false;

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_DYE,REFINE_COOLDOWN_TIME);
		if(int rst = pImp->DyeFashionItem(req->inv_index,req->dye_index))
		{
			//无法进行精炼
			pImp->_runner->error_message(rst);
		}

		return true;
	}
};

typedef feedback_provider refine_transmit_service_provider;

class refine_transmit_service_executor : public service_executor
{
public:
	struct player_request
	{
		int src_index;		//转出装备的索引
		int src_id;		//转出装备的ID
		int dest_index;		//目标装备的索引
		int dest_id;		//目标装备的ID
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//冷却限制一下
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_REFINE_TRANSMIT)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		if(req->src_index == req->dest_index) return false;
		if(!pImp->IsItemExist(req->src_index,req->src_id, 1)) return false;
		if(!pImp->IsItemExist(req->dest_index,req->dest_id, 1)) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		if(req->src_index == req->dest_index) return false;
		if(!pImp->IsItemExist(req->src_index,req->src_id, 1)) return false;
		if(!pImp->IsItemExist(req->dest_index,req->dest_id, 1)) return false;

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_REFINE_TRANSMIT,1000);
		if(int rst = pImp->RefineTransmit(req->src_index,req->dest_index))
		{
			//无法进行精炼
			pImp->_runner->error_message(rst);
		}

		return true;
	}
};

class  make_slot_executor: public service_executor
{
public:
	struct player_request
	{
		int src_index;		//要打孔装备的索引
		int src_id;		//要打孔装备的id
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		if(!pImp->IsItemExist(req->src_index,req->src_id, 1)) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		if(!pImp->IsItemExist(req->src_index,req->src_id, 1)) return false;
		pImp->ItemMakeSlot(req->src_index, req->src_id);
		return true;
	}
};

//lgc
typedef feedback_provider elf_dec_attribute_provider;
class elf_dec_attribute_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char inv_idx_elf;	//小精灵在包裹中位置
		unsigned char inv_idx_ticket; //洗点道具在包裹中位置
		short str;
		short agi;
		short vit;
		short eng;
	};
#pragma pack()
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		
		//这里只做简单检查
		if(req->str < 0 || req->agi < 0 || req->vit < 0 || req->eng < 0)
			return false;
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	virtual bool OnServe(gplayer_imp * pImp, const XID & provider, const A3DVECTOR & pos, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		
		pImp->ElfDecAttribute(req->inv_idx_elf, req->inv_idx_ticket, req->str, req->agi, req->vit, req->eng);
		return true;
	}	

};



typedef feedback_provider elf_flush_genius_provider;
class elf_flush_genius_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char inv_idx_elf;	//小精灵在包裹中位置
		unsigned char inv_idx_ticket; //洗点道具在包裹中位置
	};
#pragma pack()
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	virtual bool OnServe(gplayer_imp * pImp, const XID & provider, const A3DVECTOR & pos, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		
		pImp->ElfFlushGenius(req->inv_idx_elf, req->inv_idx_ticket);
		return true;
	}	
	
};


class elf_learn_skill_provider : public general_id_provider
{

public:
#pragma pack(1)
	struct request
	{
		unsigned char inv_idx_elf;//小精灵在包裹中索引
		unsigned short skill_id;
	};
#pragma pack()
private:
	elf_learn_skill_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new elf_learn_skill_provider(*this);
	}
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		ASSERT(size == sizeof(request));
		request * req = (request*)buf;
		int id = req->skill_id;
		if(std::binary_search(_list.begin(),_list.end(),id))
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class elf_learn_skill_executor : public service_executor
{

public:
	typedef elf_learn_skill_provider::request player_request;
	elf_learn_skill_executor(){}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*) buf;
		pImp->ElfLearnSkill(req->inv_idx_elf, req->skill_id);
		return true;
	}
};

typedef feedback_provider elf_forget_skill_provider;
class elf_forget_skill_executor : public service_executor
{
public:
#pragma pack(1)
	struct  player_request
	{	
		unsigned char inv_index;//小精灵在包裹中的索引
		unsigned short skill_id;
		short forget_level;
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		
		player_request * req = (player_request*)buf;
		if(req->forget_level <= 0) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;
		ASSERT(req->forget_level > 0);
		pImp->ElfForgetSkill(req->inv_index, req->skill_id, req->forget_level);
		return true;
	}
};

typedef feedback_provider elf_refine_provider;
class elf_refine_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char inv_idx_elf;	//小精灵在包裹中的位置
		unsigned char inv_idx_ticket;		//精炼道具在包裹中的位置
		short ticket_cnt;					//精炼道具的数目只有梦幻丹可以放多个，其他只能一个
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;

		//价格冷却限制一下
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_REFINE)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		/*if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}*/

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_REFINE,REFINE_COOLDOWN_TIME);
		pImp->ElfRefine(req->inv_idx_elf, req->inv_idx_ticket, req->ticket_cnt);
		return true;
	}
};

typedef feedback_provider elf_refine_transmit_provider;
class elf_refine_transmit_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char src_index;		//转出装备的索引
		unsigned char dest_index;		//目标装备的索引
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		//冷却限制一下
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_REFINE_TRANSMIT)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		/*if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}*/

		if(req->src_index == req->dest_index) return false;
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		if(req->src_index == req->dest_index) return false;

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_REFINE_TRANSMIT,1000);
		pImp->ElfRefineTransmit(req->src_index,req->dest_index);
		return true;
	}
};


typedef feedback_provider elf_decompose_provider;
class elf_decompose_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char inv_index;		//小精灵在包裹栏的索引
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		
		/*if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}*/
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;

		pImp->ElfDecompose(req->inv_index);
		return true;
	}
};


typedef feedback_provider elf_destroy_item_provider;
class elf_destroy_item_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char inv_index;	//小精灵在包裹栏的索引
		unsigned char mask;			//要销毁的小精灵的装备的位置mask
		unsigned char equip_index;	//准备替换的小精灵装备在包裹栏中的索引,255则为只销毁装备
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		if(req->mask != 0x01 && req->mask != 0x02 && req->mask != 0x04 && req->mask != 0x08)
			return false;

		/*if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}*/
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		if(req->mask != 0x01 && req->mask != 0x02 && req->mask != 0x04 && req->mask != 0x08)
			return false;

		//冷却
		pImp->ElfDestroyItem(req->inv_index,req->mask,req->equip_index);
		return true;
	}
};

typedef feedback_provider dye_suit_service_provider;

class dye_suit_service_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char inv_idx_body;		//要染色的衣服在包裹中的位置,255表示衣服不染色
		unsigned char inv_idx_leg;		//要染色的裤子在包裹中的位置 ,255表示裤子不染色
		unsigned char inv_idx_foot;		//要染色的鞋在包裹中的位置 ,255表示鞋不染色
		unsigned char inv_idx_wrist;	//要染色的手套在包裹中的位置 ,255表示手套不染色
		unsigned char inv_idx_dye;		//染色材料的位置 
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		//player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_DYE)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_DYE,REFINE_COOLDOWN_TIME);
		if(int rst = pImp->DyeSuitFashionItem(req->inv_idx_body,req->inv_idx_leg,req->inv_idx_foot,req->inv_idx_wrist,req->inv_idx_dye))
		{
			pImp->_runner->error_message(rst);
		}
		return true;
	}
};

typedef feedback_provider repair_damaged_item_provider;
class repair_damaged_item_executor: public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char inv_idx;	//要修理的损毁物品在包裹中的位置
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		
		player_request * req = (player_request*)buf;
		if(req->inv_idx >= pImp->GetInventory().Size()) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		
		player_request * req = (player_request*)buf;
		if(req->inv_idx >= pImp->GetInventory().Size()) return false;

		pImp->RepairDamagedItem(req->inv_idx);
		return true;
	}
};

class produce3_provider : public general_provider	//升级生产
{
public:
#pragma pack(1)
	struct request
	{
		int skill;
		int recipe_id;
		int materials_id[16];
		int idxs[16];
		int equip_id;		//待升级装备id
		int equip_inv_idx;	//待升级装备在包裹栏中位置
		char inherit_type;	//bit0:是否继承精练，bit1:是否继承孔数，bit2:是否继承宝石,如继承宝石必须继承孔数
	};
#pragma pack()

private:
	float _tax_rate;
	int _skill_id;
	abase::vector<int> _target_list;	//可生产的目标列表

	virtual bool Save(archive & ar)
	{
		ar << _tax_rate << _skill_id;
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate >> _skill_id;
		return true;
	}

public:
	produce3_provider():_tax_rate(1.f),_skill_id(-1)
	{}
private:
	virtual produce3_provider * Clone()
	{
		ASSERT(!_is_init);
		return new produce3_provider(*this);
	}
	virtual bool OnInit(const void * buf, size_t size)
	{
		typedef npc_template::npc_statement::__service_produce  DATA;
		if(size != sizeof(DATA))
		{
			ASSERT(false);
			return false;
		}
		DATA * t = (DATA*)buf;
		_skill_id = t->produce_skill;
		_target_list.reserve(t->produce_num);
		for(int i  = 0; i < t->produce_num; i ++)
		{
			_target_list.push_back(t->produce_items[i]);
			ASSERT(t->produce_items[i]);
		}
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->skill == _skill_id)
		{
			//检查是否能生产
			for(size_t i = 0; i < _target_list.size(); i ++)
			{
				if(_target_list[i] == req->recipe_id)
				{
					//找到了合适的物品
					SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
					return;
				}
			}
		}

		SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class produce3_executor : public service_executor  //升级生产
{

public:
	typedef produce3_provider::request player_request;
	produce3_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//校验物品配方是否存在
		const recipe_template *rt = recipe_manager::GetRecipe(req->recipe_id);
		if(!rt) return false;
		if(rt->equipment_need_upgrade <= 0) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		//校验配方技能和级别是否匹配
		if(rt->produce_skill != req->skill || 
				rt->produce_skill > 0 && pImp->GetSkillLevel(rt->produce_skill) < rt->require_level) return false;

		//检查金钱是否足够
		if(pImp->GetMoney() < rt->fee) 
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return true;	//只报告金钱不够的错误
		}
		
		//检查待升级物品
		if(!pImp->IsItemExist(req->equip_inv_idx, req->equip_id, 1)) return false; 

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return false;
		}
		//	加入制造的session
		const recipe_template *rt = recipe_manager::GetRecipe(req->recipe_id);
		if(!rt) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		pImp->AddSession(new session_produce3(pImp,rt,req->materials_id, req->idxs, req->equip_id, req->equip_inv_idx, req->inherit_type));
		pImp->StartSession();
		return true;
	}
	
};

class produce4_provider : public general_provider	//新继承生产
{
public:
#pragma pack(1)
	struct request
	{
		int skill;
		int recipe_id;
		int materials_id[16];
		int idxs[16];
		int equip_id;		//待升级装备id
		int equip_inv_idx;	//待升级装备在包裹栏中位置
		char inherit_type;	//bit0:是否继承精练，bit1:是否继承孔数，bit2:是否继承宝石,如继承宝石必须继承孔数
	};
#pragma pack()

private:
	float _tax_rate;
	int _skill_id;
	abase::vector<int> _target_list;	//可生产的目标列表

	virtual bool Save(archive & ar)
	{
		ar << _tax_rate << _skill_id;
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate >> _skill_id;
		return true;
	}

public:
	produce4_provider():_tax_rate(1.f),_skill_id(-1)
	{}
private:
	virtual produce4_provider * Clone()
	{
		ASSERT(!_is_init);
		return new produce4_provider(*this);
	}
	virtual bool OnInit(const void * buf, size_t size)
	{
		typedef npc_template::npc_statement::__service_produce  DATA;
		if(size != sizeof(DATA))
		{
			ASSERT(false);
			return false;
		}
		DATA * t = (DATA*)buf;
		_skill_id = t->produce_skill;
		_target_list.reserve(t->produce_num);
		for(int i  = 0; i < t->produce_num; i ++)
		{
			_target_list.push_back(t->produce_items[i]);
			ASSERT(t->produce_items[i]);
		}
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->skill == _skill_id)
		{
			//检查是否能生产
			for(size_t i = 0; i < _target_list.size(); i ++)
			{
				if(_target_list[i] == req->recipe_id)
				{
					//找到了合适的物品
					SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
					return;
				}
			}
		}

		SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class produce4_executor : public service_executor  //新技能生产
{

public:
	typedef produce4_provider::request player_request;
	produce4_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//校验物品配方是否存在
		const recipe_template *rt = recipe_manager::GetRecipe(req->recipe_id);
		if(!rt) return false;
		if(rt->equipment_need_upgrade <= 0) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		//校验配方技能和级别是否匹配
		if(rt->produce_skill != req->skill || 
				rt->produce_skill > 0 && pImp->GetSkillLevel(rt->produce_skill) < rt->require_level) return false;

		//检查金钱是否足够
		if(pImp->GetMoney() < rt->fee) 
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return true;	//只报告金钱不够的错误
		}
		
		//检查待升级物品
		if(!pImp->IsItemExist(req->equip_inv_idx, req->equip_id, 1)) return false; 

		//检查玩家是不是已经有session了
		if (pImp->HasSession())
			return false;

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		//检查玩家是不是已经有session了
		if (pImp->HasSession())
			return false;

		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return false;
		}
		//	加入制造的session
		const recipe_template *rt = recipe_manager::GetRecipe(req->recipe_id);
		if(!rt) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		pImp->AddSession(new session_produce4(pImp,rt,req->materials_id, req->idxs, req->equip_id, req->equip_inv_idx, req->inherit_type));
		pImp->StartSession();
		return true;
	}
	
};

class produce5_provider : public general_provider	//升级生产
{
public:
#pragma pack(1)
	struct request
	{
		int skill;
		int recipe_id;
		int materials_id[16];
		int idxs[16];
		int equip_id;		//待升级装备id
		int equip_inv_idx;	//待升级装备在包裹栏中位置
		char inherit_type;	//bit0:是否继承精练，bit1:是否继承孔数，bit2:是否继承宝石,如继承宝石必须继承孔数, bit3是否继承镌刻，bit4是否继承附加属性
	};
#pragma pack()

private:
	float _tax_rate;
	int _skill_id;
	abase::vector<int> _target_list;	//可生产的目标列表

	virtual bool Save(archive & ar)
	{
		ar << _tax_rate << _skill_id;
		return true;
	}
	virtual bool Load(archive & ar) 
	{
		ar >> _tax_rate >> _skill_id;
		return true;
	}

public:
	produce5_provider():_tax_rate(1.f),_skill_id(-1)
	{}
private:
	virtual produce5_provider * Clone()
	{
		ASSERT(!_is_init);
		return new produce5_provider(*this);
	}
	virtual bool OnInit(const void * buf, size_t size)
	{
		typedef npc_template::npc_statement::__service_produce  DATA;
		if(size != sizeof(DATA))
		{
			ASSERT(false);
			return false;
		}
		DATA * t = (DATA*)buf;
		_skill_id = t->produce_skill;
		_target_list.reserve(t->produce_num);
		for(int i  = 0; i < t->produce_num; i ++)
		{
			_target_list.push_back(t->produce_items[i]);
			ASSERT(t->produce_items[i]);
		}
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->skill == _skill_id)
		{
			//检查是否能生产
			for(size_t i = 0; i < _target_list.size(); i ++)
			{
				if(_target_list[i] == req->recipe_id)
				{
					//找到了合适的物品
					SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
					return;
				}
			}
		}

		SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SKILL_NOT_AVAILABLE,NULL,0);
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class produce5_executor : public service_executor  //升级生产
{

public:
	typedef produce5_provider::request player_request;
	produce5_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;
		
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//校验物品配方是否存在
		const recipe_template *rt = recipe_manager::GetRecipe(req->recipe_id);
		if(!rt) return false;
		if(rt->equipment_need_upgrade <= 0) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		//校验配方技能和级别是否匹配
		if(rt->produce_skill != req->skill || 
				rt->produce_skill > 0 && pImp->GetSkillLevel(rt->produce_skill) < rt->require_level) return false;

		//检查金钱是否足够
		if(pImp->GetMoney() < rt->fee) 
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return true;	//只报告金钱不够的错误
		}
		
		//检查待升级物品
		if(!pImp->IsItemExist(req->equip_inv_idx, req->equip_id, 1)) return false; 

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return false;
		}
		//	加入制造的session
		const recipe_template *rt = recipe_manager::GetRecipe(req->recipe_id);
		if(!rt) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, rt->recipe_id)) return false;

		pImp->AddSession(new session_produce5(pImp,rt,req->materials_id, req->idxs, req->equip_id, req->equip_inv_idx, req->inherit_type));
		pImp->StartSession();
		return true;
	}
	
};

typedef feedback_provider equip_signature_provider;
class equip_signature_executor : public service_executor  //装备改签名
{

public:
#pragma pack(1)
	struct player_request
	{
		int ink_inv_idx;		//染色剂在背包栏的索引
		int ink_id;						//染色剂id
		int equip_inv_idx;		//需要修改签名的装备在背包栏的索引
		int equip_id;					//需要修改签名的装备id
		unsigned short signature_len;	//要修改的签名长度
		char signature[];				//要修改的签名
	};
#pragma pack()
	equip_signature_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		
		player_request * req = (player_request*)buf;

		size_t signature_len = req->signature_len;
		if(signature_len > 38 ||
			(signature_len & 0x01) || signature_len + sizeof(player_request) != size) 
		{
			pImp->_runner->error_message(S2C::ERR_EQUIP_SIGNATURE_WRONG);
			return true;
		}

		//检测字符是否包含屏蔽字符
		if (signature_len != 0) 
		{
			//remove all spaces
			abase::octets signature_nospace(req->signature, req->signature_len);
			short *pChar =  (short*)signature_nospace.begin();
			while(*pChar != 0 && pChar != (short *)signature_nospace.end())
			{
				if (32 == *pChar) signature_nospace.erase((unsigned char*)pChar,(unsigned char*)(pChar+1));	//remove space
				else ++pChar;
			}

			if (signature_nospace.size() == 0 || GMSV::CheckMatcher((char*)signature_nospace.begin(), signature_nospace.size()))
			{
				pImp->_runner->error_message(S2C::ERR_EQUIP_SIGNATURE_WRONG);
				return true;
			}
		}
		
		//检测墨水是否存在
		unsigned int ink_num = signature_len/2 > EQUIP_SIGNATURE_CLEAN_CONSUME ? signature_len/2 : EQUIP_SIGNATURE_CLEAN_CONSUME;
		if(!pImp->IsItemExist(req->ink_inv_idx, req->ink_id, ink_num)) return false; 

		//检测需要更改签名的装备是否存在
		if(!pImp->IsItemExist(req->equip_inv_idx, req->equip_id, 1)) return false;
		
		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}

	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size < sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;

		size_t signature_len = req->signature_len;
		if (signature_len > 38 ||
			(signature_len & 0x01) || signature_len + sizeof(player_request) != size)
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}

		return pImp->EquipSign(req->ink_inv_idx, req->ink_id, req->equip_inv_idx, req->equip_id, req->signature, signature_len);
	}
};

typedef feedback_provider player_rename_provider;
class player_rename_executor : public service_executor  //玩家付费改名
{

public:
#pragma pack(1)
	struct player_request
	{
		int item_inv_idx;				//消耗物品在背包栏的索引
		int item_id;					//消耗物品id
		unsigned short new_name_len;	//要修改之后的名字长度
		char new_name[];				//要修改之后的名字
	};
#pragma pack()
	player_rename_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		
		player_request * req = (player_request*)buf;

		//名字长度和消息长度是否合法
		size_t new_name_len = req->new_name_len;
		if(new_name_len > MAX_USERNAME_LENGTH || new_name_len == 0 ||
			(new_name_len & 0x01) || new_name_len + sizeof(player_request) != size) 
		{
			pImp->_runner->error_message(S2C::ERR_PLAYER_RENAME);
			return true;
		}

		//检查是否可以开始
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		
		//是否处于可以交易状态
		if(!pImp->CanTrade(provider))
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return true;
		}

		//检测消耗物品是否存在
		if(!pImp->IsItemExist(req->item_inv_idx, req->item_id, 1)) return false; 
		
		//是否是指定物品
		if (req->item_id != PLAYER_RENAME_ITEM_ID && req->item_id != PLAYER_RENAME_ITEM_ID_2) return false;
		
		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}

	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size < sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;
		
		//名字长度和消息长度是否合法
		size_t new_name_len = req->new_name_len;
		if (new_name_len > MAX_USERNAME_LENGTH || new_name_len == 0 ||
			(new_name_len & 0x01) || new_name_len + sizeof(player_request) != size)
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		
		//检查是否可以开始
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		//是否处于可以交易状态
		if(!pImp->CanTrade(provider))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}

		//检测消耗物品是否存在
		if(!pImp->IsItemExist(req->item_inv_idx, req->item_id, 1)) return false; 

		//是否是指定物品
		if (req->item_id != PLAYER_RENAME_ITEM_ID && req->item_id != PLAYER_RENAME_ITEM_ID_2) return false;
		
		// 发消息给gdeliveryd确认修改名字，走邮件的流程，玩家进入交易状态
		object_interface oi(pImp);
		GMSV::SendPlayerRename(pImp->_parent->ID.id, req->item_inv_idx, req->item_id, 1, req->new_name, req->new_name_len, oi);
		return true;
	}
};

typedef feedback_provider user_trashbox_open_provider;
class user_trashbox_open_executor : public service_executor
{
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != 0) return false;

		if(pImp->InCentralServer()) return false;

		if(pImp->HasSession())
		{
			pImp->_runner->error_message(S2C::ERR_OTHER_SESSION_IN_EXECUTE);
			return true;
		}

		//考虑添加重试计数策略 
		//不考虑钱数
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,0,0);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(pImp->InCentralServer()) return false;

		//开启钱箱
		//加入当前的
		if(!pImp->HasSession())
		{	
			__PRINTF("芝麻开门啦、\n");
			session_use_user_trashbox *pSession = new session_use_user_trashbox(pImp);
			pImp->AddSession(pSession);
			pImp->StartSession();
		}
		return true;
	}
};

typedef feedback_provider webtrade_service_provider;
class webtrade_service_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;
		
		//检查是否可以开始
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		player_request * req = (player_request*)buf;
		//发送请求数据
		if(!GNET::ForwardWebTradeSysOP(req->service_id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了网上交易操作",pImp->_parent->ID.id);
		return true;
	}
};

typedef feedback_provider god_evil_convert_service_provider;
class god_evil_convert_service_executor: public service_executor
{
public:
#pragma pack(1)
    struct player_request
    {
		unsigned char mode;
	};
#pragma pack()

private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		//防止此刻正在施放技能	
		if(pImp->HasSession())
		{
			pImp->_runner->error_message(S2C::ERR_OTHER_SESSION_IN_EXECUTE);
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	virtual bool OnServe(gplayer_imp * pImp, const XID & provider, const A3DVECTOR & pos, const void * buf, size_t size)
	{
        if(size != sizeof(player_request)) return false;
        player_request* req = (player_request *)buf;

		//防止此刻正在施放技能	
		if(pImp->HasSession())
		{
			pImp->_runner->error_message(S2C::ERR_OTHER_SESSION_IN_EXECUTE);
			return false;
		}

		pImp->GodEvilConvert(req->mode);
		return true;
	}	
	
};

class wedding_book_provider: public feedback_provider
{
	virtual wedding_book_provider* Clone()
	{
		ASSERT(!_is_init);
		return new wedding_book_provider(*this);
	}

	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		world_manager::GetInstance()->WeddingSendBookingList(player.id, cs_index, sid);	
		return;
	}
};

class wedding_book_executor: public service_executor
{
public:
	enum
	{
		TYPE_BOOK = 1,
		TYPE_CANCELBOOK,
	};
	struct player_request
	{
		int type;
		int wedding_start_time;
		int wedding_end_time;
		int wedding_scene;
		int bookcard_index;
	};
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;

		if(req->type != TYPE_BOOK && req->type != TYPE_CANCELBOOK) return false;
		if(req->wedding_scene < 0 || req->wedding_scene >= WEDDING_SCENE_COUNT) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	virtual bool OnServe(gplayer_imp * pImp, const XID & provider, const A3DVECTOR & pos, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		
		int ret = 0;
		if(req->type == TYPE_BOOK)
			ret = pImp->WeddingBook(req->wedding_start_time, req->wedding_end_time, req->wedding_scene, req->bookcard_index);
		else if(req->type == TYPE_CANCELBOOK)
			ret = pImp->WeddingCancelBook(req->wedding_start_time, req->wedding_end_time, req->wedding_scene);
		if(ret)
			pImp->_runner->error_message(ret);	
		return true;
	}	
};

typedef feedback_provider wedding_invite_provider;
class wedding_invite_executor: public service_executor
{
public:
	struct player_request
	{
		int invitecard_index;	//结婚者使用的请柬的索引
		int invitee;
	};
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;

		if(req->invitecard_index < 0 || (size_t)req->invitecard_index >= pImp->GetInventory().Size()) return false;
		if(req->invitee < 0) return false;

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	virtual bool OnServe(gplayer_imp * pImp, const XID & provider, const A3DVECTOR & pos, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		
		int ret = pImp->WeddingInvite(req->invitecard_index, req->invitee);
		if(ret)
			pImp->_runner->error_message(ret);	
		return true;
	}	
};

typedef feedback_provider factionfortress_service_provider;
class factionfortress_service_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		player_request * req = (player_request*)buf;
		//发送请求数据
		if(!GNET::ForwardFactionFortressOP(req->service_id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了帮派基地操作",pImp->_parent->ID.id);
		return true;
	}
};

typedef feedback_provider factionfortress_service_provider2;
class factionfortress_service_executor2 : public service_executor
{
public:
	enum
	{
		RT_LEVEL_UP = 1,
		RT_SET_TECH_POINT,
		RT_CONSTRUCT,
		RT_HAND_IN_MATERIAL,
		RT_HAND_IN_CONTRIB,
		RT_LEAVE_FORTRESS,
		RT_DISMANTLE,
		RT_RESET_TECH_POINT,	//重置科技点数
		RT_MAX,
	};
	struct player_request
	{
		int type;
		int param[3];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		
		switch(req->type)
		{
			case RT_LEVEL_UP:
			case RT_SET_TECH_POINT:
			case RT_CONSTRUCT:
			case RT_DISMANTLE:
			case RT_RESET_TECH_POINT:
				if(!pImp->OI_IsMafiaMaster()) return false;
				break;
			case RT_HAND_IN_MATERIAL:
			case RT_HAND_IN_CONTRIB:
			case RT_LEAVE_FORTRESS:
				if(!pImp->OI_IsMafiaMember()) return false;
				break;
			default:
				return false;
		}
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;
		
		int ret = 0;
		switch(req->type)
		{
			case RT_LEVEL_UP:
				ret = pImp->FactionFortressLevelUp();
				break;
			case RT_SET_TECH_POINT:
				ret = pImp->FactionFortressSetTechPoint(req->param[0]);
				break;
			case RT_CONSTRUCT:
				ret = pImp->FactionFortressConstruct(req->param[0], req->param[1]);
				break;
			case RT_HAND_IN_MATERIAL:
				ret = pImp->FactionFortressHandInMaterial(req->param[0],req->param[1],req->param[2]);
				break;
			case RT_HAND_IN_CONTRIB:
				ret = pImp->FactionFortressHandInContrib(req->param[0]);
				break;
			case RT_LEAVE_FORTRESS:
				pImp->_filters.ModifyFilter(FILTER_CHECK_INSTANCE_KEY,FMID_CLEAR_AEFF,NULL,0);
				break;
			case RT_DISMANTLE:
				ret = pImp->FactionFortressDismantle(req->param[0]);
				break;
			case RT_RESET_TECH_POINT:
				ret = pImp->FactionFortressResetTechPoint(req->param[0], req->param[1], req->param[2]);
				break;
			default:
				return false;
		}
		
		if(ret > 0)
			pImp->_runner->error_message(ret);	
		
		return true;
	}
};

typedef feedback_provider factionfortress_material_exchange_provider;
class factionfortress_material_exchange_executor : public service_executor
{
public:
	struct player_request
	{
		size_t src_index;
		size_t dst_index;
		int material;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		
		if(!pImp->OI_IsMafiaMaster()) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;
		
		int ret = pImp->FactionFortressMaterialExchange(req->src_index,req->dst_index,req->material);
		
		if(ret > 0)
			pImp->_runner->error_message(ret);	
		
		return true;
	}
};

typedef feedback_provider dye_pet_service_provider;
class dye_pet_service_executor : public service_executor
{
public:
	struct player_request
	{
		size_t pet_index;		//要染色的宠物在宠物栏的位置
		int pet_tid;		//宠物的模板id
		size_t dye_index;		//染色材料的位置 
		int dye_type;		//染色剂的id
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//价格冷却限制一下
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_DYE)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return true;
		}

		if(!pImp->IsPetExist(req->pet_index,req->pet_tid)) return false;
		if(!pImp->IsItemExist(req->dye_index,req->dye_type, 1)) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));

		//检查是否开始
		player_request * req = (player_request*)buf;
		if(!pImp->IsPetExist(req->pet_index,req->pet_tid)) return false;
		if(!pImp->IsItemExist(req->dye_index,req->dye_type, 1)) return false;

		//冷却
		pImp->SetCoolDown(COOLDOWN_INDEX_DYE,REFINE_COOLDOWN_TIME);
		if(int rst = pImp->DyePet(req->pet_index,req->dye_index))
		{
			//无法进行精炼
			pImp->_runner->error_message(rst);
		}

		return true;
	}
};

typedef feedback_provider trashbox_open_view_only_provider;	//无需仓库密码,即可查看仓库的物品,用于客服查看
class trashbox_open_view_only_executor : public service_executor
{
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != 0) return false;

		if(pImp->HasSession())
		{
			pImp->_runner->error_message(S2C::ERR_OTHER_SESSION_IN_EXECUTE);
			return true;
		}
		
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,0,0);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(!pImp->HasSession())
		{	
			__PRINTF("芝麻开门啦、\n");
			session_use_trashbox *pSession = new session_use_trashbox(pImp);
			pSession->SetViewOnly(true);
			pImp->AddSession(pSession);
			pImp->StartSession();
		}
		return true;
	}
};

class engrave_provider : public general_id_provider
{

public:
	struct request
	{
		int recipe_id;
		size_t inv_index;
	};
private:
	engrave_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new engrave_provider(*this);
	}
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		ASSERT(size == sizeof(request));
		request * req = (request*)buf;
		int id = req->recipe_id;
		if(std::binary_search(_list.begin(),_list.end(),id))
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class engrave_executor : public service_executor
{

public:
	typedef engrave_provider::request player_request;
	engrave_executor(){}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		
		//校验物品配方是否存在
		const engrave_recipe_template * ert = recipe_manager::GetEngraveRecipe(req->recipe_id);
		if(!ert) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, ert->recipe_id)) return false;

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*) buf;

		const engrave_recipe_template * ert = recipe_manager::GetEngraveRecipe(req->recipe_id);
		if(!ert) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, ert->recipe_id)) return false;

		//	加入制造的session
		pImp->AddSession(new session_engrave(pImp,ert,req->inv_index));
		pImp->StartSession();
		return true;
	}
};

typedef feedback_provider dpsrank_service_provider;
class dpsrank_service_executor : public service_executor
{
public:
	enum REQ_TYPE
	{
		RT_GET_RANK = 0,
		RT_MAX,
	};

#pragma pack(1)
	struct player_request
	{
		int type;
		int param;
	};
#pragma pack()

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		
		switch(req->type)
		{
			case RT_GET_RANK:
				if(!pImp->CheckCoolDown(COOLDOWN_INDEX_DPS_DPH_RANK)) return false;
			break;
			
			default:
				return false;
			break;
		}

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request *)buf;

		switch(req->type)
		{
			case RT_GET_RANK:
				pImp->PlayerGetDpsDphRank(req->param);
			break;
			
			default:
				return false;
			break;
		}

		return true;
	}
};

class addonregen_provider : public general_id_provider
{

public:
	struct request
	{
		int recipe_id;
		size_t inv_index;
	};
private:
	addonregen_provider *  Clone()
	{
		ASSERT(!_is_init);
		return new addonregen_provider(*this);
	}
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		ASSERT(size == sizeof(request));
		request * req = (request*)buf;
		int id = req->recipe_id;
		if(std::binary_search(_list.begin(),_list.end(),id))
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
	}
};

class addonregen_executor : public service_executor
{

public:
	typedef addonregen_provider::request player_request;
	addonregen_executor(){}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		
		//校验物品配方是否存在
		const addonregen_recipe_template * arrt = recipe_manager::GetAddonRegenRecipe(req->recipe_id);
		if(!arrt) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, arrt->recipe_id)) return false;

		//校验配方技能和级别是否匹配
		if(arrt->produce_skill > 0 && 
				pImp->GetSkillLevel(arrt->produce_skill) < arrt->require_level) return false;

		//检查金钱是否足够
		if(pImp->GetMoney() < arrt->fee) 
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return true;	//只报告金钱不够的错误
		}

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*) buf;

		const addonregen_recipe_template * arrt = recipe_manager::GetAddonRegenRecipe(req->recipe_id);
		if(!arrt) return false;

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, arrt->recipe_id)) return false;

		//	加入制造的session
		pImp->AddSession(new session_addonregen(pImp,arrt,req->inv_index));
		pImp->StartSession();
		return true;
	}
};
class player_force_service_provider : public general_provider
{
public:
	struct request
	{
		int type;
		int force_id;
	};
private:
	int _force_id;
	virtual bool Save(archive & ar)
	{
		ar << _force_id;
		return true;
	}
	virtual bool Load(archive & ar)
	{
		ar >> _force_id;
		return true;
	}

public:
	player_force_service_provider():_force_id(0){}

private:
	virtual player_force_service_provider * Clone()
	{
		ASSERT(!_is_init);
		return new player_force_service_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size != sizeof(int)) 
		{
			ASSERT(false);
			return false;
		}
		_force_id = *(int*)buf;
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能要返回位置列表或者税率
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}
	
	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(request))
		{
			ASSERT(false);
			return ;
		}
		request * req = (request*)buf;
		if(req->force_id == _force_id)
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{
	}
	
	virtual void OnControl(int param ,const void * buf, size_t size)
	{
		//主要是控制税率
	}
};

class player_force_service_executor : public service_executor
{
public:
	enum REQ_TYPE
	{
		RT_JOIN	= 0,
		RT_LEAVE,
		RT_MAX,
	};
	typedef player_force_service_provider::request player_request;
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;

		if(pImp->InCentralServer()) return false;

		int cur_force = pImp->_player_force.GetForce();
		switch(req->type)
		{
			case RT_JOIN:
				if(!world_manager::GetForceGlobalDataMan().IsDataReady()) return false;
				if(!pImp->CheckCoolDown(COOLDOWN_INDEX_PLAYER_JOIN_FORCE)) return false;
				if(cur_force != 0 || req->force_id == 0) return false;
			break;
			
			case RT_LEAVE:
				if(!world_manager::GetForceGlobalDataMan().IsDataReady()) return false;
				if(!pImp->CheckCoolDown(COOLDOWN_INDEX_PLAYER_LEAVE_FORCE)) return false;
				if(cur_force == 0 || req->force_id != cur_force) return false;
			break;
			
			default:
				return false;
			break;
		}

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request *)buf;

		if(pImp->InCentralServer()) return false;

		int ret = -1;
		switch(req->type)
		{
			case RT_JOIN:
				ret = pImp->PlayerJoinForce(req->force_id);
			break;

			case RT_LEAVE:
				ret = pImp->PlayerLeaveForce();
			break;
			
			default:
				return false;
			break;
		}

		if(ret > 0)
			pImp->_runner->error_message(ret);	

		return true;
	}
};

class unlimited_transmit_provider : public general_provider
{
public:
	struct request
	{
		size_t count;
		int waypoint_list[];
	};
	
	unlimited_transmit_provider():_waypoint(0){}
private:
	int _waypoint;
	virtual bool Save(archive & ar) { ar << _waypoint; return true; } 
	virtual bool Load(archive & ar) { ar >> _waypoint; return true; }

private:
	unlimited_transmit_provider*  Clone()
	{
		ASSERT(!_is_init);
		return new unlimited_transmit_provider(*this);
	}

	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size != sizeof(int) ) return false;
		_waypoint = *(int*)buf;
		ASSERT(_waypoint >= 0);
		return true;
	}
	
	virtual void GetContent(const XID & player,int cs_index, int sid)
	{
		//可能需要返回任务列表
		//SendServiceContent(player.id, cs_idnex, sid, _left_list.begin(),_left_list.size() * sizeof(int));
		return ;
	}

	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		request * req = (request*)buf;
		if(req->count < 2 || req->count > 32 || size != sizeof(request) + sizeof(int)*req->count)
		{
			ASSERT(false);
			return ;
		}
		if(req->waypoint_list[0] == _waypoint)
		{
			//找到回应
			SendMessage(GM_MSG_SERVICE_DATA,player,_type,buf,size);
		}
		else
		{
			//未找到，发送错误
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
		}
	}

	virtual void OnHeartbeat()
	{} 

	virtual void OnControl(int param ,const void * buf, size_t size)
	{}
};

class unlimited_transmit_executor: public service_executor
{
public:
	typedef unlimited_transmit_provider::request player_request;

private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		if(req->count < 2 || req->count > 32 || size != sizeof(player_request) + sizeof(int)*req->count) return false;

		//检查状态是否正确
		if(pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL) return false;

		if(!pImp->IsItemExist(UNLIMITED_TRANSMIT_SCROLL_ID1)
		 && !pImp->IsItemExist(UNLIMITED_TRANSMIT_SCROLL_ID2)
		 && !pImp->IsItemExist(UNLIMITED_TRANSMIT_SCROLL_ID3)
		 && !pImp->IsItemExist(UNLIMITED_TRANSMIT_SCROLL_ID4))
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		player_request * req = (player_request *)buf;
		if(req->count < 2 || req->count > 32 || size != sizeof(player_request) + sizeof(int)*req->count)
		{
			ASSERT(false);
			return false;
		}

		//检查状态是否正确
		if(pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL) return false;

		if(!pImp->IsItemExist(UNLIMITED_TRANSMIT_SCROLL_ID1)
		 && !pImp->IsItemExist(UNLIMITED_TRANSMIT_SCROLL_ID2)
		 && !pImp->IsItemExist(UNLIMITED_TRANSMIT_SCROLL_ID3)
		 && !pImp->IsItemExist(UNLIMITED_TRANSMIT_SCROLL_ID4))
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return false;
		}

		size_t fee = 0;
		npc_template::npc_statement::__st_ent entry;
		world_manager * manager = world_manager::GetInstance();
		for(size_t i=0; i<req->count-1; i++)
		{
			if(!manager->GetTransmitEntry(req->waypoint_list[i], req->waypoint_list[i+1], entry))
			{
				return false;
			}
			if(!pImp->IsWaypointActived( entry.target_waypoint & 0xFFFF))
			{
				pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
				return false;
			}
			if(pImp->_basic.level < entry.require_level)
			{
				pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
				return false;
			}
			fee += entry.fee;
		}

		//检查钱数
		if(pImp->GetMoney() < fee)
		{
			//钱数不够
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return false;
		}
		gplayer * pParent = pImp->GetParent();
		GLog::log(GLOG_INFO,"用户%d使用传送服务从(%f,%f,%f)到(%f,%f,%f)",pParent->ID.id,pParent->pos.x,pParent->pos.y,pParent->pos.z,entry.target.x,entry.target.y,entry.target.z);

		if(!pImp->_plane->PosInWorld(entry.target))	
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}

		//减少钱...
		if(fee)
		{
			pImp->SpendMoney(fee);
			pImp->_runner->spend_money(fee);
		}

		//让player进行传送
		pImp->LongJump(entry.target, entry.world_tag);
		return true;
	}
};
typedef feedback_provider country_service_provider;
class country_service_executor : public service_executor
{
public:
	enum
	{
		RT_JOIN	= 1,
		RT_LEAVE,
		RT_MAX,
	};
	struct player_request
	{
		int type;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		
		switch(req->type)
		{
			case RT_JOIN:
			case RT_LEAVE:
				break;
			default:
				return false;
		}
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;
		
		int ret = 0;
		switch(req->type)
		{
			case RT_JOIN:
				ret = pImp->CountryJoinApply();
				break;
			case RT_LEAVE:
				ret = pImp->CountryLeave();
				break;
			default:
				return false;
		}
		
		if(ret > 0)
			pImp->_runner->error_message(ret);	
		
		return true;
	}
};
typedef feedback_provider countrybattle_leave_provider;
class countrybattle_leave_executor : public service_executor
{
public:
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != 0) return false;
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		pImp->_filters.ModifyFilter(FILTER_CHECK_INSTANCE_KEY,FMID_CLEAR_AECB,NULL,0);
		return true;
	}
};
void __SendTo(gplayer_imp * pImp, int message, const XID & provider, int type)
{
	pImp->SendTo<0>(message, provider, type);
}
typedef feedback_provider change_ds_provider;
template<int CHANGE_DS_FLAG>
class change_ds_executor : public service_executor
{
public:
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != 0) return false;
		__SendTo(pImp,GM_MSG_SERVICE_REQUEST,provider,_type);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		int ret = pImp->PlayerTryChangeDS(CHANGE_DS_FLAG);
		if(ret > 0)
			pImp->_runner->error_message(ret);	
		return true;
	}
};
typedef change_ds_executor<GMSV::CHDS_FLAG_DS_TO_CENTRALDS> change_ds_forward_executor;
typedef change_ds_executor<GMSV::CHDS_FLAG_CENTRALDS_TO_DS> change_ds_backward_executor;


typedef feedback_provider addon_change_service_provider;
class addon_change_service_executor : public service_executor
{
	enum 
	{ 
		ADDON_CHANGE_MATERIAL_COUNT = 8
	};
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char equip_idx;  // 装备所在背包位 
		unsigned char equip_socket_idx;      // 需要转化的魂石所在孔位 
		int old_stone_type;        // 旧魂石类型 (对应孔位中魂石类型冗余校检) 
		int new_stone_type;        // 目标魂石类型 
		int recipe_type;	   //  配方id   (表中项与旧魂石和 目标魂石 一致)
		int materials_ids[ADDON_CHANGE_MATERIAL_COUNT];       // 材料类型 (排列顺序同 配方)
		unsigned char idxs[ADDON_CHANGE_MATERIAL_COUNT];     // 材料所在背包位
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
	
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}	
		
		const addonchange_recipe_template * acrt = recipe_manager::GetAddonChangeRecipe(req->recipe_type);
		
		if(!acrt) 
		{
			pImp->_runner->error_message(S2C::ERR_NO_EQUAL_RECIPE_FAIL);
			return true;
		}

		if(acrt->id_old_stone != req->old_stone_type || acrt->id_new_stone != req->new_stone_type)
		{
			pImp->_runner->error_message(S2C::ERR_NO_EQUAL_RECIPE_FAIL);
			return true;
		}

        if (world_manager::GetGlobalController().CheckServerForbid(SERVER_FORBID_RECIPE, acrt->recipe_id)) return true;

		for(int idx = 0; idx < ADDON_CHANGE_MATERIAL_COUNT; ++idx)
		{
			if(req->materials_ids[idx] != acrt->material_list[idx].item_id)
			{
				pImp->_runner->error_message(S2C::ERR_NO_EQUAL_RECIPE_FAIL);
				GLog::log(GLOG_ERR, "用户[%d]魂石转化和配方材料不符合%d",pImp->GetParent()->ID.id,idx);
				return true;
			}
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;

		return pImp->ChangeEquipAddon(req->equip_idx,
				req->equip_socket_idx,
				req->old_stone_type,
				req->new_stone_type,
				req->recipe_type,
				req->materials_ids,
				req->idxs,
				ADDON_CHANGE_MATERIAL_COUNT);
	}
};

typedef feedback_provider addon_replace_service_provider;
class addon_replace_service_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned char equip_idx;  // 装备 所在背包位
		unsigned char equip_socket_idx;      // 装备旧魂石所在孔位
		int old_stone_type;   // 旧魂石类型(对应孔位中魂石类型冗余校检) 
		unsigned char new_stone_idx;  // 目标 魂石所在背包位
		int new_stone_type;           // 目标魂石类型(冗余检查)
	};
#pragma pack()
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		DATA_TYPE dt_old ,dt_new;
		STONE_ESSENCE * ess_old = (STONE_ESSENCE *)world_manager::GetDataMan().get_data_ptr(req->old_stone_type, ID_SPACE_ESSENCE, dt_old);
		STONE_ESSENCE * ess_new = (STONE_ESSENCE *)world_manager::GetDataMan().get_data_ptr(req->new_stone_type, ID_SPACE_ESSENCE, dt_new);

		if(ess_old == NULL || dt_old != DT_STONE_ESSENCE)	return false;
		if(ess_new == NULL || dt_new != DT_STONE_ESSENCE)       return false;
		if(ess_new->level < ess_old->level)       
		{
			pImp->_runner->error_message(S2C::ERR_NO_EQUAL_DEST_FAIL);
			GLog::log(GLOG_ERR, "用户[%d]魂石替换等级不符",pImp->GetParent()->ID.id);

			return true;
		}

		if((unsigned int)(ess_old->uninstall_price + ess_new->install_price) > pImp->GetMoney())
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			GLog::log(GLOG_ERR, "用户[%d]魂石替换金钱不足",pImp->GetParent()->ID.id);
			
			return true;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;

		return pImp->ReplaceEquipAddon(req->equip_idx,
				req->equip_socket_idx,
				req->old_stone_type,
				req->new_stone_type,
				req->new_stone_idx);
	}
};

typedef feedback_provider kingelection_service_provider;
class kingelection_service_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;
		
		//检查是否可以开始
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));

		//检查是否开始
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		player_request * req = (player_request*)buf;
		//发送请求数据
		if(!GNET::ForwardKingElectionSysOP(req->service_id,req->buf, size - sizeof(player_request),object_interface(pImp)))
		{
			//发送错误
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		GLog::log(GLOG_INFO,"用户%d执行了国王选举操作",pImp->_parent->ID.id);
		return true;
	}
};

typedef feedback_provider player_shop_service_provider;
class player_shop_service_executor : public service_executor
{
public:
	struct player_request
	{
		int service_id;
		char buf[];
	};
	
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size < sizeof(player_request)) return false;
		if(size > 4096) return false;
		if(pImp->InCentralServer()) return false;
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			return false;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}

	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size >= sizeof(player_request));
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		if(!pImp->CanTrade(provider))
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}
		player_request * req = (player_request*)buf;
		if(!GNET::ForwardPShopSysOP(req->service_id, req->buf, size-sizeof(player_request), object_interface(pImp)))
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return false;
		}

		GLog::log(GLOG_INFO,"用户%d执行了玩家寄卖店铺相关操作(协议号%d).",pImp->_parent->ID.id, req->service_id);
		return true;
	}
};
typedef feedback_provider decompose_fashion_item_provider;
class decompose_fashion_item_executor : public service_executor
{
public:
	struct player_request
	{
		unsigned int index;
		int type;
	};
	enum
	{ 
		DECOMPOSE_FEE = 100000,			//分解费用 
		PRODUCTION_ID = 26684,			//分解产物
	};
	decompose_fashion_item_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request*)buf;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		if(!pImp->CheckItemExist(req->index, req->type, 1))
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return true;
		}

		if(pImp->GetMoney() < DECOMPOSE_FEE) 
		{
			pImp->_runner->error_message(S2C::ERR_OUT_OF_FUND);
			return true;
		}

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}

	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != sizeof(player_request))
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}
		player_request * req = (player_request*)buf;

		int rst = pImp->PlayerDecomposeFashionItem(req->index, req->type, DECOMPOSE_FEE, PRODUCTION_ID);
		if(rst) pImp->_runner->error_message(rst);
		return true;
	}
};
typedef feedback_provider reincarnation_provider;
class reincarnation_executor : public service_executor
{
public:
	reincarnation_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != 0) return false;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}

	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != 0)
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}

		pImp->PlayerReincarnation();
		return true;
	}
};
typedef feedback_provider giftcardredeem_provider;
class giftcardredeem_executor : public service_executor
{
public:
	giftcardredeem_executor() {}
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != player_giftcard::GIFT_CARDNUMBER_LEN) return false;

		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}

		//发出功能请求
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}

	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(size != player_giftcard::GIFT_CARDNUMBER_LEN)
		{
			//发出的时候就应该正确的
			ASSERT(false);
			return false;
		}

		char cardnumber[player_giftcard::GIFT_CARDNUMBER_LEN];
		memcpy(cardnumber,buf,player_giftcard::GIFT_CARDNUMBER_LEN);

		pImp->PlayerRedeemGiftCard(cardnumber);
		return true;
	}
};
typedef feedback_provider trickbattle_apply_service_provider;
class trickbattle_apply_service_executor : public service_executor
{
public:
	struct player_request
	{
		int chariot;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;
		
		int ret = pImp->PlayerTrickBattleApply(req->chariot);
		if(ret > 0)
			pImp->_runner->error_message(ret);	
		
		return true;
	}
};
typedef feedback_provider generalcard_rebirth_service_provider;
class generalcard_rebirth_service_executor : public service_executor
{
public:
	struct player_request
	{
		unsigned char major_inv_idx;
		unsigned char minor_inv_idx;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;
		
		int ret = pImp->PlayerGeneralCardRebirth(req->major_inv_idx, req->minor_inv_idx);
		if(ret > 0)
			pImp->_runner->error_message(ret);	
		
		return true;
	}
};
typedef feedback_provider improve_flysword_service_provider;
class improve_flysword_service_executor : public service_executor
{
public:
	struct player_request
	{
		unsigned int inv_idx;
		int flysword_id;
	};
private:
	
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false;
		player_request * req = (player_request *)buf;
		
		if(!pImp->IsItemExist(req->inv_idx, req->flysword_id, 1))
		{
			pImp->_runner->error_message(S2C::ERR_ITEM_NOT_IN_INVENTORY);
			return true;
		}
		
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}


	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		ASSERT(size == sizeof(player_request));
		player_request * req = (player_request*)buf;
		
		int ret = pImp->PlayerImproveFlysword(req->inv_idx, req->flysword_id);
		if(ret > 0)
			pImp->_runner->error_message(ret);	
		
		return true;
	}
};

typedef feedback_provider mafia_pvp_signup_provider;

class mafia_pvp_signup_executor : public service_executor
{
public:
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(!((gplayer*)pImp->_parent)->id_mafia || ((gplayer*)pImp->_parent)->rank_mafia > 4)
		{
			pImp->_runner->error_message(S2C::ERR_MAFIA_NO_PERMISSION);
			return true;
		}
		if(((gplayer*)pImp->_parent)->mafia_pvp_mask)
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_ERR_REQUEST);
			return true;
		}
		if(!world_manager::GetWorldFlag().mafia_pvp_flag)
		{
			pImp->_runner->error_message(S2C::ERR_SERVICE_UNAVILABLE);
			return true;
		}

		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;
	}
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(!((gplayer*)pImp->_parent)->id_mafia || ((gplayer*)pImp->_parent)->rank_mafia > 4 
				|| ((gplayer*)pImp->_parent)->mafia_pvp_mask || !world_manager::GetWorldFlag().mafia_pvp_flag)
			return false;
		
		GMSV::SendMafiaPvPEvent(0,((gplayer*)pImp->_parent)->id_mafia,0,pImp->_parent->ID.id,0,0);	
		
		return true;
	}

};

class npc_goldshop_provider : public general_provider
{
private:	
	unsigned int _shop_tid;
	virtual bool Save(archive & ar)
	{
		ar << _shop_tid;
		return true;
	}
	virtual bool Load(archive & ar)
	{
		ar >> _shop_tid;
		return true;
	}
public:
#pragma pack(1)
	struct player_request
	{
		unsigned int count;
		struct __entry
		{
			int goods_id;
			int goods_index;
			int goods_slot;
		}list[1];
	};
	struct server_request
	{
		int shop_tid;
		player_request data;		
	};
#pragma pack()
	npc_goldshop_provider():_shop_tid(0) {}
private:
	virtual npc_goldshop_provider * Clone()
	{
		ASSERT(!_is_init);
		return new npc_goldshop_provider(*this);	
	}
	virtual bool OnInit(const void * buf, size_t size)
	{
		if(size  != sizeof(int))
		{
			ASSERT(false);
			return false;
		}
		_shop_tid = *((int*)buf);
		return true;
	}
	virtual void TryServe(const XID & player, const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) 
		{
			SendMessage(GM_MSG_ERROR_MESSAGE,player,S2C::ERR_SERVICE_ERR_REQUEST,NULL,0);
			return;
		}

		server_request param;
		param.shop_tid = _shop_tid;
		memcpy(&param.data,buf,size);
		SendMessage(GM_MSG_SERVICE_DATA, player,_type, &param, sizeof(server_request));
	}
	virtual void GetContent(const XID & player,int cs_index,int sid){}
	virtual void OnHeartbeat(){}
	virtual void OnControl(int param ,const void * buf, size_t size){}
};

class npc_goldshop_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned int count;
		struct __entry
		{
			int goods_id;
			int goods_index;
			int goods_slot;
		}list[1];
	};
	struct server_request
	{
		int shop_tid;
		player_request data;		
	};
#pragma pack()
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false; 
		player_request* param = (player_request*)buf;
		if(param->count != 1) return false;
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;	
	}
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return false;
		}

		ASSERT(size == sizeof(server_request));
		server_request* param = (server_request*)buf;
		
		return pImp->PlayerDoShopping(1,(const int*)param->data.list,param->shop_tid);	
	}
};

typedef npc_goldshop_provider npc_dividendshop_provider;
class npc_dividendshop_executor : public service_executor
{
public:
#pragma pack(1)
	struct player_request
	{
		unsigned int count;
		struct __entry
		{
			int goods_id;
			int goods_index;
			int goods_slot;
		}list[1];
	};
	struct server_request
	{
		int shop_tid;
		player_request data;		
	};
#pragma pack()
private:
	virtual bool SendRequest(gplayer_imp * pImp, const XID & provider,const void * buf, size_t size)
	{
		if(size != sizeof(player_request)) return false; 
		player_request* param = (player_request*)buf;
		if(param->count != 1) return false;
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return true;
		}
		//发送转发数据
		pImp->SendTo<0>(GM_MSG_SERVICE_REQUEST,provider,_type,buf,size);
		return true;	
	}
	virtual bool OnServe(gplayer_imp *pImp,const XID & provider, const A3DVECTOR & pos,const void * buf, size_t size)
	{
		if(pImp->OI_TestSafeLock())
		{
			pImp->_runner->error_message(S2C::ERR_FORBIDDED_OPERATION_IN_SAFE_LOCK);
			return false;
		}

		ASSERT(size == sizeof(server_request));
		server_request* param = (server_request*)buf;
		
		return pImp->PlayerDoDividendShopping(1,(const int*)param->data.list,param->shop_tid);	
	}
};

}
using namespace NG_ELEMNET_SERVICE;

static service_inserter si1 = SERVICE_INSERTER(vendor_provider,vendor_executor,1);
static service_inserter si2 = SERVICE_INSERTER(purchase_provider,purchase_executor,2);
static service_inserter si3 = SERVICE_INSERTER(repair_provider,repair_executor,3);
static service_inserter si4 = SERVICE_INSERTER(heal_provider,heal_executor,4);
static service_inserter si5 = SERVICE_INSERTER(transmit_provider,transmit_executor,5);
static service_inserter si6 = SERVICE_INSERTER(task_in_provider,task_in_executor,6);
static service_inserter si7 = SERVICE_INSERTER(task_out_provider,task_out_executor,7);
static service_inserter si8 = SERVICE_INSERTER(task_matter_provider,task_matter_executor,8);
static service_inserter si9 = SERVICE_INSERTER(skill_provider,skill_executor,9);
static service_inserter si10 = SERVICE_INSERTER(install_provider,install_executor,10);
static service_inserter si11 = SERVICE_INSERTER(uninstall_provider,uninstall_executor,11);
static service_inserter si12 = SERVICE_INSERTER(produce_provider,produce_executor,12);
static service_inserter si13 = SERVICE_INSERTER(decompose_provider,decompose_executor,13);
static service_inserter si14 = SERVICE_INSERTER(trashbox_passwd_provider,trashbox_passwd_executor,14);
static service_inserter si15 = SERVICE_INSERTER(trashbox_open_provider,trashbox_open_executor,15);
static service_inserter si16 = SERVICE_INSERTER(plane_switch_provider, plane_switch_executor,16);
static service_inserter si17 = SERVICE_INSERTER(identify_provider, identify_executor,17);
static service_inserter si18 = SERVICE_INSERTER(faction_service_provider, faction_service_executor,18);
static service_inserter si19 = SERVICE_INSERTER(player_market_provider, player_market_executor,19);
static service_inserter si20 = SERVICE_INSERTER(vehicle_service_provider, vehicle_service_executor,20);
static service_inserter si21 = SERVICE_INSERTER(player_market_provider, player_market_executor2,21);
static service_inserter si22 = SERVICE_INSERTER(waypoint_service_provider, waypoint_service_executor,22);
static service_inserter si23 = SERVICE_INSERTER(unlearn_skill_provider,unlearn_skill_executor ,23);
static service_inserter si24 = SERVICE_INSERTER(cosmetic_provider,cosmetic_executor,24);
static service_inserter si25 = SERVICE_INSERTER(mail_service_provider,mail_service_executor,25);
static service_inserter si26 = SERVICE_INSERTER(auction_service_provider,auction_service_executor,26);
static service_inserter si27 = SERVICE_INSERTER(double_exp_provider,double_exp_executor,27);
static service_inserter si28 = SERVICE_INSERTER(hatch_pet_service_provider, hatch_pet_service_executor, 28);
static service_inserter si29 = SERVICE_INSERTER(restore_pet_service_provider,restore_pet_service_executor ,29);
static service_inserter si30 = SERVICE_INSERTER(battle_service_provider,battle_service_executor,30);
static service_inserter si31 = SERVICE_INSERTER(towerbuild_provider,towerbuild_executor,31);
static service_inserter si32 = SERVICE_INSERTER(battle_leave_provider,battle_leave_executor,32);
static service_inserter si33 = SERVICE_INSERTER(resetprop_provider,resetprop_executor,33);
static service_inserter si34 = SERVICE_INSERTER(spec_trade_provider,spec_trade_executor,34);
static service_inserter si35 = SERVICE_INSERTER(refine_service_provider, refine_service_executor ,35);
static service_inserter si36 = SERVICE_INSERTER(change_pet_name_provider, change_pet_name_executor ,36);
static service_inserter si37 = SERVICE_INSERTER(forget_pet_skill_provider, forget_pet_skill_executor ,37);
static service_inserter si38 = SERVICE_INSERTER(pet_skill_provider, pet_skill_executor ,38);
static service_inserter si39 = SERVICE_INSERTER(bind_item_provider, bind_item_executor ,39);
static service_inserter si40 = SERVICE_INSERTER(destory_bind_item_provider, destory_bind_item_executor,40);
static service_inserter si41 = SERVICE_INSERTER(destory_item_restore_provider,destory_item_restore_executor,41);
static service_inserter si42 = SERVICE_INSERTER( stock_service_provider,stock_service_executor1,42);
static service_inserter si43 = SERVICE_INSERTER( stock_service_provider,stock_service_executor2,43);
static service_inserter si44 = SERVICE_INSERTER(dye_service_provider ,dye_service_executor,44);
static service_inserter si45 = SERVICE_INSERTER(refine_transmit_service_provider ,refine_transmit_service_executor,45);
static service_inserter si46 = SERVICE_INSERTER(produce2_provider,produce2_executor,46);
static service_inserter si47 = SERVICE_INSERTER(feedback_provider,make_slot_executor,47);
static service_inserter si48 = SERVICE_INSERTER(elf_dec_attribute_provider,elf_dec_attribute_executor,48);//lgc
static service_inserter si49 = SERVICE_INSERTER(elf_flush_genius_provider,elf_flush_genius_executor,49);
static service_inserter si50 = SERVICE_INSERTER(elf_learn_skill_provider, elf_learn_skill_executor, 50);
static service_inserter si51 = SERVICE_INSERTER(elf_forget_skill_provider, elf_forget_skill_executor, 51);
static service_inserter si52 = SERVICE_INSERTER(elf_refine_provider, elf_refine_executor, 52);
static service_inserter si53 = SERVICE_INSERTER(elf_refine_transmit_provider ,elf_refine_transmit_executor,53);
static service_inserter si54 = SERVICE_INSERTER(elf_decompose_provider, elf_decompose_executor, 54);
static service_inserter si55 = SERVICE_INSERTER(elf_destroy_item_provider, elf_destroy_item_executor, 55);
static service_inserter si56 = SERVICE_INSERTER(dye_suit_service_provider ,dye_suit_service_executor,56);
static service_inserter si57 = SERVICE_INSERTER(repair_damaged_item_provider,repair_damaged_item_executor,57);
static service_inserter si58 = SERVICE_INSERTER(produce3_provider,produce3_executor,58);
static service_inserter si59 = SERVICE_INSERTER(user_trashbox_open_provider,user_trashbox_open_executor,59);
static service_inserter si60 = SERVICE_INSERTER(webtrade_service_provider,webtrade_service_executor,60);
static service_inserter si61 = SERVICE_INSERTER(god_evil_convert_service_provider,god_evil_convert_service_executor,61);
static service_inserter si62 = SERVICE_INSERTER(wedding_book_provider,wedding_book_executor,62);
static service_inserter si63 = SERVICE_INSERTER(wedding_invite_provider,wedding_invite_executor,63);
static service_inserter si64 = SERVICE_INSERTER(factionfortress_service_provider,factionfortress_service_executor,64);
static service_inserter si65 = SERVICE_INSERTER(factionfortress_service_provider2,factionfortress_service_executor2,65);
static service_inserter si66 = SERVICE_INSERTER(factionfortress_material_exchange_provider,factionfortress_material_exchange_executor,66);
static service_inserter si67 = SERVICE_INSERTER(dye_pet_service_provider,dye_pet_service_executor,67);
static service_inserter si68 = SERVICE_INSERTER(trashbox_open_view_only_provider,trashbox_open_view_only_executor,68);
static service_inserter si69 = SERVICE_INSERTER(engrave_provider,engrave_executor,69);
static service_inserter si70 = SERVICE_INSERTER(dpsrank_service_provider,dpsrank_service_executor,70);
static service_inserter si71 = SERVICE_INSERTER(addonregen_provider,addonregen_executor,71);
static service_inserter si72 = SERVICE_INSERTER(player_force_service_provider,player_force_service_executor,72);
static service_inserter si73 = SERVICE_INSERTER(unlimited_transmit_provider,unlimited_transmit_executor,73);
static service_inserter si74 = SERVICE_INSERTER(produce4_provider,produce4_executor,74);
static service_inserter si75 = SERVICE_INSERTER(country_service_provider,country_service_executor,75);
static service_inserter si76 = SERVICE_INSERTER(countrybattle_leave_provider,countrybattle_leave_executor,76);
static service_inserter si77 = SERVICE_INSERTER(equip_signature_provider,equip_signature_executor,77);
static service_inserter si78 = SERVICE_INSERTER(change_ds_provider,change_ds_forward_executor,78);
static service_inserter si79 = SERVICE_INSERTER(change_ds_provider,change_ds_backward_executor,79);
static service_inserter si80 = SERVICE_INSERTER(player_rename_provider,player_rename_executor,80);
static service_inserter si81 = SERVICE_INSERTER(addon_change_service_provider,addon_change_service_executor,81);
static service_inserter si82 = SERVICE_INSERTER(addon_replace_service_provider,addon_replace_service_executor,82);
static service_inserter si83 = SERVICE_INSERTER(kingelection_service_provider,kingelection_service_executor,83);
static service_inserter si84 = SERVICE_INSERTER(decompose_fashion_item_provider,decompose_fashion_item_executor,84);
static service_inserter si85 = SERVICE_INSERTER(player_shop_service_provider,player_shop_service_executor,85);
static service_inserter si86 = SERVICE_INSERTER(reincarnation_provider,reincarnation_executor,86);
static service_inserter si87 = SERVICE_INSERTER(giftcardredeem_provider,giftcardredeem_executor,87);
static service_inserter si88 = SERVICE_INSERTER(trickbattle_apply_service_provider,trickbattle_apply_service_executor,88);
static service_inserter si89 = SERVICE_INSERTER(generalcard_rebirth_service_provider,generalcard_rebirth_service_executor,89);
static service_inserter si90 = SERVICE_INSERTER(improve_flysword_service_provider,improve_flysword_service_executor,90);
static service_inserter si91 = SERVICE_INSERTER(mafia_pvp_signup_provider,mafia_pvp_signup_executor,91);
static service_inserter si92 = SERVICE_INSERTER(produce5_provider,produce5_executor,92);
static service_inserter si93 = SERVICE_INSERTER(npc_goldshop_provider,npc_goldshop_executor,93);
static service_inserter si94 = SERVICE_INSERTER(npc_dividendshop_provider,npc_dividendshop_executor,94);

