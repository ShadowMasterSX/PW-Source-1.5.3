#ifndef __ONLINEGAME_GS_SERVICE_NPC_H__
#define __ONLINEGAME_GS_SERVICE_NPC_H__

#include "npc.h"
#include "serviceprovider.h"

class service_npc : public gnpc_imp
{
	typedef provider_list<MAX_PROVIDER_PER_NPC>  LIST;
	LIST _service_list;
	float _tax_rate;
	bool _need_domain;
	bool _serve_distance_unlimited; 
	int  _domain_test_time;
	int  _domain_mafia;
	int	 _life_time;		//npc寿命，0表示永久
	int  _src_monster;
public:
DECLARE_SUBSTANCE(service_npc);

public:
	service_npc():_tax_rate(0),_need_domain(false),_serve_distance_unlimited(false),_domain_test_time(0),_domain_mafia(0),_life_time(0),_src_monster(0){}
	~service_npc();
	virtual void Reborn()
	{
		gnpc_imp::Reborn();
		//这里好像还没有什么可做的，列表应该依然可用
		//也有可能列表需要重新刷新，在相应服务加入后再处理
	}

	void SetTaxRate(float taxrate)
	{
		_tax_rate = taxrate;
	}
	
	float GetTaxRate() 
	{ 
		return _tax_rate;
	}
	
	int GetCurIDMafia();

	void SetNeedDomain(bool need_domain)
	{
		_need_domain = need_domain;
	}

	void SetServeDistanceUnlimited(bool b)
	{
		_serve_distance_unlimited = b;
	}
	
	void SetSrcMonster(int src_monster)
	{
		_src_monster = src_monster;
	}
	
	void AddProvider(service_provider * provider,  const void * buf, size_t size)
	{
		if(provider->Init(this,buf,size))
		{
			_service_list.AddProvider(provider);
		}
		else
		{
			delete provider;
		}
	}

	bool CheckDistanceLimit(const A3DVECTOR & pos);

	virtual int MessageHandler(world * pPlane ,const MSG & msg);
	virtual int GetSrcMonster() { return _src_monster; };

	bool Save(archive & ar);
	bool Load(archive & ar);
	virtual void OnNpcEnterWorld();
	virtual void OnNpcLeaveWorld();
	virtual int DoAttack(const XID & target, char force_attack);
	virtual void FillAttackMsg(const XID & target, attack_msg & attack);
	virtual void FillEnchantMsg(const XID & target, enchant_msg & enchant);
	virtual void OnHeartbeat(size_t tick);
	virtual void SetLifeTime(int life);
};

#endif

