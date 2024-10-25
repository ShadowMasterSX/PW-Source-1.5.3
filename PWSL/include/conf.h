/*
	配置文件读取器， 由于使用了STL，可能具有一定的不兼容性（编译器相关）
	作者：忘了
	修改：崔铭 ， 在原有的基础上增加了组的概念，避免不同配置文件的冲突 2009-06-16
	公司：完美时空
*/

#ifndef __GNET_CONF_H__
#define __GNET_CONF_H__

#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <string>
#include <strings.h>
#include <fstream>
#include <vector>

#include "rwlock.h"

namespace GNET
{

using std::string;
class Conf
{
public:
	typedef string section_type;
	typedef string key_type;
	typedef string value_type;
private:
	time_t mtime;
	struct stringcasecmp
	{
		bool operator() (const string &x, const string &y) const { return strcasecmp(x.c_str(), y.c_str()) < 0; }
	};
	static Conf instance;
	static abase::RWLock locker;
	
	typedef std::map<key_type, value_type, stringcasecmp> section_hash;
	typedef std::map<section_type, section_hash, stringcasecmp> conf_hash;
	conf_hash confhash;
	string filename;
	void reload();

	Conf() : mtime(0) { }

	void Merge(Conf & rhs);
public:

	explicit Conf(const char *file):mtime(0)
	{
		if (file && access(file, R_OK) == 0)
		{
			filename = file;
			reload();
		}
	}

	value_type find(const section_type &section, const key_type &key)
	{
		abase::RWLock::Keeper keeper(locker);
		keeper.LockRead();
		return confhash[section][key];
	}
	value_type put(const section_type &section, const key_type &key, const value_type &value)
	{
		abase::RWLock::Keeper keeper(locker);
		keeper.LockWrite();
		value_type oldvalue = confhash[section][key];
		confhash[section][key] = value;
		return oldvalue;
	}
	void getkeys(const section_type &section, std::vector<key_type> &keys)
	{
		keys.clear();
		abase::RWLock::Keeper keeper(locker);
		keeper.LockRead();
		section_hash h = confhash[section];
		for( section_hash::const_iterator it=h.begin(); it!=h.end(); ++it )
		{
			keys.push_back( (*it).first );
		}
	}
	static Conf *GetInstance(const char *file = NULL, const char * conf_group = NULL);
	void dump(FILE * out);
	static void AppendConfFile( const char * file, const char * conf_group = NULL)
	{
		Conf * conf = GetInstance(NULL, conf_group);
		Conf tmp(file);
		conf->Merge(tmp);
	}
};	

}

#endif

