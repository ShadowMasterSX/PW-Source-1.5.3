
#ifndef __GNET_POSTFACTIONRENAME_HPP
#define __GNET_POSTFACTIONRENAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PostFactionRename : public GNET::Protocol
{
	#include "postfactionrename"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		UniqueNameServer::GetInstance()->TransformName(newname);
		Octets name;
		CharsetConverter::conv_charset_u2l( newname, name );
		LOG_TRACE("PostFactionRename(retcode=%d, zoneid=%d factionid=%d newname=%.*s newnamesize=%d oldnamesize=%d)", retcode,zid,fid,name.size(),(char*)name.begin(),newname.size(),oldname.size());
		try
		{
			StorageEnv::Storage * punamefaction = StorageEnv::GetStorage("unamefaction");
			StorageEnv::AtomTransaction txn;
			try
			{
				Marshal::OctetsStream value_unamefaction,value_old;
				if(retcode)
				{
					punamefaction->del( newname, txn );
					LOG_TRACE("PostFacitonRename erase newname ok! fid=%d,name=%.*s ", fid,name.size(),(char*)name.begin());
				}
				else
				{
					int status = UNIQUENAME_USED;
					value_unamefaction << zid << (int)fid << status << (int)Timer::GetTime();
					punamefaction->insert(newname, value_unamefaction, txn);
					LOG_TRACE("PostFacitonRename modify newname ok! fid=%d,name=%.*s ", fid,name.size(),(char*)name.begin());
					//  ÀÏÃû×Ö·ÏÆú
					if(punamefaction->find(oldname, value_old, txn))
					{
						int old_zid,old_fid,old_status,old_time;
						value_old >> old_zid >> old_fid >> old_status >> old_time;
						if((unsigned int)old_fid == fid)
						{
							old_status = UNIQUENAME_OBSOLETE;
							value_old.clear();
							value_old << old_zid << old_fid << old_status << (int)Timer::GetTime();
							punamefaction->insert(oldname, value_old, txn);
						}
						else
							Log::log(LOG_ERR, "PostFactionRename oldfid %d newfid %d not equal", old_fid, fid);

					}
				}
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
			Log::log( LOG_ERR, "PostFactionRename failed, zoneid=%d, factionid=%d, what=%s\n", zid, fid, e.what() );
		}
	}
};

};

#endif
