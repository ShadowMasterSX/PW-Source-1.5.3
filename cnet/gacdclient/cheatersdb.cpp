
#include "cheatersdb.h"

namespace GNET
{

	/*
	int area_id;
    int cheat_id;
    time_t time_begin;
	*/

	std::string CheatersDB::trim(std::string const& source, char const* delims) const {
  std::string result(source);
  std::string::size_type index = result.find_last_not_of(delims);
  if(index != std::string::npos)
    result.erase(++index);

  index = result.find_first_not_of(delims);
  if(index != std::string::npos)
    result.erase(0, index);
  else
    result.erase();
  return result;
}

	std::string CheatersDB::GetFileName() const 
	{
		struct tm t_tm = *(localtime(&time_begin));
		char fn_buffer[30];
		sprintf(fn_buffer, "cheatdb/%d/%d/%04d_%02d_%02d.ch", area_id, cheat_id,
			t_tm.tm_year + 1900, t_tm.tm_mon + 1, t_tm.tm_mday);
		printf("filename %s\n", fn_buffer);
		return fn_buffer;
	}
	XmlObject CheatersDB::WriteObject() const 
	{
		XmlObject obj("cheats");
		for(std::vector<Item>::const_iterator it = cheaters.begin();
				it != cheaters.end(); ++it)
		{
			obj.AddChild(XmlObject("item").SetAttr("userid", (*it).uid).SetAttr("subid",
				(*it).subid).SetAttr("time", (*it).ctime).SetAttr("strtime",trim(std::string(ctime(&((*it).ctime))))));
		}
		return obj;
	}
	void CheatersDB::LoadObject(const XmlObject &_obj)	
	{
		cheaters.clear();
        for(XmlObject::Children::const_iterator it = _obj.children.begin();
            it != _obj.children.end(); ++it)
        {
			cheaters.push_back(Item((*it).GetAttr("userid", int()), (*it).GetAttr("subid", int()),
				(*it).GetAttr("time", time_t())));
        }
	}
	/*
	std::vector<int> v;
    std::string GetFileName() const { return "data/vec.xml"; }
    XmlObject WriteObject() const {
        XmlObject obj("vector");
        for(size_t i=0;i<v.size();++i)
            obj.AddChild(XmlObject("element").SetAttr("value",v[i]));
        return obj;
    }
    void LoadObject(const XmlObject &_obj) {
        v.clear();
        for(XmlObject::Children::const_iterator it = _obj.children.begin();
            it != _obj.children.end(); ++it)
        {
            v.push_back( (*it).GetAttr("value", int()));
        }
    }
	*/

	void CheatersDB::SaveCheaters(const std::map<int, std::pair<std::string, std::vector<std::pair<int, std::pair<int, time_t> > > > > &c, int aid)
	{
		for(std::map<int, std::pair<std::string, std::vector<std::pair<int, std::pair<int, time_t> > > > >::
			const_iterator it = c.begin(); it != c.end(); ++it)
		{
			int c_id = (*it).first;
			//if( c_id == 201 || c_id == 203 || c_id == 101 ) continue;//TODO temp
			const std::vector<std::pair<int, std::pair<int, time_t> > > &ch = (*it).second.second;
			if( ! ch.empty() )
			{
				time_t t_begin = ch[0].second.second;
				CheatersDB tmp;
				tmp.area_id = aid;
				tmp.cheat_id = c_id;
				tmp.time_begin = t_begin;
				tmp.Load();
				for(std::vector<std::pair<int, std::pair<int, time_t> > >::const_iterator it2 = ch.begin();
					it2 != ch.end(); ++it2)
				{
					tmp.AddItem(Item((*it2).first, (*it2).second.first, (*it2).second.second));
				}
				tmp.Save();
			}
		}
	}
};

