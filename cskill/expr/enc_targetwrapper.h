#ifndef __GNET_TARGETWRAPPER_H
#define __GNET_TARGETWRAPPER_H
#include <string>
#include "bridge.h"
	
#include "targetwrapper.h"	
namespace GNET
{
	class TargetWrapperCO : public CommonObject
	{
	private:
		TargetWrapper* pObj;

		static Method_Map method_map_get,method_map_set;
		bool GetValid() const { return pObj->GetValid(); }
		void  SetValid(bool value) { pObj->SetValid(value); }
	public:
		TargetWrapper* GetContent() { return pObj; }
		void SetContent(TargetWrapper* _pObj) { pObj=_pObj; }
		static TargetWrapperCO* Create()
		{
			TargetWrapper* p=new TargetWrapper();
			return new TargetWrapperCO(p);
		}	
		static void Destroy(TargetWrapperCO* p)
		{
			delete p->GetContent();
			delete p;
		}
		TargetWrapperCO(TargetWrapper* _pObj) : pObj(_pObj)	
		{
			if (method_map_get.size() == 0)
			{
				method_map_get["valid"]=(memfunc_pointer_t)mem_fun_p(&TargetWrapperCO::GetValid);
			}
			if (method_map_set.size() == 0)
			{
				method_map_set["valid"]=(memfunc_pointer_t)mem_fun_p(&TargetWrapperCO::SetValid);
			}
		}	
		virtual ~TargetWrapperCO() 
		{
		}

		template <typename _t>
		bool GetProperty(const std::string& p_name,_t & value)
		{
			Method_Map::iterator it=method_map_get.find(p_name);
			if (it == method_map_get.end()) return false;
			unary_function_t<_t,TargetWrapperCO>* pFunc = dynamic_cast< unary_function_t<_t,TargetWrapperCO>* >((*it).second);
			if (pFunc == NULL) return false;
			value = (*pFunc)(this);
			return true;
		}
		template <typename _t>
		bool SetProperty(const std::string& p_name,const _t& value)   
		{
			Method_Map::iterator it=method_map_set.find(p_name);
			if (it == method_map_set.end()) return false;
			binary_function_t<void,_t,TargetWrapperCO>* pFunc = dynamic_cast< binary_function_t<void,_t,TargetWrapperCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,value);
			return true;
		}	
		
		template <typename _t>
		bool GetArrayValue(const int index,_t& value)
		{
			Method_Map::iterator it=method_map_get.find("value");
			if (it == method_map_get.end()) return false;
			const_binary_function_t<_t,int,TargetWrapperCO>* pFunc2 = dynamic_cast< const_binary_function_t<_t,int,TargetWrapperCO>* >((*it).second);
			if (pFunc2 == NULL) return false;
			value = (*pFunc2)(this,index);
			return true;
		}
		template <typename _t>	
		bool SetArrayValue(const int index,const _t& value)
		{
			Method_Map::iterator it=method_map_set.find("value");
			if (it == method_map_set.end()) return false;
			binary_function_t<void,const pair_t<_t>&,TargetWrapperCO>* pFunc = dynamic_cast< binary_function_t<void,const pair_t<_t>&,TargetWrapperCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,pair_t<_t>(index,value));
			return true;
		}	
	
	}; //end of class TargetWrapperCO
}; // end of namespace	

#endif
