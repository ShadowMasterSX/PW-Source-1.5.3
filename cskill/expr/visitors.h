#ifndef __GNET_VISITORS_H
#define __GNET_VISITORS_H
#include "bridge.h"
#include "array.h"
#include "objectgraph.hpp"
#include "enc_skilllevel.h"
#include "enc_callup.h"
#include "enc_playerwrapper.h"
#include "enc_targetwrapper.h"
#include "enc_skill.h"
namespace GNET
{	
	class OperationVisitor : public Visitor
	{
		CommonObject* co;
	public:	
		void visit(CommonObject* _co)
		{
			co=_co;
		}
		static CommonObject* CreateArray(int size,unsigned int type)
		{
			switch(type)
			{
				case BOOL_T: { return ArrayCO<bool>::Create(size); } 
				case CHAR_T: { return ArrayCO<char>::Create(size); }
				case INT_T: { return ArrayCO<int>::Create(size); } 
				case FLOAT_T: { return ArrayCO<float>::Create(size); } 
				case DOUBLE_T: { return ArrayCO<double>::Create(size); } 
				default:
					return NULL;			   
			}
		}
		static void DestroyArray(CommonObject* co,unsigned int type)
		{
			switch(type)
			{
				case BOOL_T: 
				{ 
					ArrayCO<bool>* p=dynamic_cast<ArrayCO<bool>*>(co);
					if (p!=NULL) ArrayCO<bool>::Destroy(p);
					break;
				}
				case CHAR_T: 
				{ 
					ArrayCO<char>* p=dynamic_cast<ArrayCO<char>*>(co);
					if (p!=NULL) ArrayCO<char>::Destroy(p);
					break;
				}
				case INT_T: 
				{ 
					ArrayCO<int>* p=dynamic_cast<ArrayCO<int>*>(co);
					if (p!=NULL) ArrayCO<int>::Destroy(p);
					break;
				}
				case FLOAT_T: 
				{ 
					ArrayCO<float>* p=dynamic_cast<ArrayCO<float>*>(co);
					if (p!=NULL) ArrayCO<float>::Destroy(p);
					break;
				}
				case DOUBLE_T: 
				{ 
					ArrayCO<double>* p=dynamic_cast<ArrayCO<double>*>(co);
					if (p!=NULL) ArrayCO<double>::Destroy(p);
					break;
				}
				default:
					return;			   
			}
		}

		static CommonObject* CreateCO(unsigned int type,ObjectGraph& og)
		{
			if (type==og.GetIdentifier("SkillLevel")) { return SkillLevelCO::Create(); }
			if (type==og.GetIdentifier("Callup")) { return CallupCO::Create(); }
			if (type==og.GetIdentifier("PlayerWrapper")) { return PlayerWrapperCO::Create(); }
			if (type==og.GetIdentifier("TargetWrapper")) { return TargetWrapperCO::Create(); }
			if (type==og.GetIdentifier("Skill")) { return SkillCO::Create(); }
			return NULL;
		}
		static void DestroyCO(CommonObject* co)
		{
			if (co == NULL) return;
			{
				SkillLevelCO* pco=dynamic_cast<SkillLevelCO*>(co);
				if (pco!=NULL) SkillLevelCO ::Destroy(pco);
				return;
			}	
			{
				CallupCO* pco=dynamic_cast<CallupCO*>(co);
				if (pco!=NULL) CallupCO ::Destroy(pco);
				return;
			}	
			{
				PlayerWrapperCO* pco=dynamic_cast<PlayerWrapperCO*>(co);
				if (pco!=NULL) PlayerWrapperCO ::Destroy(pco);
				return;
			}	
			{
				TargetWrapperCO* pco=dynamic_cast<TargetWrapperCO*>(co);
				if (pco!=NULL) TargetWrapperCO ::Destroy(pco);
				return;
			}	
			{
				SkillCO* pco=dynamic_cast<SkillCO*>(co);
				if (pco!=NULL) SkillCO ::Destroy(pco);
				return;
			}	
		}
		template<typename _t>
  		bool Get(const std::string& p_name,_t& value)
		{
			if (co == NULL) return false;
			{
				SkillLevelCO* pco=dynamic_cast<SkillLevelCO*>(co);
				if (pco!=NULL) return pco->GetProperty(p_name,value);
			}	
			{
				CallupCO* pco=dynamic_cast<CallupCO*>(co);
				if (pco!=NULL) return pco->GetProperty(p_name,value);
			}	
			{
				PlayerWrapperCO* pco=dynamic_cast<PlayerWrapperCO*>(co);
				if (pco!=NULL) return pco->GetProperty(p_name,value);
			}	
			{
				TargetWrapperCO* pco=dynamic_cast<TargetWrapperCO*>(co);
				if (pco!=NULL) return pco->GetProperty(p_name,value);
			}	
			{
				SkillCO* pco=dynamic_cast<SkillCO*>(co);
				if (pco!=NULL) return pco->GetProperty(p_name,value);
			}	
			{
				ArrayCO<int>* pco=dynamic_cast<ArrayCO<int>*>(co);
				if (pco!=NULL) return pco->GetProperty(p_name,value);
			}
			{
				ArrayCO<float>* pco=dynamic_cast<ArrayCO<float>*>(co);
				if (pco!=NULL) return pco->GetProperty(p_name,value);
			}
			return false;
		}
		template<typename _t>
  		bool Set(const std::string& p_name,_t& value)
		{
			if (co == NULL) return false;
			{
				SkillLevelCO* pco=dynamic_cast<SkillLevelCO*>(co);
				if (pco!=NULL) return pco->SetProperty(p_name,value);
			}	
			{
				CallupCO* pco=dynamic_cast<CallupCO*>(co);
				if (pco!=NULL) return pco->SetProperty(p_name,value);
			}	
			{
				PlayerWrapperCO* pco=dynamic_cast<PlayerWrapperCO*>(co);
				if (pco!=NULL) return pco->SetProperty(p_name,value);
			}	
			{
				TargetWrapperCO* pco=dynamic_cast<TargetWrapperCO*>(co);
				if (pco!=NULL) return pco->SetProperty(p_name,value);
			}	
			{
				SkillCO* pco=dynamic_cast<SkillCO*>(co);
				if (pco!=NULL) return pco->SetProperty(p_name,value);
			}	
			{
				ArrayCO<int>* pco=dynamic_cast<ArrayCO<int>*>(co);
				if (pco!=NULL) return pco->SetProperty(p_name,value);
			}
			{
				ArrayCO<float>* pco=dynamic_cast<ArrayCO<float>*>(co);
				if (pco!=NULL) return pco->SetProperty(p_name,value);
			}
			return false;
		}
		template<typename _t>
  		bool Get(const int index,_t& value)
		{
			if (co == NULL) return false;
			{
				SkillLevelCO* pco=dynamic_cast<SkillLevelCO*>(co);
				if (pco!=NULL) return pco->GetArrayValue(index,value);
			}	
			{
				CallupCO* pco=dynamic_cast<CallupCO*>(co);
				if (pco!=NULL) return pco->GetArrayValue(index,value);
			}	
			{
				PlayerWrapperCO* pco=dynamic_cast<PlayerWrapperCO*>(co);
				if (pco!=NULL) return pco->GetArrayValue(index,value);
			}	
			{
				TargetWrapperCO* pco=dynamic_cast<TargetWrapperCO*>(co);
				if (pco!=NULL) return pco->GetArrayValue(index,value);
			}	
			{
				SkillCO* pco=dynamic_cast<SkillCO*>(co);
				if (pco!=NULL) return pco->GetArrayValue(index,value);
			}	
			{
				ArrayCO<int>* pco=dynamic_cast<ArrayCO<int>*>(co);
				if (pco!=NULL) return pco->GetArrayValue(index,value);
			}
			{
				ArrayCO<float>* pco=dynamic_cast<ArrayCO<float>*>(co);
				if (pco!=NULL) return pco->GetArrayValue(index,value);
			}
			return false;
		}
		template<typename _t>
  		bool Set(const int index,_t& value)
		{
			if (co == NULL) return false;
			{
				SkillLevelCO* pco=dynamic_cast<SkillLevelCO*>(co);
				if (pco!=NULL) return pco->SetArrayValue(index,value);
			}	
			{
				CallupCO* pco=dynamic_cast<CallupCO*>(co);
				if (pco!=NULL) return pco->SetArrayValue(index,value);
			}	
			{
				PlayerWrapperCO* pco=dynamic_cast<PlayerWrapperCO*>(co);
				if (pco!=NULL) return pco->SetArrayValue(index,value);
			}	
			{
				TargetWrapperCO* pco=dynamic_cast<TargetWrapperCO*>(co);
				if (pco!=NULL) return pco->SetArrayValue(index,value);
			}	
			{
				SkillCO* pco=dynamic_cast<SkillCO*>(co);
				if (pco!=NULL) return pco->SetArrayValue(index,value);
			}	
			{
				ArrayCO<int>* pco=dynamic_cast<ArrayCO<int>*>(co);
				if (pco!=NULL) return pco->SetArrayValue(index,value);
			}
			{
				ArrayCO<float>* pco=dynamic_cast<ArrayCO<float>*>(co);
				if (pco!=NULL) return pco->SetArrayValue(index,value);
			}
			return false;
		}
	};//end of class
}; //end of namespace

#endif
