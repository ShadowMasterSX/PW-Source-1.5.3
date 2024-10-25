
#ifndef __ONLINEGAME_GS_PLAYERFORCE_H__
#define __ONLINEGAME_GS_PLAYERFORCE_H__

#include <common/base_wrapper.h>
#include <amemory.h>
#include "staticmap.h"
#include <db_if.h>

//Íæ¼ÒÊÆÁ¦

class gplayer_imp;
class player_force
{
public:
	struct force_data
	{
		int reputation;
		int contribution;
		force_data(int repu=0,int contri=0):reputation(repu),contribution(contri){}
	};
	typedef abase::static_multimap<int, force_data, abase::fast_alloc<> > DATA_MAP;


public:
	player_force(gplayer_imp * imp):_imp(imp),_cur_force_id(0){}

	bool InitFromDBData(const GDB::forcedata_list & list);
	bool MakeDBData(GDB::forcedata_list &list);
	static void ReleaseDBData(GDB::forcedata_list & list);

	bool Save(archive & ar)
	{
		ar << _cur_force_id << _data_map.size();
		if(_data_map.size()) ar.push_back(_data_map.begin(), _data_map.size()*sizeof(int)*3);
		return true;
	}
	bool Load(archive & ar)
	{
		size_t size = 0;
		ar >> _cur_force_id >> size;
		_data_map.reserve(size);
		_data_map.clear();
		for(size_t i=0; i<size; i++)
		{
			int id,repu,contri;
			ar >> id >> repu >> contri;
			_data_map[id] = force_data(repu,contri);
		}
		return true;
	}
	void Swap(player_force & rhs)
	{
		abase::swap(_cur_force_id, rhs._cur_force_id);
		_data_map.swap(rhs._data_map);
	}

	void NotifyClient();

	int GetForce(){ return _cur_force_id; }
	void ChangeForce(int force);

	int GetReputation();
	int GetContribution();
	void IncReputation(int repu);
	void DecReputation(int repu);
	void IncContribution(int contri);
	void DecContribution(int contri);

private:
	gplayer_imp * _imp;
	int _cur_force_id;
	DATA_MAP _data_map;
};

#endif
