#ifndef __GNET_SKILLLEVEL_H
#define __GNET_SKILLLEVEL_H
#include <string>
#include "bridge.h"
	
#include "skilllevel.h"	
namespace GNET
{
	class SkillLevelCO : public CommonObject
	{
	private:
		SkillLevel* pObj;

		static Method_Map method_map_get,method_map_set;
		int GetValue(int index) const { return pObj->GetValue(index); }
		void  SetValue(const pair_t<int>& p) { pObj->SetValue(p.index,p.value); }
	public:
		SkillLevel* GetContent() { return pObj; }
		void SetContent(SkillLevel* _pObj) { pObj=_pObj; }
		static SkillLevelCO* Create()
		{
			SkillLevel* p=new SkillLevel();
			return new SkillLevelCO(p);
		}	
		static void Destroy(SkillLevelCO* p)
		{
			delete p->GetContent();
			delete p;
		}
		SkillLevelCO(SkillLevel* _pObj) : pObj(_pObj)	
		{
			if (method_map_get.size() == 0)
			{
				method_map_get["value"]=(memfunc_pointer_t)mem_fun_p(&SkillLevelCO::GetValue);
			}
			if (method_map_set.size() == 0)
			{
				method_map_set["value"]=(memfunc_pointer_t)mem_fun_p(&SkillLevelCO::SetValue);
			}
		}	
		virtual ~SkillLevelCO() 
		{
		}

		template <typename _t>
		bool GetProperty(const std::string& p_name,_t & value)
		{
			Method_Map::iterator it=method_map_get.find(p_name);
			if (it == method_map_get.end()) return false;
			unary_function_t<_t,SkillLevelCO>* pFunc = dynamic_cast< unary_function_t<_t,SkillLevelCO>* >((*it).second);
			if (pFunc == NULL) return false;
			value = (*pFunc)(this);
			return true;
		}
		template <typename _t>
		bool SetProperty(const std::string& p_name,const _t& value)   
		{
			Method_Map::iterator it=method_map_set.find(p_name);
			if (it == method_map_set.end()) return false;
			binary_function_t<void,_t,SkillLevelCO>* pFunc = dynamic_cast< binary_function_t<void,_t,SkillLevelCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,value);
			return true;
		}	
		
		template <typename _t>
		bool GetArrayValue(const int index,_t& value)
		{
			Method_Map::iterator it=method_map_get.find("value");
			if (it == method_map_get.end()) return false;
			const_binary_function_t<_t,int,SkillLevelCO>* pFunc2 = dynamic_cast< const_binary_function_t<_t,int,SkillLevelCO>* >((*it).second);
			if (pFunc2 == NULL) return false;
			value = (*pFunc2)(this,index);
			return true;
		}
		template <typename _t>	
		bool SetArrayValue(const int index,const _t& value)
		{
			Method_Map::iterator it=method_map_set.find("value");
			if (it == method_map_set.end()) return false;
			binary_function_t<void,const pair_t<_t>&,SkillLevelCO>* pFunc = dynamic_cast< binary_function_t<void,const pair_t<_t>&,SkillLevelCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,pair_t<_t>(index,value));
			return true;
		}	
	
	}; //end of class SkillLevelCO
}; // end of namespace	

#endif
