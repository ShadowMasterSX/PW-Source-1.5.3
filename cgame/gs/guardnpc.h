#ifndef __ONLINEGAME_GS_GUARDNPC_H__
#define __ONLINEGAME_GS_GUARDNPC_H__

#include "aipolicy.h"

class guard_agent : public substance
{
public:
	DECLARE_SUBSTANCE(guard_agent);
	virtual bool GatherTarget(ai_object * self ,ai_policy * policy, int exclude_faction = 0);
	bool Save(archive &ar) { return true;}
	bool Load(archive &ar) { return true;}

	template<typename SLICE>
	struct search_target
	{
		ai_object* _self;
		float _squared_radius;
		int _enemy_faction;
		int _exclude_faction;
		search_target(ai_object* self,float radius,int enemy_faction,int exclude_faction):_self(self),_squared_radius(radius*radius),_enemy_faction(enemy_faction),_exclude_faction(exclude_faction){}

		inline void operator()(SLICE *pPiece,const A3DVECTOR & pos)
		{
			if(!pPiece->npc_list && !pPiece->player_list)  return;
			pPiece->Lock();
			gobject * pObj = pPiece->npc_list; 
			while(pObj)
			{
				if(pos.squared_distance(pObj->pos) < _squared_radius)
				{
					gnpc * pNPC = (gnpc*)pObj;
					if(pNPC->IsActived()&&!pNPC->IsZombie() && (pNPC->base_info.faction&_enemy_faction)
					&& !(pNPC->base_info.faction&_exclude_faction))
					{
						//考虑找到一个就结束
						_self->ChangeAggroEntry(pNPC->ID,1);
					}
				}
				pObj = pObj->pNext;
			}

			/*	不再寻找玩家，考虑改为用怪物策略来完成此操作
				gplayer * pPlayer = (gplayer*)(pPiece->player_list); 
				while(pPlayer)
				{
				if(!pPlayer->invisible && pos.squared_distance(pPlayer->pos) < _squared_radius)
				{
				if(pPlayer->IsActived()&&!pPlayer->IsZombie() 
				&& (pPlayer->base_info.faction & _enemy_faction))
				{
			//考虑找到一个就结束
			_self->ChangeAggroEntry(pPlayer->ID,1);
			}
			}
			pPlayer = (gplayer*) pPlayer->pNext;
			}
			 */
			pPiece->Unlock();
		}
	};
};

class guard_policy :  public ai_policy
{
	virtual float GetReturnHomeRange() { return 0.f*0.f;}
protected:
	guard_agent * _agent;
public:
	DECLARE_SUBSTANCE(guard_policy);
	guard_policy():ai_policy(),_agent(new guard_agent)
	{}
	virtual ~guard_policy() { if(_agent) delete _agent; }
	virtual void OnHeartbeat();
	virtual bool Save(archive & ar)
	{
		ai_policy::Save(ar);
		_agent->SaveInstance(ar);
		return true;
	}

	virtual bool Load(archive & ar)
	{
		ai_policy::Load(ar);
		if(_agent) delete _agent;
		_agent = substance::DynamicCast<guard_agent>(substance::LoadInstance(ar));
		ASSERT(_agent);
		return true;
	}
};

#endif

