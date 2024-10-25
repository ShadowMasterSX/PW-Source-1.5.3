#ifndef __GNET_ARRAYCO_H
#define __GNET_ARRAYCO_H
#include <string>
#include "bridge.h"

namespace GNET
{	
	template<typename __Array_Type>
	class ArrayCO : public CommonObject
	{
		__Array_Type* 	pObj;
		int				size;
		
		Method_Map method_map_get,method_map_set;
		
        int GetSize() const { return size; }
        void  SetSize(int value) { return; /*size=value;*/ }  //resize array is forbiddened
		
		__Array_Type GetValue(int index) const { return pObj[index]; }
		void SetValue(const pair_t<__Array_Type>& p) { pObj[p.index]=p.value; }
		
		ArrayCO(__Array_Type* _pObj) : pObj(_pObj)
		{
			if (method_map_get.size() == 0)
			{
				method_map_get["size"]=(memfunc_pointer_t)mem_fun_p(& ArrayCO::GetSize);
				method_map_get["value"]=(memfunc_pointer_t)mem_fun_p(& ArrayCO::GetValue);
			}
			if (method_map_set.size() == 0)
			{
				method_map_set["size"]=(memfunc_pointer_t)mem_fun_p(&ArrayCO::SetSize);
				method_map_set["value"]=(memfunc_pointer_t)mem_fun_p(&ArrayCO::SetValue);
			}
		}
		
	public:	
		__Array_Type* GetContent() { return pObj; }
		static ArrayCO* Create(int _size=1)
		{
			__Array_Type* p=new __Array_Type[_size];
			ArrayCO* co=new ArrayCO(p);
			//printf("createA %p\ncreateA co %p\n",p,co);
			co->size=_size;
			return co;
		}
		static void Destroy(ArrayCO* p)
		{
			//printf("destroyA %p\ndestroyA co %p\n",p->GetContent(),p);
			delete p->GetContent();
			delete p;
		}

		virtual ~ArrayCO() 
		{
		 	Method_Map::iterator it;
			it=method_map_get.begin();
			for(;it!=method_map_get.end();delete (*it).second,it++);		
			method_map_get.clear(); 
			
			it=method_map_set.begin();
			for(;it!=method_map_set.end();delete (*it).second,it++);		
			method_map_set.clear(); 
		}
		template <typename _t>
		bool GetProperty(const std::string& p_name,_t & value)
		{
			Method_Map::iterator it=method_map_get.find(p_name);
			if (it == method_map_get.end()) return false;
			unary_function_t<_t,ArrayCO<__Array_Type> >* pFunc = dynamic_cast< unary_function_t<_t,ArrayCO<__Array_Type> >* >((*it).second);
			if (pFunc == NULL) return false;
			value = (*pFunc)(this);
			return true;
		}
		template <typename _t>
		bool SetProperty(const std::string& p_name,const _t& value)   
		{
			Method_Map::iterator it=method_map_set.find(p_name);
			if (it == method_map_set.end()) return false;
			binary_function_t<void,_t,ArrayCO<__Array_Type> >* pFunc = dynamic_cast< binary_function_t<void,_t,ArrayCO<__Array_Type> >* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,value);
			return true;
		}
		template <typename _t>
		bool GetArrayValue(const int index,_t& value)
		{
			Method_Map::iterator it=method_map_get.find("value");
			if (it == method_map_get.end()) return false;
			const_binary_function_t<_t,int,ArrayCO<__Array_Type> >* pFunc2 = dynamic_cast< const_binary_function_t<_t,int,ArrayCO<__Array_Type> >* >((*it).second);
			if (pFunc2 == NULL) return false;
			value = (*pFunc2)(this,index);
			return true;
		}
		template <typename _t>	
		bool SetArrayValue(const int index,const _t& value)
		{
			Method_Map::iterator it=method_map_set.find("value");
			if (it == method_map_set.end()) return false;
			binary_function_t<void,const pair_t<_t>&,ArrayCO<__Array_Type> >* pFunc = dynamic_cast< binary_function_t<void,const pair_t<_t>&,ArrayCO<__Array_Type> >* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,pair_t<_t>(index,value));
			return true;
		}	
	};
};
#endif
