#include <strings.h>
#include "conf.h"

namespace ONET
{

Conf Conf::instance;
Thread::RWLock Conf::locker;
void Conf::reload()
{
	struct stat st;
	Thread::RWLock::WRScoped l(locker);
	for ( stat(filename.c_str(), &st); mtime != st.st_mtime; stat(filename.c_str(), &st) )
	{
		mtime = st.st_mtime;
		std::ifstream ifs(filename.c_str());
		string line;
		section_type section;
		section_hash sechash;
		if (!confhash.empty()) confhash.clear();
		while (std::getline(ifs, line))
		{
			const char c = line[0];
			if (c == '#' || c == ';') continue;
			if (c == '[')
			{
				string::size_type start = line.find_first_not_of(" \t", 1);
				if (start == string::npos) continue;
				string::size_type end   = line.find_first_of(" \t]", start);
				if (end   == string::npos) continue;
				if (!section.empty()) confhash[section] = sechash;
				section = section_type(line, start, end - start);
				sechash.clear();
			} else {
				string::size_type key_start = line.find_first_not_of(" \t");
				if (key_start == string::npos) continue;
				string::size_type key_end   = line.find_first_of(" \t=", key_start);
				if (key_end == string::npos) continue;
				string::size_type val_start = line.find_first_of("=", key_end);
				if (val_start == string::npos) continue;
				val_start = line.find_first_not_of(" \t", val_start + 1);
				string::size_type val_end = line.find_last_not_of(" \t\r\n");
				if (val_start == string::npos) continue;
				if (val_end == string::npos) continue;
				sechash[key_type(line, key_start, key_end - key_start)] = value_type(line, val_start,val_end - val_start + 1);
			}
		}
		if (!section.empty()) confhash[section] = sechash;
	}
}

void 
Conf::Merge(Conf & rhs)
{
	Thread::RWLock::RDScoped l(locker);
	conf_hash::iterator it_c = rhs.confhash.begin();
	for(;it_c != rhs.confhash.end(); ++it_c)
	{
		//察看是否有一样的section
		conf_hash::iterator it_e = confhash.find(it_c->first);
		if(it_e == confhash.end())
		{
			//没有一样的，直接复制
			confhash[it_c->first] = it_c->second;
		}
		else
		{
			section_hash & shash = it_c->second;
			section_hash & sechash = it_e->second;
			section_hash::iterator it_s = shash.begin();
			for(;it_s != shash.end(); ++it_s)
			{
				sechash[it_s->first] = it_s->second;
			}
		}
	}
}

void 
Conf::dump(FILE * out)
{
	Thread::RWLock::RDScoped l(locker);
	conf_hash::iterator it_c = confhash.begin();
	for(;it_c != confhash.end(); ++it_c)
	{
		fprintf(out,"[%s]\n",it_c->first.c_str());
		section_hash & shash = it_c->second;
		section_hash::iterator it_s = shash.begin();
		for(;it_s != shash.end(); ++it_s)
		{
			fprintf(out,"%s\t=\t%s\n",it_s->first.c_str(), it_s->second.c_str());
		}
		fprintf(out,"\n");
	}
	
}
};

