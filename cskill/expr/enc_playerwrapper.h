#ifndef __GNET_PLAYERWRAPPER_H
#define __GNET_PLAYERWRAPPER_H
#include <string>
#include "bridge.h"
	
#include "playerwrapper.h"	
#include "enc_skilllevel.h"
#include "enc_callup.h"
namespace GNET
{
	class PlayerWrapperCO : public CommonObject
	{
	private:
		PlayerWrapper* pObj;
		mutable SkillLevelCO*	skilllevel;
		mutable CallupCO*	callup;

		static Method_Map method_map_get,method_map_set;
		float GetProbability() const { return pObj->GetProbability(); }
		void  SetProbability(float value) { pObj->SetProbability(value); }
		int GetTime() const { return pObj->GetTime(); }
		void  SetTime(int value) { pObj->SetTime(value); }
		float GetRatio() const { return pObj->GetRatio(); }
		void  SetRatio(float value) { pObj->SetRatio(value); }
		float GetIncrement() const { return pObj->GetIncrement(); }
		void  SetIncrement(float value) { pObj->SetIncrement(value); }
		float GetAmount() const { return pObj->GetAmount(); }
		void  SetAmount(float value) { pObj->SetAmount(value); }
		float GetValue() const { return pObj->GetValue(); }
		void  SetValue(float value) { pObj->SetValue(value); }
		float GetDistance() const { return pObj->GetDistance(); }
		void  SetDistance(float value) { pObj->SetDistance(value); }
		float GetFactor() const { return pObj->GetFactor(); }
		void  SetFactor(float value) { pObj->SetFactor(value); }
		float GetDamage() const { return pObj->GetDamage(); }
		void  SetDamage(float value) { pObj->SetDamage(value); }
		float GetGolddamage() const { return pObj->GetGolddamage(); }
		void  SetGolddamage(float value) { pObj->SetGolddamage(value); }
		float GetWooddamage() const { return pObj->GetWooddamage(); }
		void  SetWooddamage(float value) { pObj->SetWooddamage(value); }
		float GetWaterdamage() const { return pObj->GetWaterdamage(); }
		void  SetWaterdamage(float value) { pObj->SetWaterdamage(value); }
		float GetFiredamage() const { return pObj->GetFiredamage(); }
		void  SetFiredamage(float value) { pObj->SetFiredamage(value); }
		float GetEarthdamage() const { return pObj->GetEarthdamage(); }
		void  SetEarthdamage(float value) { pObj->SetEarthdamage(value); }
		CommonObject* GetSkilllevel() const 
		{
			if (skilllevel)
			{
				skilllevel->SetContent(pObj->GetSkilllevel());
				return skilllevel;
			}
			return skilllevel=new SkillLevelCO(pObj->GetSkilllevel());
		}
		void SetSkilllevel(CommonObject* co)
		{
			SkillLevelCO* wrapper=dynamic_cast<SkillLevelCO*>(co);
			if (wrapper != NULL) pObj->SetSkilllevel(wrapper->GetContent());
		}
		int GetLevel() const { return pObj->GetLevel(); }
		void  SetLevel(int value) { pObj->SetLevel(value); }
		int GetDecmp() const { return pObj->GetDecmp(); }
		void  SetDecmp(int value) { pObj->SetDecmp(value); }
		int GetDecsp() const { return pObj->GetDecsp(); }
		void  SetDecsp(int value) { pObj->SetDecsp(value); }
		int GetMp() const { return pObj->GetMp(); }
		void  SetMp(int value) { pObj->SetMp(value); }
		int GetSp() const { return pObj->GetSp(); }
		void  SetSp(int value) { pObj->SetSp(value); }
		float GetWeapondistance() const { return pObj->GetWeapondistance(); }
		void  SetWeapondistance(float value) { pObj->SetWeapondistance(value); }
		int GetPray() const { return pObj->GetPray(); }
		void  SetPray(int value) { pObj->SetPray(value); }
		int GetInform() const { return pObj->GetInform(); }
		void  SetInform(int value) { pObj->SetInform(value); }
		int GetPerform() const { return pObj->GetPerform(); }
		void  SetPerform(int value) { pObj->SetPerform(value); }
		int GetEnhancedefence() const { return pObj->GetEnhancedefence(); }
		void  SetEnhancedefence(int value) { pObj->SetEnhancedefence(value); }
		int GetEnhancespeed() const { return pObj->GetEnhancespeed(); }
		void  SetEnhancespeed(int value) { pObj->SetEnhancespeed(value); }
		int GetEnhancehp() const { return pObj->GetEnhancehp(); }
		void  SetEnhancehp(int value) { pObj->SetEnhancehp(value); }
		float GetIncswordattack() const { return pObj->GetIncswordattack(); }
		void  SetIncswordattack(float value) { pObj->SetIncswordattack(value); }
		float GetIncspearattack() const { return pObj->GetIncspearattack(); }
		void  SetIncspearattack(float value) { pObj->SetIncspearattack(value); }
		float GetIncaxeattack() const { return pObj->GetIncaxeattack(); }
		void  SetIncaxeattack(float value) { pObj->SetIncaxeattack(value); }
		float GetIncboxingattack() const { return pObj->GetIncboxingattack(); }
		void  SetIncboxingattack(float value) { pObj->SetIncboxingattack(value); }
		int GetIncskillgold() const { return pObj->GetIncskillgold(); }
		void  SetIncskillgold(int value) { pObj->SetIncskillgold(value); }
		int GetIncskillwood() const { return pObj->GetIncskillwood(); }
		void  SetIncskillwood(int value) { pObj->SetIncskillwood(value); }
		int GetIncskillwater() const { return pObj->GetIncskillwater(); }
		void  SetIncskillwater(int value) { pObj->SetIncskillwater(value); }
		int GetIncskillfire() const { return pObj->GetIncskillfire(); }
		void  SetIncskillfire(int value) { pObj->SetIncskillfire(value); }
		int GetIncskillearth() const { return pObj->GetIncskillearth(); }
		void  SetIncskillearth(int value) { pObj->SetIncskillearth(value); }
		int GetIncmprestore() const { return pObj->GetIncmprestore(); }
		void  SetIncmprestore(int value) { pObj->SetIncmprestore(value); }
		float GetIncdodge() const { return pObj->GetIncdodge(); }
		void  SetIncdodge(float value) { pObj->SetIncdodge(value); }
		float GetIncdefence() const { return pObj->GetIncdefence(); }
		void  SetIncdefence(float value) { pObj->SetIncdefence(value); }
		float GetIncattackratio() const { return pObj->GetIncattackratio(); }
		void  SetIncattackratio(float value) { pObj->SetIncattackratio(value); }
		CommonObject* GetCallup() const 
		{
			if (callup)
			{
				callup->SetContent(pObj->GetCallup());
				return callup;
			}
			return callup=new CallupCO(pObj->GetCallup());
		}
		void SetCallup(CommonObject* co)
		{
			CallupCO* wrapper=dynamic_cast<CallupCO*>(co);
			if (wrapper != NULL) pObj->SetCallup(wrapper->GetContent());
		}
		int GetStartcallup() const { return pObj->GetStartcallup(); }
		void  SetStartcallup(int value) { pObj->SetStartcallup(value); }
		bool GetRepel() const { return pObj->GetRepel(); }
		void  SetRepel(bool value) { pObj->SetRepel(value); }
		bool GetBreak() const { return pObj->GetBreak(); }
		void  SetBreak(bool value) { pObj->SetBreak(value); }
		bool GetPassive() const { return pObj->GetPassive(); }
		void  SetPassive(bool value) { pObj->SetPassive(value); }
		bool GetDebaseresist() const { return pObj->GetDebaseresist(); }
		void  SetDebaseresist(bool value) { pObj->SetDebaseresist(value); }
		bool GetAwed() const { return pObj->GetAwed(); }
		void  SetAwed(bool value) { pObj->SetAwed(value); }
		bool GetFrighten() const { return pObj->GetFrighten(); }
		void  SetFrighten(bool value) { pObj->SetFrighten(value); }
		bool GetSwivet() const { return pObj->GetSwivet(); }
		void  SetSwivet(bool value) { pObj->SetSwivet(value); }
		bool GetSleep() const { return pObj->GetSleep(); }
		void  SetSleep(bool value) { pObj->SetSleep(value); }
		bool GetComa() const { return pObj->GetComa(); }
		void  SetComa(bool value) { pObj->SetComa(value); }
		bool GetFix() const { return pObj->GetFix(); }
		void  SetFix(bool value) { pObj->SetFix(value); }
		bool GetEnvelop() const { return pObj->GetEnvelop(); }
		void  SetEnvelop(bool value) { pObj->SetEnvelop(value); }
		bool GetSlow() const { return pObj->GetSlow(); }
		void  SetSlow(bool value) { pObj->SetSlow(value); }
		bool GetPhysicalinjury() const { return pObj->GetPhysicalinjury(); }
		void  SetPhysicalinjury(bool value) { pObj->SetPhysicalinjury(value); }
		bool GetMetalinjury() const { return pObj->GetMetalinjury(); }
		void  SetMetalinjury(bool value) { pObj->SetMetalinjury(value); }
		bool GetWoodinjury() const { return pObj->GetWoodinjury(); }
		void  SetWoodinjury(bool value) { pObj->SetWoodinjury(value); }
		bool GetWaterinjury() const { return pObj->GetWaterinjury(); }
		void  SetWaterinjury(bool value) { pObj->SetWaterinjury(value); }
		bool GetFireinjury() const { return pObj->GetFireinjury(); }
		void  SetFireinjury(bool value) { pObj->SetFireinjury(value); }
		bool GetEarthinjury() const { return pObj->GetEarthinjury(); }
		void  SetEarthinjury(bool value) { pObj->SetEarthinjury(value); }
		int GetHeal() const { return pObj->GetHeal(); }
		void  SetHeal(int value) { pObj->SetHeal(value); }
		float GetRevive() const { return pObj->GetRevive(); }
		void  SetRevive(float value) { pObj->SetRevive(value); }
		bool GetFeathershield() const { return pObj->GetFeathershield(); }
		void  SetFeathershield(bool value) { pObj->SetFeathershield(value); }
	public:
		PlayerWrapper* GetContent() { return pObj; }
		void SetContent(PlayerWrapper* _pObj) { pObj=_pObj; }
		static PlayerWrapperCO* Create()
		{
			PlayerWrapper* p=new PlayerWrapper();
			return new PlayerWrapperCO(p);
		}	
		static void Destroy(PlayerWrapperCO* p)
		{
			delete p->GetContent();
			delete p;
		}
		PlayerWrapperCO(PlayerWrapper* _pObj) : pObj(_pObj),skilllevel(NULL),callup(NULL)	
		{
			if (method_map_get.size() == 0)
			{
				method_map_get["probability"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetProbability);
				method_map_get["time"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetTime);
				method_map_get["ratio"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetRatio);
				method_map_get["increment"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncrement);
				method_map_get["amount"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetAmount);
				method_map_get["value"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetValue);
				method_map_get["distance"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetDistance);
				method_map_get["factor"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetFactor);
				method_map_get["damage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetDamage);
				method_map_get["golddamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetGolddamage);
				method_map_get["wooddamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetWooddamage);
				method_map_get["waterdamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetWaterdamage);
				method_map_get["firedamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetFiredamage);
				method_map_get["earthdamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetEarthdamage);
				method_map_get["skilllevel"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetSkilllevel);
				method_map_get["level"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetLevel);
				method_map_get["decmp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetDecmp);
				method_map_get["decsp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetDecsp);
				method_map_get["mp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetMp);
				method_map_get["sp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetSp);
				method_map_get["weapondistance"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetWeapondistance);
				method_map_get["pray"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetPray);
				method_map_get["inform"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetInform);
				method_map_get["perform"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetPerform);
				method_map_get["enhancedefence"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetEnhancedefence);
				method_map_get["enhancespeed"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetEnhancespeed);
				method_map_get["enhancehp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetEnhancehp);
				method_map_get["incswordattack"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncswordattack);
				method_map_get["incspearattack"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncspearattack);
				method_map_get["incaxeattack"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncaxeattack);
				method_map_get["incboxingattack"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncboxingattack);
				method_map_get["incskillgold"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncskillgold);
				method_map_get["incskillwood"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncskillwood);
				method_map_get["incskillwater"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncskillwater);
				method_map_get["incskillfire"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncskillfire);
				method_map_get["incskillearth"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncskillearth);
				method_map_get["incmprestore"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncmprestore);
				method_map_get["incdodge"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncdodge);
				method_map_get["incdefence"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncdefence);
				method_map_get["incattackratio"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetIncattackratio);
				method_map_get["callup"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetCallup);
				method_map_get["startcallup"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetStartcallup);
				method_map_get["repel"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetRepel);
				method_map_get["break"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetBreak);
				method_map_get["passive"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetPassive);
				method_map_get["debaseresist"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetDebaseresist);
				method_map_get["awed"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetAwed);
				method_map_get["frighten"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetFrighten);
				method_map_get["swivet"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetSwivet);
				method_map_get["sleep"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetSleep);
				method_map_get["coma"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetComa);
				method_map_get["fix"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetFix);
				method_map_get["envelop"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetEnvelop);
				method_map_get["slow"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetSlow);
				method_map_get["physicalinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetPhysicalinjury);
				method_map_get["metalinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetMetalinjury);
				method_map_get["woodinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetWoodinjury);
				method_map_get["waterinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetWaterinjury);
				method_map_get["fireinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetFireinjury);
				method_map_get["earthinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetEarthinjury);
				method_map_get["heal"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetHeal);
				method_map_get["revive"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetRevive);
				method_map_get["feathershield"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::GetFeathershield);
			}
			if (method_map_set.size() == 0)
			{
				method_map_set["probability"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetProbability);
				method_map_set["time"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetTime);
				method_map_set["ratio"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetRatio);
				method_map_set["increment"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncrement);
				method_map_set["amount"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetAmount);
				method_map_set["value"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetValue);
				method_map_set["distance"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetDistance);
				method_map_set["factor"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetFactor);
				method_map_set["damage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetDamage);
				method_map_set["golddamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetGolddamage);
				method_map_set["wooddamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetWooddamage);
				method_map_set["waterdamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetWaterdamage);
				method_map_set["firedamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetFiredamage);
				method_map_set["earthdamage"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetEarthdamage);
				method_map_set["skilllevel"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetSkilllevel);
				method_map_set["level"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetLevel);
				method_map_set["decmp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetDecmp);
				method_map_set["decsp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetDecsp);
				method_map_set["mp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetMp);
				method_map_set["sp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetSp);
				method_map_set["weapondistance"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetWeapondistance);
				method_map_set["pray"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetPray);
				method_map_set["inform"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetInform);
				method_map_set["perform"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetPerform);
				method_map_set["enhancedefence"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetEnhancedefence);
				method_map_set["enhancespeed"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetEnhancespeed);
				method_map_set["enhancehp"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetEnhancehp);
				method_map_set["incswordattack"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncswordattack);
				method_map_set["incspearattack"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncspearattack);
				method_map_set["incaxeattack"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncaxeattack);
				method_map_set["incboxingattack"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncboxingattack);
				method_map_set["incskillgold"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncskillgold);
				method_map_set["incskillwood"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncskillwood);
				method_map_set["incskillwater"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncskillwater);
				method_map_set["incskillfire"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncskillfire);
				method_map_set["incskillearth"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncskillearth);
				method_map_set["incmprestore"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncmprestore);
				method_map_set["incdodge"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncdodge);
				method_map_set["incdefence"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncdefence);
				method_map_set["incattackratio"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetIncattackratio);
				method_map_set["callup"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetCallup);
				method_map_set["startcallup"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetStartcallup);
				method_map_set["repel"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetRepel);
				method_map_set["break"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetBreak);
				method_map_set["passive"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetPassive);
				method_map_set["debaseresist"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetDebaseresist);
				method_map_set["awed"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetAwed);
				method_map_set["frighten"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetFrighten);
				method_map_set["swivet"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetSwivet);
				method_map_set["sleep"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetSleep);
				method_map_set["coma"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetComa);
				method_map_set["fix"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetFix);
				method_map_set["envelop"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetEnvelop);
				method_map_set["slow"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetSlow);
				method_map_set["physicalinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetPhysicalinjury);
				method_map_set["metalinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetMetalinjury);
				method_map_set["woodinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetWoodinjury);
				method_map_set["waterinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetWaterinjury);
				method_map_set["fireinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetFireinjury);
				method_map_set["earthinjury"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetEarthinjury);
				method_map_set["heal"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetHeal);
				method_map_set["revive"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetRevive);
				method_map_set["feathershield"]=(memfunc_pointer_t)mem_fun_p(&PlayerWrapperCO::SetFeathershield);
			}
		}	
		virtual ~PlayerWrapperCO() 
		{
			delete skilllevel;
			delete callup;
		}

		template <typename _t>
		bool GetProperty(const std::string& p_name,_t & value)
		{
			Method_Map::iterator it=method_map_get.find(p_name);
			if (it == method_map_get.end()) return false;
			unary_function_t<_t,PlayerWrapperCO>* pFunc = dynamic_cast< unary_function_t<_t,PlayerWrapperCO>* >((*it).second);
			if (pFunc == NULL) return false;
			value = (*pFunc)(this);
			return true;
		}
		template <typename _t>
		bool SetProperty(const std::string& p_name,const _t& value)   
		{
			Method_Map::iterator it=method_map_set.find(p_name);
			if (it == method_map_set.end()) return false;
			binary_function_t<void,_t,PlayerWrapperCO>* pFunc = dynamic_cast< binary_function_t<void,_t,PlayerWrapperCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,value);
			return true;
		}	
		
		template <typename _t>
		bool GetArrayValue(const int index,_t& value)
		{
			Method_Map::iterator it=method_map_get.find("value");
			if (it == method_map_get.end()) return false;
			const_binary_function_t<_t,int,PlayerWrapperCO>* pFunc2 = dynamic_cast< const_binary_function_t<_t,int,PlayerWrapperCO>* >((*it).second);
			if (pFunc2 == NULL) return false;
			value = (*pFunc2)(this,index);
			return true;
		}
		template <typename _t>	
		bool SetArrayValue(const int index,const _t& value)
		{
			Method_Map::iterator it=method_map_set.find("value");
			if (it == method_map_set.end()) return false;
			binary_function_t<void,const pair_t<_t>&,PlayerWrapperCO>* pFunc = dynamic_cast< binary_function_t<void,const pair_t<_t>&,PlayerWrapperCO>* >((*it).second);
			if (pFunc == NULL) return false;
			(*pFunc)(this,pair_t<_t>(index,value));
			return true;
		}	
	
	}; //end of class PlayerWrapperCO
}; // end of namespace	

#endif
