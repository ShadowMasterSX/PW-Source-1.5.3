/*
 *		作者:zengpan
 * 		无修改(移动一部分代码到cpp)
 */

#ifndef __CONF_H
#define __CONF_H

#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <string>
#include <fstream>
#include <stdio.h>

#include "threadpool.h"

namespace ONET
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
	typedef std::map<key_type, value_type, stringcasecmp> section_hash;
	typedef std::map<section_type, section_hash, stringcasecmp> conf_hash;
	conf_hash confhash;
	string filename;
	static Thread::RWLock locker;
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

	void dump(FILE * out);
	
	value_type find(const section_type &section, const key_type &key)
	{
		Thread::RWLock::RDScoped l(locker);
		return confhash[section][key];
	}
	static Conf *GetInstance(const char *file = NULL)
	{
		if (file && access(file, R_OK) == 0)
		{
			instance.filename = file;
			instance.reload();
		}
		return &instance; 
	}

	static void AppendConfFile(const char * file)
	{
		Conf * conf = GetInstance();
		Conf tmp(file);
		conf->Merge(tmp);
	}
};	

};

#endif

