
#ifndef __ONLINEGAME_GS_FORCEGLOBALDATAMAN_H__
#define __ONLINEGAME_GS_FORCEGLOBALDATAMAN_H__

#include "staticmap.h"
#include "spinlock.h"

class ForceGlobalDataMan
{
public:
	struct ForceGlobalData
	{
		int player_count; 
		int development; 
		int construction; 
		int activity; 
		int activity_level; 
	};
	typedef abase::static_multimap<int, ForceGlobalData, abase::fast_alloc<> > DATA_MAP;

public:
	ForceGlobalDataMan() : _lock(0),_data_ready(false) {}
	bool IsDataReady(){ return _data_ready; }

	void SetData(int force_id, int player_count, int development, int construction, int activity, int activity_level)
	{
		spin_autolock keeper(_lock);
		if(!_data_ready) _data_ready = true;
		if(force_id <= 0) return;
		ForceGlobalData & data = _data_map[force_id];
		data.player_count = player_count; 
		data.development = development; 
		data.construction = construction; 
		data.activity = activity; 
		data.activity_level = activity_level; 
	}

	bool GetData(size_t& count, const void ** data, size_t& data_size)
	{
		spin_autolock keeper(_lock);
		if(!_data_ready) 
		{
			count = 0;
			*data = NULL;
			data_size = 0;
			return false;
		}
		count = _data_map.size();
		*data = _data_map.begin();
		data_size = _data_map.size() * sizeof(DATA_MAP::value_type);
		return true;	
	}
	
	int GetActivityLevel(int force_id)
	{
		spin_autolock keeper(_lock);
		if(!_data_ready) return 0;
		DATA_MAP::iterator it = _data_map.find(force_id);
		if(it == _data_map.end()) return 0;
		return it->second.activity_level;
	}
	
	class stream
	{
	public:
		virtual ~stream(){}
		virtual void dump(const char * str)  {}
	};
	void Dump(stream * s)
	{
		spin_autolock keeper(_lock);
		char buf[32];
		snprintf(buf,32,"data_ready:%d\n",_data_ready);				s->dump(buf);
		for(DATA_MAP::iterator it=_data_map.begin(); it!=_data_map.end(); ++it)
		{
			ForceGlobalData & data = it->second;
			snprintf(buf,32,"force_id:%d\n",it->first);				s->dump(buf);
			snprintf(buf,32,"player_count:%d\n",data.player_count);	s->dump(buf);
			snprintf(buf,32,"development:%d\n",data.development);	s->dump(buf);
			snprintf(buf,32,"construction:%d\n",data.construction);	s->dump(buf);
			snprintf(buf,32,"activity:%d\n",data.activity);			s->dump(buf);
			snprintf(buf,32,"activity_level:%d\n",data.activity_level);	s->dump(buf);
		}
	}

private:
	int _lock;
	bool _data_ready;
	DATA_MAP _data_map;
};


#endif
