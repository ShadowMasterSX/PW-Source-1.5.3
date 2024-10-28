
#ifndef __GNET_DEBUGADDCASH_HPP
#define __GNET_DEBUGADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class DebugAddCash : public GNET::Protocol
{
	#include "debugaddcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		try
		{
			StorageEnv::Storage * puser = StorageEnv::GetStorage("user");
			StorageEnv::CommonTransaction txn;
			Marshal::OctetsStream key;
			User user;
			try
			{
				key << (unsigned int)userid;
				Marshal::OctetsStream(puser->find(key,txn))>>user;
				if(user.cash_used > 2000000000)	//gs debug加元宝命令会减少cash_used，导致该值接近unsigned int上限
				{
					user.cash_buy = 0;
					user.cash_sell = 0;
					user.cash_used = 0;
					user.cash_add = 0;
					user.cash = 0;
					user.cash_sysauction.clear();
				}
				user.cash_add += cash;
				LOG_TRACE("debugaddcash userid=%d:cash_add=%d:cash_used=%d:cash_buy=%d:cash_sell=%d", userid, user.cash_add, user.cash_used, user.cash_buy, user.cash_sell);
				puser->insert( key, Marshal::OctetsStream()<<user, txn );
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				DbException ee( DB_OLD_VERSION );
				txn.abort( ee );
				throw ee;
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "DebugAddCash, userid=%d, what=%s\n", userid, e.what() );
		}

	}
};

};

#endif
