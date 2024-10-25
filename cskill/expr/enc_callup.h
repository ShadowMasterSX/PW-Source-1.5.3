#ifndef __GNET_CALLUP_H
#define __GNET_CALLUP_H
#include <string>
#include "bridge.h"
	
#include "callup.h"	
namespace GNET
{
	class CallupCO : public CommonObject
	{
	private:
		Callup* pObj;

		static Method_Map method_map_get,method_map_set;
		int GetId() const { return pObj->GetId(); }
		void  SetId(int value) { pObj->SetId(value); }
		int GetCount() const { return pObj->GetCount(); }
		void  SetCount(int value) { pObj->SetCount(value); }
		int GetLivetime() const { return pObj->GetLivetime(); }
		void  SetLivetime(int value) { pObj->SetLivetime(value); }
		int GetHp() const { return pObj->GetHp(); }
		void  SetHp(int value) { pObj->SetHp(value); }
		int GetAttackmode() const { return pObj->GetAttackmode(); }
		void  SetAttackmode(int value) { pObj->SetAttackmode(value); }
		bool GetRestricttarget() const { return pObj->GetRestricttarget(); }
		void  SetRestricttarget(bool value) { pObj->SetRestricttarget(value); }
		float GetRectlength() const { return pObj->GetRectlength(); }
		void  SetRectlength(float value) { pObj->SetRectlength(value); }
		int GetAttack() const { return pObj->GetAttack(); }
		void  SetAttack(int value) { pObj->SetAttack(value); }
		int GetPoison() const { return pObj->GetPoison(); }
		void  SetPoison(int value) { pObj->SetPoison(value); }
		int GetAttackwater() const { return pObj->GetAttackwater(); }
		void  SetAttackwater(int value) { pObj->SetAttackwater(value); }
		float GetWoodfactor() const { return pObj->GetWoodfactor(); }
		void  SetWoodfactor(float value) { pObj->SetWoodfactor(value); }
		float GetWoodprob() const { return pObj->GetWoodprob(); }
		void  SetWoodprob(float value) { pObj->SetWoodprob(value); }
		float GetFixprob() const { return pObj->GetFixprob(); }
		void  SetFixprob(float value) { pObj->SetFixprob(value); }
		int GetDefence() const { return pObj->GetDefence(); }
		void  SetDefence(int value) { pObj->SetDefence(value); }
		float GetVelocity() const { return pObj->GetVelocity(); }
		void  SetVelocity(float value) { pObj->SetVelocity(value); }
		float GetInterval() const { return pObj->GetInterval(); }
		void  SetInterval(float value) { pObj->SetInterval(value); }
		float GetView() const { return pObj->GetView(); }
		void  SetView(float value) { pObj->SetView(value); }
		float GetSearchdistance() const { return pObj->GetSearchdistance(); }
		void  SetSearchdistance(float value) { pObj->SetSearchdistance(value); }
		float GetAttackdistance() const { return pObj->GetAttackdistance(); }
		void  SetAttackdistance(float value) { pObj->SetAttackdistance(value); }
	public:
		Callup* GetContent() { return pObj; }
		void SetContent(Callup* _pObj) { pObj=_pObj; }
		static CallupCO* Create()
		{
			Callup* p=new Callup();
			return new CallupCO(p);
		}	
		static void Destroy(CallupCO* p)
		{
			delete p->GetContent();
			delete p;
		}
		CallupCO(Callup* _pObj) : pObj(_pObj)	
		{
			if (method_map_get.size() == 0)
			{
				method_map_get["id"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetId);
				method_map_get["count"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetCount);
				method_map_get["livetime"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetLivetime);
				method_map_get["hp"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetHp);
				method_map_get["attackmode"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetAttackmode);
				method_map_get["restricttarget"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetRestricttarget);
				method_map_get["rectlength"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetRectlength);
				method_map_get["attack"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetAttack);
				method_map_get["poison"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetPoison);
				method_map_get["attackwater"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetAttackwater);
				method_map_get["woodfactor"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetWoodfactor);
				method_map_get["woodprob"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetWoodprob);
				method_map_get["fixprob"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetFixprob);
				method_map_get["defence"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetDefence);
				method_map_get["velocity"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetVelocity);
				method_map_get["interval"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetInterval);
				method_map_get["view"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetView);
				method_map_get["searchdistance"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetSearchdistance);
				method_map_get["attackdistance"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::GetAttackdistance);
			}
			if (method_map_set.size() == 0)
			{
				method_map_set["id"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetId);
				method_map_set["count"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetCount);
				method_map_set["livetime"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetLivetime);
				method_map_set["hp"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetHp);
				method_map_set["attackmode"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetAttackmode);
				method_map_set["restricttarget"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetRestricttarget);
				method_map_set["rectlength"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetRectlength);
				method_map_set["attack"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetAttack);
				method_map_set["poison"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetPoison);
				method_map_set["attackwater"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetAttackwater);
				method_map_set["woodfactor"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetWoodfactor);
				method_map_set["woodprob"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetWoodprob);
				method_map_set["fixprob"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetFixprob);
				method_map_set["defence"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetDefence);
				method_map_set["velocity"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetVelocity);
				method_map_set["interval"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetInterval);
				method_map_set["view"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetView);
				method_map_set["searchdistance"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetSearchdistance);
				method_map_set["attackdistance"]=(memfunc_pointer_t)mem_fun_p(&CallupCO::SetAttackdistance);
			}
		}	
		virtual ~CallupCO() 
		{
		}

		template <typename _t>
		bool GetProperty(const std::string& p_name,_t & value)
		{
			Method_Map::iterator it=method_map_get.find(p_name);
			if (it == method_map_get.end()) return false;
			unary_function_t<_t,CallupCO>* pFunc = dynamic_cast< unary_function_t<_t,CallupCO>* >((*it).second);
			if (pFunc == NULL) return false;
			value = (*pFunc)(this);
			return true;
		}
		template <typename _t>
		bool SetProperty(const std::string& p_name,const _t& value)   
		{
			Method_Map::iterator it=method_map_set.find(p_name);
			if (it == method_map_set.end()) return false;
			binary_function_t<void,_t,CallupCO>* pFunc = dynamic_cast< binary_function_t<void,_t,CallupCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,value);
			return true;
		}	
		
		template <typename _t>
		bool GetArrayValue(const int index,_t& value)
		{
			Method_Map::iterator it=method_map_get.find("value");
			if (it == method_map_get.end()) return false;
			const_binary_function_t<_t,int,CallupCO>* pFunc2 = dynamic_cast< const_binary_function_t<_t,int,CallupCO>* >((*it).second);
			if (pFunc2 == NULL) return false;
			value = (*pFunc2)(this,index);
			return true;
		}
		template <typename _t>	
		bool SetArrayValue(const int index,const _t& value)
		{
			Method_Map::iterator it=method_map_set.find("value");
			if (it == method_map_set.end()) return false;
			binary_function_t<void,const pair_t<_t>&,CallupCO>* pFunc = dynamic_cast< binary_function_t<void,const pair_t<_t>&,CallupCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,pair_t<_t>(index,value));
			return true;
		}	
	
	}; //end of class CallupCO
}; // end of namespace	

#endif
