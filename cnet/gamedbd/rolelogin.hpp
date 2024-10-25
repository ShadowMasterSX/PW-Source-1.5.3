
#ifndef __GNET_ROLELOGIN_HPP
#define __GNET_ROLELOGIN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "timer.h"

namespace GNET
{

class RoleLogin : public GNET::Protocol
{
	#include "rolelogin"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Marshal::OctetsStream key;

		LOG_TRACE( "RoleLogin, update lastlogin_time=%d, roleid=%d.\n", Timer::GetTime(), roleid );

		key << roleid;
		try
		{
			StorageEnv::Storage * pbase = StorageEnv::GetStorage("base");
			StorageEnv::CommonTransaction txn;
			try
			{
				GRoleBase	base;
				Marshal::OctetsStream( pbase->find( key, txn ) ) >> base;
				base.lastlogin_time = Timer::GetTime();
				pbase->insert( key, Marshal::OctetsStream() << base, txn );
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
			Log::log( LOG_ERR, "RoleLogin, roleid=%d, what=%s\n", roleid, e.what() );
		}
	}
};

};

#endif
