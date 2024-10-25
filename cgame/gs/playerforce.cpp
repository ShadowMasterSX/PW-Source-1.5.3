#include "string.h"

#include "playerforce.h"
#include "world.h"
#include "player_imp.h"

bool player_force::InitFromDBData(const GDB::forcedata_list & list)
{
	_cur_force_id = list.cur_force_id;
	const GDB::forcedata * pData = list.list;
	for(size_t i=0; i<list.count; i++)
	{
		int force_id = pData[i].force_id;
		if(force_id <= 0 || _data_map.find(force_id) != _data_map.end())
		{
			__PRINTF("错误/重复的势力id=%d\n",force_id);
			continue;
		}
		_data_map[force_id] = force_data(pData[i].reputation, pData[i].contribution);
	}
	//
	gplayer * pPlayer = (gplayer *)_imp->_parent;
	if(_cur_force_id > 0)
	{
		pPlayer->object_state |= gactive_object::STATE_PLAYERFORCE;
		pPlayer->force_id = _cur_force_id;
	}
	return true;
}
bool player_force::MakeDBData(GDB::forcedata_list &list)
{
	list.cur_force_id = _cur_force_id;
	list.count = _data_map.size();
	list.list = NULL;
	if(list.count == 0) return true;
	list.list = (GDB::forcedata*)abase::fast_allocator::alloc(list.count*sizeof(GDB::forcedata));
	GDB::forcedata * pData = list.list;
	size_t i=0;
	for(DATA_MAP::iterator it=_data_map.begin(); it!=_data_map.end(); ++it)
	{
		pData[i].force_id = it->first;
		pData[i].reputation = it->second.reputation;
		pData[i].contribution = it->second.contribution;
		++ i;
	}
	return true;
}
void player_force::ReleaseDBData(GDB::forcedata_list & list)
{
	if(list.list && list.count)
	{
		abase::fast_allocator::free(list.list, list.count*sizeof(GDB::forcedata));
		list.count = 0;
		list.list = NULL;
	}	
}


void player_force::NotifyClient()
{
	_imp->_runner->send_player_force_data(_cur_force_id,_data_map.size(),_data_map.begin(),_data_map.size()*sizeof(DATA_MAP::value_type));
}

void player_force::ChangeForce(int force)
{ 
	_cur_force_id = force; 
	gplayer * pPlayer = (gplayer *)_imp->_parent;
	if(force) 
		pPlayer->object_state |= gactive_object::STATE_PLAYERFORCE;
	else
		pPlayer->object_state &= ~gactive_object::STATE_PLAYERFORCE;
	pPlayer->force_id = force;
	_imp->_runner->player_force_changed(force);
}

int player_force::GetReputation()
{ 
	if(_cur_force_id == 0) return 0;
	DATA_MAP::iterator it = _data_map.find(_cur_force_id);
	if(it == _data_map.end()) return 0;
	return it->second.reputation;
}
int player_force::GetContribution()
{
	if(_cur_force_id == 0) return 0;
	DATA_MAP::iterator it = _data_map.find(_cur_force_id);
	if(it == _data_map.end()) return 0;
	return it->second.contribution;
}
void player_force::IncReputation(int repu)
{
	ASSERT(_cur_force_id);
	force_data & data = _data_map[_cur_force_id];
	data.reputation += repu;
	_imp->_runner->player_force_data_update(_cur_force_id,data.reputation,data.contribution);
}
void player_force::DecReputation(int repu)
{
	ASSERT(_cur_force_id);
	force_data & data = _data_map[_cur_force_id];
	ASSERT(data.reputation >= repu);
	data.reputation -= repu;
	_imp->_runner->player_force_data_update(_cur_force_id,data.reputation,data.contribution);
}
void player_force::IncContribution(int contri)
{
	ASSERT(_cur_force_id);
	force_data & data = _data_map[_cur_force_id];
	data.contribution += contri;
	_imp->_runner->player_force_data_update(_cur_force_id,data.reputation,data.contribution);
}
void player_force::DecContribution(int contri)
{
	ASSERT(_cur_force_id);
	force_data & data = _data_map[_cur_force_id];
	ASSERT(data.contribution >= contri);
	data.contribution -= contri;
	_imp->_runner->player_force_data_update(_cur_force_id,data.reputation,data.contribution);
}
