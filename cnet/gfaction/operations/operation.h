/*
 * define base class of operation
 * @author: liping
 * @date: 2005-4-13
 */ 

#ifndef __GNET_OPERATION_H
#define __GNET_OPERATION_H
#include "macros.h"
#include "ids.hxx"
#include "privilege.h"
//#include "factiondb.h"

#include "marshal.h"
#include "errcode.h"
#include "gfactionserver.hpp"

#include "opresult.hxx"

#define __SYNCDATA__ pWrapper->GetSyncData()
#define WRAPPER_WREF (OperWrapper::FindWrapper(nWrapperID))
namespace GNET
{
	class OperWrapper;
	//define base class of all operation on a Faction
	class Operation
	{
		public:
			typedef GNET::Operations Type;
		protected:
			Type type;
			OperWrapper* pWrapper;
			unsigned int nWrapperID;
			bool         blDecodeContext;

			virtual bool PrivilegePolicy(GNET::Roles _role) 
			{ return  Privilege::GetStub(_role)==NULL ? false:Privilege::GetStub(_role)->PrivilegeCheck(type); }
			
			virtual bool PrivilegeCheck() { return true; }
			virtual bool ConditionCheck() { return true; }
			virtual bool PrepareContext() { return true; }
			virtual bool DecodeContext()
			{
				if ( blDecodeContext ) return true;
				bool err_log=false;
				try 
				{
					if ( PrepareContext() )
					{
						blDecodeContext=true;
						return true;
					}
				}
				catch (Marshal::Exception )
				{
					err_log=true;
				}
				if ( err_log )
					Log::log(LOG_ERR,"gfaction:operation params unmarshal error!");
				return false; //marshal error
			}
			virtual int	CheckAll()
			{
				if (!blDecodeContext && !DecodeContext()) { return ERR_FC_DATAERROR; }
				if (!PrivilegeCheck()) { return ERR_FC_NO_PRIVILEGE; }
				if (!ConditionCheck()) { return ERR_FC_CHECKCONDITION; }
				return ERR_SUCCESS;/* ERR_SUCCESS=0 */
			}
		private:
			typedef std::map<Type,Operation*> OperMap;
			static OperMap& GetMap()
			{
				static OperMap map;
				return map;
			}
			static Operation* GetStub(Type type) {
				OperMap::iterator it=GetMap().find(type);
				return it==GetMap().end() ? NULL : (*it).second;
			}
		public:
			Operation(Type _type) : type(_type)
			{ 
				if (GetStub(_type)==NULL) GetMap().insert(std::make_pair(_type,this)); 
				pWrapper=NULL;
				nWrapperID=0;
				blDecodeContext=false;
			}
			virtual ~Operation() { }
			
			static Operation* Create(Type _type) {
				return GetStub(_type)==NULL ? NULL : GetStub(_type)->Clone();
			}
			virtual Operation* Clone() {
				return new Operation(type);
			}
			Type GetType() { return type; }
			//Attach a Wrapper object 
			void AttachWrapper(OperWrapper* _p,unsigned int _wid) { pWrapper=_p; nWrapperID=_wid; }
			unsigned int GetWrapperID() { return nWrapperID; }
			//Execute operation
			virtual int Execute() { return 0; }
			virtual int QueryAddInfo() { return 0; }
			virtual void SetResult(void* pResult) { return; }

			virtual bool NeedSync() {  return false; }
			virtual bool NeedAddInfo() { return false; }
	};
	
};//end of namespace
#endif
