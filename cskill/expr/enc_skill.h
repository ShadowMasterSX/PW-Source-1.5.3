#ifndef __GNET_SKILL_H
#define __GNET_SKILL_H
#include <string>
#include "bridge.h"
	
#include "skill.h"	
#include "enc_playerwrapper.h"
#include "enc_targetwrapper.h"
#include "enc_playerwrapper.h"
namespace GNET
{
	class SkillCO : public CommonObject
	{
	private:
		Skill* pObj;
		mutable PlayerWrapperCO*	player;
		mutable TargetWrapperCO*	target;
		mutable PlayerWrapperCO*	xtarget;

		static Method_Map method_map_get,method_map_set;
		int GetId() const { return pObj->GetId(); }
		void  SetId(int value) { pObj->SetId(value); }
		int GetLevel() const { return pObj->GetLevel(); }
		void  SetLevel(int value) { pObj->SetLevel(value); }
		int GetPerformerlevel() const { return pObj->GetPerformerlevel(); }
		void  SetPerformerlevel(int value) { pObj->SetPerformerlevel(value); }
		int GetWarmuptime() const { return pObj->GetWarmuptime(); }
		void  SetWarmuptime(int value) { pObj->SetWarmuptime(value); }
		int GetRand() const { return pObj->GetRand(); }
		void  SetRand(int value) { pObj->SetRand(value); }
		CommonObject* GetPlayer() const 
		{
			if (player)
			{
				player->SetContent(pObj->GetPlayer());
				return player;
			}
			return player=new PlayerWrapperCO(pObj->GetPlayer());
		}
		void SetPlayer(CommonObject* co)
		{
			PlayerWrapperCO* wrapper=dynamic_cast<PlayerWrapperCO*>(co);
			if (wrapper != NULL) pObj->SetPlayer(wrapper->GetContent());
		}
		CommonObject* GetTarget() const 
		{
			if (target)
			{
				target->SetContent(pObj->GetTarget());
				return target;
			}
			return target=new TargetWrapperCO(pObj->GetTarget());
		}
		void SetTarget(CommonObject* co)
		{
			TargetWrapperCO* wrapper=dynamic_cast<TargetWrapperCO*>(co);
			if (wrapper != NULL) pObj->SetTarget(wrapper->GetContent());
		}
		CommonObject* GetXtarget() const 
		{
			if (xtarget)
			{
				xtarget->SetContent(pObj->GetXtarget());
				return xtarget;
			}
			return xtarget=new PlayerWrapperCO(pObj->GetXtarget());
		}
		void SetXtarget(CommonObject* co)
		{
			PlayerWrapperCO* wrapper=dynamic_cast<PlayerWrapperCO*>(co);
			if (wrapper != NULL) pObj->SetXtarget(wrapper->GetContent());
		}
	public:
		Skill* GetContent() { return pObj; }
		void SetContent(Skill* _pObj) { pObj=_pObj; }
		static SkillCO* Create()
		{
			Skill* p=new Skill();
			return new SkillCO(p);
		}	
		static void Destroy(SkillCO* p)
		{
			delete p->GetContent();
			delete p;
		}
		SkillCO(Skill* _pObj) : pObj(_pObj),player(NULL),target(NULL),xtarget(NULL)	
		{
			if (method_map_get.size() == 0)
			{
				method_map_get["id"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::GetId);
				method_map_get["level"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::GetLevel);
				method_map_get["performerlevel"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::GetPerformerlevel);
				method_map_get["warmuptime"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::GetWarmuptime);
				method_map_get["rand"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::GetRand);
				method_map_get["player"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::GetPlayer);
				method_map_get["target"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::GetTarget);
				method_map_get["xtarget"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::GetXtarget);
			}
			if (method_map_set.size() == 0)
			{
				method_map_set["id"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::SetId);
				method_map_set["level"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::SetLevel);
				method_map_set["performerlevel"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::SetPerformerlevel);
				method_map_set["warmuptime"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::SetWarmuptime);
				method_map_set["rand"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::SetRand);
				method_map_set["player"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::SetPlayer);
				method_map_set["target"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::SetTarget);
				method_map_set["xtarget"]=(memfunc_pointer_t)mem_fun_p(&SkillCO::SetXtarget);
			}
		}	
		virtual ~SkillCO() 
		{
			delete player;
			delete target;
			delete xtarget;
		}

		template <typename _t>
		bool GetProperty(const std::string& p_name,_t & value)
		{
			Method_Map::iterator it=method_map_get.find(p_name);
			if (it == method_map_get.end()) return false;
			unary_function_t<_t,SkillCO>* pFunc = dynamic_cast< unary_function_t<_t,SkillCO>* >((*it).second);
			if (pFunc == NULL) return false;
			value = (*pFunc)(this);
			return true;
		}
		template <typename _t>
		bool SetProperty(const std::string& p_name,const _t& value)   
		{
			Method_Map::iterator it=method_map_set.find(p_name);
			if (it == method_map_set.end()) return false;
			binary_function_t<void,_t,SkillCO>* pFunc = dynamic_cast< binary_function_t<void,_t,SkillCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,value);
			return true;
		}	
		
		template <typename _t>
		bool GetArrayValue(const int index,_t& value)
		{
			Method_Map::iterator it=method_map_get.find("value");
			if (it == method_map_get.end()) return false;
			const_binary_function_t<_t,int,SkillCO>* pFunc2 = dynamic_cast< const_binary_function_t<_t,int,SkillCO>* >((*it).second);
			if (pFunc2 == NULL) return false;
			value = (*pFunc2)(this,index);
			return true;
		}
		template <typename _t>	
		bool SetArrayValue(const int index,const _t& value)
		{
			Method_Map::iterator it=method_map_set.find("value");
			if (it == method_map_set.end()) return false;
			binary_function_t<void,const pair_t<_t>&,SkillCO>* pFunc = dynamic_cast< binary_function_t<void,const pair_t<_t>&,SkillCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,pair_t<_t>(index,value));
			return true;
		}	
	
	}; //end of class SkillCO
}; // end of namespace	

#endif
