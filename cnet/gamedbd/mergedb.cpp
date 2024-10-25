#include <sys/types.h>
#include <dirent.h>

#include <set>
#include <map>
#include <vector>
#include <fstream>

#include "macros.h"
#include "accessdb.h"

#include "user"
#include "gauctionitem"
#include "gauctiondetail"
#include "localmacro.h"

namespace GNET
{

struct lt_Octets
{
	bool operator() (const Octets & o1, const Octets & o2) const
	{
		if( o1.size() < o2.size() )
			return true;
		if( o1.size() > o2.size() )
			return false;
		return memcmp(o1.begin(),o2.begin(),o2.size()) < 0;
	}
};

std::set<int>									g_sameid;
std::map<Octets, std::pair<int,int>, lt_Octets>	g_samename;
int	server2_usercount = 0;
int	same_usercount = 0;
int	server1_samerolecount = 0;
int	server2_rolecount = 0;


class MergeDBQuery : public StorageEnv::IQuery
{
public:
	std::string	destdbname;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		//static int count = 0;
		//if( ! (count++ % 10000) ) printf( "\tmerge database records counter %d...\n", count );
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage(destdbname.c_str());
			try
			{
				if( 0 == strcmp("user",destdbname.c_str()) )
				{
					server2_usercount ++;
					Marshal::OctetsStream	value_new, value1, value2(value);
					if( pstorage->find( key, value1, txn ) )
					{
						User	user1, user2;
						value1 >> user1;
						value2 >> user2;
						if( user1.logicuid != 16 && ROLELIST_DEFAULT != (user2.rolelist & user1.rolelist) )
							printf( "user duplicates, userid=%d, old=%x, new=%x.\n",
									user1.logicuid, user1.rolelist,user2.rolelist );
						RoleList r1(user1.rolelist), r2(user2.rolelist);
						if( r1.GetRoleCount() + r2.GetRoleCount() > 8 )
						{
							printf( "user role count more than 8, userid=%d, count1=%d, count2=%d.\n",
									user1.logicuid, r1.GetRoleCount(), r2.GetRoleCount() );
						}
						if( r1.GetRoleCount()>0 && r2.GetRoleCount()>0 )	same_usercount ++;
						server1_samerolecount += r1.GetRoleCount();
						server2_rolecount += r2.GetRoleCount();
						if( 0 == user1.logicuid )	user1.logicuid = user2.logicuid;
						user1.rolelist |= user2.rolelist;
						user1.cash += user2.cash;
						user1.money += user2.money;
						user1.cash_add += user2.cash_add;
						user1.cash_buy += user2.cash_buy;
						user1.cash_sell += user2.cash_sell;
						user1.cash_used += user2.cash_used;
						user1.add_serial = std::max(user1.add_serial, user2.add_serial);
						user1.use_serial = std::max(user1.use_serial, user2.use_serial);
						user1.exg_log.insert( user1.exg_log.end(), user2.exg_log.begin(), user2.exg_log.end() );
						while( user1.exg_log.size() > 80 )
							user1.exg_log.erase( user1.exg_log.begin() );
						user1.addiction.clear();
						if( user1.cash_password.size() == 0 && user2.cash_password.size() > 0 )
							user1.cash_password	=	user2.cash_password;

						value_new << user1;
						pstorage->insert( key, value_new, txn );
					}
					else
						pstorage->insert( key, value, txn );
				}
				else
				{
					Marshal::OctetsStream value1, value2(value);
					if( pstorage->find( key, value1, txn ) )
					{
						if( 0 == strcmp("rolename",destdbname.c_str()) )
						{
							Octets name;
							CharsetConverter::conv_charset_u2l( key, name );
							int roleid1, roleid2;
							value1 >> roleid1;
							value2 >> roleid2;
							g_samename.insert(std::make_pair(key,std::make_pair(roleid1,roleid2)));
							printf( "rolename(content=%.*s,id1=%d,id2=%d) already exists.\n",
									name.size(),(char*)name.begin(), roleid1, roleid2 );
						}
						else if( 0 == strcmp("factionname",destdbname.c_str()) )
						{
							Octets name;
							CharsetConverter::conv_charset_u2l( key, name );
							int fid1, fid2;
							value1 >> fid1;
							value2 >> fid2;
							printf( "factionname(content=%.*s,id1=%d,id2=%d) already exists.\n",
									name.size(),(char*)name.begin(), fid1, fid2 );
						}
						else if( 0 == strcmp("shoplog",destdbname.c_str()) )
						{
							/* OLD
							int64_t	shoplogid = 0, shoplogidnew = 0;
							static int64_t shoplogid1 = 0;
							shoplogid1	+=	0x0000000100000000LL;
							Marshal::OctetsStream os(key), os2;
							os >> shoplogid;
							shoplogidnew = ( (shoplogid & 0x00000000FFFFFFFFLL) | shoplogid1 );
							os2 << shoplogidnew;
							printf( "shoplog(key=%lld) already exists. change to %lld.\n", shoplogid, shoplogidnew );
							pstorage->insert( os2, value, txn );
							*/
							Octets	value_tmp;
							int64_t	shoplogid = 0, shoplogidnew = 0;
							static int64_t shoplogid_inc = 0;
							shoplogid_inc	+= 0x0000000100000000LL;
							Marshal::OctetsStream os(key);
							os >> shoplogid;
							shoplogidnew = shoplogid + shoplogid_inc;
							while( true )
							{
								if( !pstorage->find( Marshal::OctetsStream()<<shoplogidnew, value_tmp, txn ) )
								{
									printf( "shoplog(key=%lld) already exists. change to %lld.\n", shoplogid, shoplogidnew );
									pstorage->insert( Marshal::OctetsStream()<<shoplogidnew, value, txn );
									break;
								}
								shoplogid_inc	+= 0x0000000100000000LL;
								shoplogidnew	+= shoplogid_inc;
							}
						}
						else if( key.size() == 4 )
						{
							int key_int = -1;
							Marshal::OctetsStream os(key);
							os >> key_int;
							if( key_int >= 32 )
							{
								if( 0 == strcmp("base",destdbname.c_str()) )
									g_sameid.insert( key_int );
								printf( "key(size=%d,content=%d) already exists.\n",
										key.size(),key_int );
							}
						}
						else
							printf( "fatal duplicate error.\n" );
					}
					else
						pstorage->insert( key, value, txn );
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
			Log::log( LOG_ERR, "MergeDBQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeDB( const char * srcpath, const char * srcdbname, const char * destdbname )
{
	printf( "\nmerge database %s/%s to %s:\n", srcpath, srcdbname, destdbname );

	std::string data_dir = StorageEnv::get_datadir();
	std::string src_dir = srcpath;
	system( ("/bin/cp -f " + src_dir + "/" + srcdbname + " " + data_dir + "/conv_temp").c_str() );

	MergeDBQuery q;
	q.destdbname = destdbname;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( "conv_temp" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
			cursor.walk( q );
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
		Log::log( LOG_ERR, "MergeDB, error when walk, what=%s\n", e.what() );
	}
	StorageEnv::checkpoint();
	StorageEnv::Close();
	if( 0 == access( (data_dir + "/conv_temp").c_str(), R_OK ) )
		system( ("/bin/rm -f " + data_dir + "/conv_temp ").c_str() );
	StorageEnv::Open();
}

class WalkAuctionQuery : public StorageEnv::IQuery
{
public:
	int	maxid;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		Marshal::OctetsStream value_key = key;
		int id;
		value_key >> id;
		if( id > maxid )
			maxid = id;
		return true;
	}
};

class MergeAuctionQuery : public StorageEnv::IQuery
{
public:
	int	nextid;
	bool Update( StorageEnv::Transaction& txn, Octets& key, Octets& value)
	{
		try
		{
			StorageEnv::Storage * pstorage = StorageEnv::GetStorage("auction");
			try
			{
				Marshal::OctetsStream	value_key, value_value(value), value_value_new;
				int id = nextid;
				nextid ++;
				value_key << id;
				GAuctionDetail	auction;
				value_value >> auction;
				auction.info.auctionid = id;
				value_value_new << auction;
				pstorage->insert( value_key, value_value_new, txn );
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
			Log::log( LOG_ERR, "MergeAuctionQuery, what=%s\n", e.what() );
		}
		return true;
	}
};

void MergeAuction( const char * srcpath )
{
	printf( "\nmerge database %s/auction to auction:\n", srcpath );

	std::string data_dir = StorageEnv::get_datadir();
	std::string src_dir = srcpath;

	WalkAuctionQuery aq;
	MergeAuctionQuery maq;
	try
	{
		StorageEnv::Storage * pstorage = StorageEnv::GetStorage( "auction" );
		StorageEnv::Storage * pstorage2 = StorageEnv::GetStorage( "2_auction" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			{
				aq.maxid = 1;
				StorageEnv::Storage::Cursor cursor = pstorage->cursor( txn );
				cursor.walk( aq );
			}
			{
				maq.nextid = aq.maxid + 1;
				StorageEnv::Storage::Cursor cursor = pstorage2->cursor( txn );
				cursor.walk( maq );
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
		Log::log( LOG_ERR, "MergeAuction, error when walk, what=%s\n", e.what() );
	}
}

bool ShowRoleMainInfo( int roleid, GRoleStatus & status, const char * basetable, const char * statustable,
						const char * storehousetable )
{
	Marshal::OctetsStream key, value_base, value_status, value_storehouse;

	try
	{
		StorageEnv::Storage * pbase = StorageEnv::GetStorage( basetable );
		StorageEnv::Storage * pstatus = StorageEnv::GetStorage( statustable );
		StorageEnv::Storage * pstorehouse = StorageEnv::GetStorage( storehousetable );
		StorageEnv::AtomTransaction	txn;
		try
		{
			key << roleid;
			GRoleBase base;
			GRoleStorehouse storehouse;
			if( pbase->find( key, value_base, txn ) )
				value_base >> base;

			if( pstatus->find( key, value_status, txn ) )
				value_status >> status;

			if( pstorehouse->find( key, value_storehouse, txn ) )
				value_storehouse >> storehouse;

			Octets name;
			CharsetConverter::conv_charset_u2l( base.name, name );

			time_t ct = base.create_time, lt = base.lastlogin_time;
			printf( "name=%.*s,level=%d,level2=%d,sp=%d,reputation=%d,money=%lld,status=%d,\n\t\tcreatime=%s",
					name.size(),(char*)name.begin(),
					status.level, status.level2, status.sp, status.reputation,
					(int64_t)0,
					0,//(int64_t)status.money+storehouse.money,base.status,
					ct>0?ctime(&ct):"-" );
			printf( "\t\tlastlogin=%s", lt>0?ctime(&lt):"-" );
			return true;
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
		Log::log( LOG_ERR, "ShowRoleMainInfo, roleid=%d, what=%s\n", roleid, e.what() );
	}
	return false;
}

int AddNewRoleIdToUserTable( int roleid )
{
	int userid = LOGICUID(roleid);
	int newroleid = -1;

	Marshal::OctetsStream key, value_user, value_user_new;

	try
	{
		StorageEnv::Storage * puser = StorageEnv::GetStorage( "user" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			User user;
			key << userid;
			value_user = puser->find( key, txn );
			value_user >> user;
			RoleList rolelist(user.rolelist);
			newroleid = userid+rolelist.AddRole();
			user.rolelist = rolelist.GetRoleList();
			value_user_new << user;
			puser->insert( key, value_user_new, txn );
			return newroleid;
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
		Log::log( LOG_ERR, "AddNewRoleIdToUserTable, roleid=%d, what=%s\n", roleid, e.what() );
	}
	return -1;
}

void CopyRoleBase( const char * srctable, const char * desttable, int srcid, int destid )
{
	Marshal::OctetsStream keysrc, keydest, value, value_new;

	try
	{
		StorageEnv::Storage * psrc = StorageEnv::GetStorage( srctable );
		StorageEnv::Storage * pdest = StorageEnv::GetStorage( desttable );
		StorageEnv::AtomTransaction	txn;
		try
		{
			keysrc << srcid;
			keydest << destid;
			if( psrc->find( keysrc, value, txn ) )
			{
				GRoleBase	base;
				value >> base;
				base.id = destid;
				value_new << base;
				pdest->insert( keydest, value_new, txn );
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
		Log::log( LOG_ERR, "CopyRoleBase, srctable=%s, desttable=%s, srcid=%d, destid=%d, what=%s\n",
					srctable, desttable, srcid, destid, e.what() );
	}
}

void CopyRole( const char * srctable, const char * desttable, int srcid, int destid )
{
	Marshal::OctetsStream keysrc, keydest, value;

	try
	{
		StorageEnv::Storage * psrc = StorageEnv::GetStorage( srctable );
		StorageEnv::Storage * pdest = StorageEnv::GetStorage( desttable );
		StorageEnv::AtomTransaction	txn;
		try
		{
			keysrc << srcid;
			keydest << destid;
			if( psrc->find( keysrc, value, txn ) )
				pdest->insert( keydest, value, txn );
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
		Log::log( LOG_ERR, "CopyRole, srctable=%s, desttable=%s, srcid=%d, destid=%d, what=%s\n",
					srctable, desttable, srcid, destid, e.what() );
	}
}

void AddUserToFactionInfo( int roleid )
{
	Marshal::OctetsStream keyroleid, keyfid, value_userfaction, value_factioninfo,
					value_userfaction_new, value_factioninfo_new;

	try
	{
		StorageEnv::Storage * puserfaction = StorageEnv::GetStorage( "userfaction" );
		StorageEnv::Storage * pfactioninfo = StorageEnv::GetStorage( "factioninfo" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			GUserFaction	uf;
			GFactionInfo	fi;
			keyroleid << roleid;
			if( !puserfaction->find( keyroleid, value_userfaction, txn ) )
				return;
			value_userfaction >> uf;
			uf.rid = roleid;
			int fid = uf.fid;
			keyfid << fid;
			value_factioninfo = pfactioninfo->find( keyfid, txn );
			value_factioninfo >> fi;

			fi.member.add( GMember(uf.rid,uf.role) );
			value_factioninfo_new << fi;
			pfactioninfo->insert( keyfid, value_factioninfo_new, txn );

			value_userfaction_new << uf;
			puserfaction->insert( keyroleid, value_userfaction_new, txn );
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
		Log::log( LOG_ERR, "AddUserToFactionInfo, roleid=%d, what=%s\n", roleid, e.what() );
	}
}

void EraseUserFromFactionInfo( const char * userfactiontable, int roleid )
{
	Marshal::OctetsStream keyroleid, keyfid, value_userfaction, value_factioninfo, value_new;

	try
	{
		StorageEnv::Storage * puserfaction = StorageEnv::GetStorage( userfactiontable );
		StorageEnv::Storage * pfactioninfo = StorageEnv::GetStorage( "factioninfo" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			GUserFaction	uf;
			GFactionInfo	fi;
			keyroleid << roleid;
			if( !puserfaction->find( keyroleid, value_userfaction, txn ) )
				return;
			value_userfaction >> uf;
			uf.rid = roleid;
			int fid = uf.fid;
			keyfid << fid;
			value_factioninfo = pfactioninfo->find( keyfid, txn );
			value_factioninfo >> fi;
			for( GMemberVector::iterator it = fi.member.begin(); it != fi.member.end(); ++ it )
			{
				if( (*it).rid == (unsigned int)roleid )
				{
					fi.member.erase(it);
					break;
				}
			}
			value_new << fi;
			pfactioninfo->insert( keyfid, value_new, txn );
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
		Log::log( LOG_ERR, "EraseUserFromFactionInfo, userfactiontable=%s, roleid=%d, what=%s\n",
				userfactiontable, roleid, e.what() );
	}
}

void AddRoleName( Octets & addname, int roleid )
{
	Marshal::OctetsStream keyroleid, value;

	try
	{
		StorageEnv::Storage * prolename = StorageEnv::GetStorage( "rolename" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			int status = 1;
			keyroleid << roleid << status;

			int i = 1;
			while( true )
			{
				Octets testname = addname;
				Octets testname_gbk;
				CharsetConverter::conv_charset_u2l( testname, testname_gbk );

				char buffer[64];
#ifdef TAIWAN
				if( 1 == i )
					sprintf( buffer, "(B)" );
				else
					sprintf( buffer, "(B%d)", i );
#else
				sprintf( buffer, "%d", i );
#endif
				testname_gbk.insert( testname_gbk.end(), buffer, strlen(buffer) );
				testname.clear();
				CharsetConverter::conv_charset_l2u( testname_gbk, testname );
				if( !prolename->find( testname, value, txn ) )
				{
					addname = testname;
					break;
				}
				i ++;
			}

			prolename->insert( addname, keyroleid, txn );
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
		Log::log( LOG_ERR, "ModifyRoleName, roleid=%d, what=%s", roleid, e.what() );
	}
}

void ModifyRoleName( int roleid, Octets & name )
{
	Marshal::OctetsStream keyroleid, value_base, value_base_new;

	try
	{
		StorageEnv::Storage * pbase = StorageEnv::GetStorage( "base" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			GRoleBase	base;
			keyroleid << roleid;
			value_base = pbase->find( keyroleid, txn );
			value_base >> base;
			base.name = name;
			value_base_new << base;
			pbase->insert( keyroleid, value_base_new, txn );
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
		Log::log( LOG_ERR, "ModifyRoleName, roleid=%d, what=%s", roleid, e.what() );
	}
}

void InsertRoleName( const Octets & name, int roleid )
{
	Marshal::OctetsStream keyroleid;

	try
	{
		StorageEnv::Storage * prolename = StorageEnv::GetStorage( "rolename" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			keyroleid << roleid;
			prolename->insert( name, keyroleid, txn );
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
		Log::log( LOG_ERR, "InsertRoleName, roleid=%d, what=%s", roleid, e.what() );
	}
}

void InsertRoleNameByBase( int roleid )
{
	Marshal::OctetsStream keyroleid, value;

	try
	{
		StorageEnv::Storage * prolename = StorageEnv::GetStorage( "rolename" );
		StorageEnv::Storage * pbase = StorageEnv::GetStorage( "base" );
		StorageEnv::AtomTransaction	txn;
		try
		{
			keyroleid << roleid;
			value = pbase->find( keyroleid, txn );
			GRoleBase	base;
			value >> base;
			prolename->insert( base.name, keyroleid, txn );
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
		Log::log( LOG_ERR, "InsertRoleNameByBase, roleid=%d, what=%s", roleid, e.what() );
	}
}

void DealWithDuplicateID( int roleid )
{
	printf( "\ndeal with duplicate roleid:%d\n", roleid );

	GRoleStatus status1, status2;
	int no = 4;
	printf( "\trole 1:" );
	ShowRoleMainInfo( roleid, status1, "base", "status", "storehouse" );
	printf( "\n\trole 2:" );
	ShowRoleMainInfo( roleid, status2, "2_base", "2_status", "2_storehouse" );
	while( 1 != no && 2 != no && 3 != no && 4 != no )
	{
		printf( "\n\tplease tell me which to change(3 for no change,4 for auto):" );
		scanf( "%d", &no );
	}
	if( 4 == no )
	{
		if( status1.level >= status2.level )	no = 2;
		else									no = 1;
	}

	if( 1 == no )
	{
		int roleidnew = AddNewRoleIdToUserTable( roleid );
		if( -1 == roleidnew )
			printf( "\terror to add new roleid to user table.\n" );

		CopyRoleBase( "base", "base", roleid, roleidnew );
		CopyRole( "equipment", "equipment", roleid, roleidnew );
		CopyRole( "friends", "friends", roleid, roleidnew );
		CopyRole( "inventory", "inventory", roleid, roleidnew );
		CopyRole( "mailbox", "mailbox", roleid, roleidnew );
		CopyRole( "messages", "messages", roleid, roleidnew );
		CopyRole( "status", "status", roleid, roleidnew );
		CopyRole( "storehouse", "storehouse", roleid, roleidnew );
		CopyRole( "task", "task", roleid, roleidnew );
		CopyRole( "userfaction", "userfaction", roleid, roleidnew );
		EraseUserFromFactionInfo( "userfaction", roleid );
		AddUserToFactionInfo( roleidnew );
		InsertRoleNameByBase( roleidnew );

		CopyRoleBase( "2_base", "base", roleid, roleid );
		CopyRole( "2_equipment", "equipment", roleid, roleid );
		CopyRole( "2_friends", "friends", roleid, roleid );
		CopyRole( "2_inventory", "inventory", roleid, roleid );
		CopyRole( "2_mailbox", "mailbox", roleid, roleid );
		CopyRole( "2_messages", "messages", roleid, roleid );
		CopyRole( "2_status", "status", roleid, roleid );
		CopyRole( "2_storehouse", "storehouse", roleid, roleid );
		CopyRole( "2_task", "task", roleid, roleid );
		CopyRole( "2_userfaction", "userfaction", roleid, roleid );

		printf( "\tadd new roleid %d.\n", roleidnew );
	}
	else if( 2 == no )
	{
		int roleidnew = AddNewRoleIdToUserTable( roleid );
		if( -1 == roleidnew )
			printf( "\terror to add new roleid to user table.\n" );

		CopyRoleBase( "2_base", "base", roleid, roleidnew );
		CopyRole( "2_equipment", "equipment", roleid, roleidnew );
		CopyRole( "2_friends", "friends", roleid, roleidnew );
		CopyRole( "2_inventory", "inventory", roleid, roleidnew );
		CopyRole( "2_mailbox", "mailbox", roleid, roleidnew );
		CopyRole( "2_messages", "messages", roleid, roleidnew );
		CopyRole( "2_status", "status", roleid, roleidnew );
		CopyRole( "2_storehouse", "storehouse", roleid, roleidnew );
		CopyRole( "2_task", "task", roleid, roleidnew );
		CopyRole( "2_userfaction", "userfaction", roleid, roleidnew );
		EraseUserFromFactionInfo( "2_userfaction", roleid );
		AddUserToFactionInfo( roleidnew );
		InsertRoleNameByBase( roleidnew );

		printf( "\tadd new roleid %d.\n", roleidnew );
	}
}

void DealWithDuplicateName( const Octets & name, int roleid1, int roleid2 )
{
	Octets name_ucs2 = name;
	Octets name_gbk;
	CharsetConverter::conv_charset_u2l( name_ucs2, name_gbk );

	printf( "\ndeal with duplicate name:%.*s,roleid1=%d,roleid2=%d\n",
					name_gbk.size(), (char*)name_gbk.begin(), roleid1, roleid2 );

	GRoleStatus status1, status2;
#ifdef TAIWAN
	int no = 2;
#else
	int no = 4;
#endif
	printf( "\trole 1:" );
	ShowRoleMainInfo( roleid1, status1, "base", "status", "storehouse" );
	printf( "\n\trole 2:" );
	ShowRoleMainInfo( roleid2, status2, "base", "status", "storehouse" );
	while( 1 != no && 2 != no && 3 != no && 4 != no )
	{
		printf( "\n\tplease tell me which to change(3 for no change,4 for auto):" );
		scanf( "%d", &no );
	}
	if( 4 == no )
	{
		if( status1.level >= status2.level )	no = 2;
		else									no = 1;
		if( roleid1 == roleid2 )				no = 3;
	}

	if( 1 == no )
	{
		Octets addname = name;
		AddRoleName( addname, roleid1 );
		ModifyRoleName( roleid1, addname );
		InsertRoleName( name, roleid2 );

		Octets addname_gbk;
		CharsetConverter::conv_charset_u2l( addname, addname_gbk );
		printf( "\trole(id=%d)'s name is changed from %.*s to %.*s.\n",
				roleid1,name_gbk.size(),(char*)name_gbk.begin(),addname_gbk.size(),(char*)addname_gbk.begin());
	}
	else if( 2 == no )
	{
		Octets addname = name;
		AddRoleName( addname, roleid2 );
		ModifyRoleName( roleid2, addname );
		InsertRoleName( name, roleid1 );

		Octets addname_gbk;
		CharsetConverter::conv_charset_u2l( addname, addname_gbk );
		printf( "\trole(id=%d)'s name is changed from %.*s to %.*s.\n",
				roleid2,name_gbk.size(),(char*)name_gbk.begin(),addname_gbk.size(),(char*)addname_gbk.begin());
	}
}

void DealWithDuplicate( )
{
	std::set<int>::iterator iti = g_sameid.begin();
	std::map<Octets, std::pair<int,int>, lt_Octets>::iterator itn = g_samename.begin();

	for( ; iti != g_sameid.end(); ++iti )
	{
		DealWithDuplicateID( *iti );
	}

	for( ; itn != g_samename.end(); ++itn )
	{
		DealWithDuplicateName( itn->first, itn->second.first, itn->second.second );
	}
}

void MergeDBAll( const char * srcpath )
{
	printf( "NOTICE: the two databases MUST have the same compress status, please check it carefully.\n" );

	std::string src_dir = srcpath;
	std::string data_dir = StorageEnv::get_datadir();

	struct dirent **namelist;
	int n = scandir(src_dir.c_str(), &namelist, 0, alphasort);
	struct stat filestat;
	char indir[1024];

	if (n < 0)
		perror("scandir");
	else
	{
		while(n--)
		{
			sprintf(indir,"%s/%s",src_dir.c_str(),namelist[n]->d_name);
			stat(indir,&filestat);
			if(!S_ISDIR(filestat.st_mode) && 0 != strcmp(STORAGE_CONFIGDB,namelist[n]->d_name) )
			{
				std::string dbname = namelist[n]->d_name;
				system( ("/bin/cp -f " + src_dir + "/" + dbname
						+ " " + data_dir + "/2_" + dbname ).c_str() );
				if( 0 != strcmp("clsconfig",dbname.c_str())
					&& 0 != strcmp("auction",dbname.c_str())
					&& 0 != strcmp("sellpoint",dbname.c_str())
					&& 0 != strcmp("translog",dbname.c_str())
					&& 0 != strcmp("order",dbname.c_str())
					&& 0 != strcmp("city",dbname.c_str())
					&& 0 != strcmp("gtask",dbname.c_str())
				)
					MergeDB( src_dir.c_str(), namelist[n]->d_name, namelist[n]->d_name );
			}

			free(namelist[n]);
		}
		free(namelist);
	}

	MergeAuction( src_dir.c_str() );

	DealWithDuplicate( );

	StorageEnv::checkpoint();
	StorageEnv::Close();

	system( ("/bin/rm -f " + data_dir + "/2_*").c_str() );
	system( ("/bin/rm -f " + data_dir + "/order").c_str() );
	system( ("/bin/rm -f " + data_dir + "/city").c_str() );
	system( ("/bin/rm -f " + data_dir + "/gtask").c_str() );

	StorageEnv::Open();
	StorageEnv::checkpoint( );
	StorageEnv::removeoldlogs( );

	printf( "\nMERGE REPORT: \n\tserver2_usercount = %d\n\tsame_usercount = %d\n\tserver1_samerolecount = %d\n\tserver2_rolecount = %d\n", server2_usercount, same_usercount, server1_samerolecount, server2_rolecount );
}

};

