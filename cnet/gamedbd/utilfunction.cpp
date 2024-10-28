#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "dbbuffer.h"
#include "gsyslog"
#include "user"
#include "grolestorehouse"
#include "localmacro.h"
#include "utilfunction.h"
#include "gamedbmanager.h"
#include "greincarnationdata"
#include "grolestatusextraprop"
#include "mnfactioninfo"
namespace GNET
{
int WriteRestSyncData(StorageEnv::Storage *pstore,StorageEnv::Storage *plog,int roleid,
	GMailSyncData& data, int money_delta, StorageEnv::Transaction& txn)
{
	Marshal::OctetsStream key;
	key << roleid;
	if(data.data_mask & SYNC_STOTEHOUSE)
	{
		Marshal::OctetsStream value;
		if(pstore->find(key, value, txn))
		{
			GRoleStorehouse store;
			value >> store;
			money_delta += data.storehouse.money - store.money;
		}
		pstore->insert(key, Marshal::OctetsStream()<<data.storehouse,txn);
	}
	/*
	if(data.logs.size() )
	{
		Marshal::OctetsStream keylog;
		keylog << GameDBManager::GetInstance()->GetShoplogid();
		plog->insert(keylog, Marshal::OctetsStream()<<data.logs, txn);

		data.data_mask |= SYNC_SHOPLOG;
	}
	*/
	if(money_delta)
		GameDBManager::GetInstance()->UpdateMoney(roleid,money_delta);
	return 0;
}

void PutSyslog(StorageEnv::Storage *plog,StorageEnv::Transaction& txn,int roleid,int ip, GRoleInventory& inv )
{
        if(plog)
        {               
		GSysLog log(roleid,Timer::GetTime(),ip,0,0);
		log.items.push_back(inv);
                Marshal::OctetsStream keylog;
                keylog << GameDBManager::GetInstance()->GetGUID();
                plog->insert(keylog, Marshal::OctetsStream()<<log, txn);
        }
}
void PutSyslog(StorageEnv::Storage *plog,StorageEnv::Transaction& txn,int roleid,int ip, int money, GRoleInventoryVector& invs)
{
        if(plog)
        {               
		GSysLog log(roleid,Timer::GetTime(),ip,1, money);
		log.items.swap(invs);
                Marshal::OctetsStream keylog;
                keylog << GameDBManager::GetInstance()->GetGUID();
                plog->insert(keylog, Marshal::OctetsStream()<<log, txn);
        }
}

int GetRoleReincarnationTimes(const Octets & odata)
{
	if(!odata.size()) return 0;
	GReincarnationData data;
	try{
		Marshal::OctetsStream(odata) >> data;
		return data.records.size();
	}
	catch(Marshal::Exception)
	{
		return 0;
	}
}

void GetRoleReincarnationDetail(const Octets & odata,int& times,int& maxlevel,GReincarnationData& data)
{
	if(!odata.size()) return;
	try{
		Marshal::OctetsStream(odata) >> data;
		times = data.records.size();
		for(int i = 0; i < times; ++i)
		{
			GReincarnationRecord& detail = data.records[i];
			if(detail.level > maxlevel) maxlevel = detail.level;
		}
	}
	catch(Marshal::Exception)
	{
        data = GReincarnationData();
		return ;
	}
}

void GetRoleRealmDetail(const Octets & odata,int& level)
{
	if(!odata.size()) return;
	try{
		Marshal::OctetsStream(odata) >> level;
	}
	catch(Marshal::Exception)
	{
		return ;
	}
}

void GetRoleVisaDetail(const Octets & odata, short& type,int64_t& ufid)
{
	type = -1; ufid = 0;
	if(!odata.size()) return;

	try{
		GRoleStatusExtraProp extraprop;
		Marshal::OctetsStream(odata) >> extraprop;

		std::map<int,Octets>::iterator it=extraprop.data.find(GROLE_STATUS_EXTRAPROP_VISA_INFO);
		if(it != extraprop.data.end())
		{
			int temp_type = -1;
			Marshal::OctetsStream(it->second) >> temp_type;
			type = (short)temp_type;
		}

		it=extraprop.data.find(GROLE_STATUS_EXTRAPROP_MNFACTION_INFO);
		if(it != extraprop.data.end())
		{
			Marshal::OctetsStream(it->second) >> ufid;
		}
	}
	catch(Marshal::Exception)  
	{
		return ;
	}
}

void IncMNFactionVersion(int64_t ufid,StorageEnv::Storage* pmnfaction,StorageEnv::Transaction& txn)
{
	Marshal::OctetsStream key, value; 
	key << ufid;
	if(pmnfaction->find(key, value, txn))
	{
		MNFactionInfo mnfinfo;
		value >> mnfinfo;
		++mnfinfo.version;
		pmnfaction->insert(key,Marshal::OctetsStream()<<mnfinfo,txn);
	}
}

static PShopFunc::PSHOP_OP limits_normal[] = {};
static PShopFunc::PSHOP_OP limits_expired[] = { PShopFunc::PSHOP_OP_TRADE, PShopFunc::PSHOP_OP_SAVE_MONEY, PShopFunc::PSHOP_OP_SET_TYPE};
std::set<PShopFunc::PSHOP_OP> PShopFunc::_limits_normal(limits_normal, limits_normal+ sizeof(limits_normal)/sizeof(PShopFunc::PSHOP_OP));
std::set<PShopFunc::PSHOP_OP> PShopFunc::_limits_expired(limits_expired, limits_expired+ sizeof(limits_expired)/sizeof(PShopFunc::PSHOP_OP));

int CashVip::cash_vip_score[CASH_VIP_MAX_LEVEL] = {0};
int CashVip::cash_vip_reduce_score[CASH_VIP_MAX_LEVEL+1] = {0};

};
