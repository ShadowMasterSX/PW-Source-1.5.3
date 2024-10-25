#ifndef __ONLINE_GAME_GS_PATROL_AGENT_IMP_H__
#define __ONLINE_GAME_GS_PATROL_AGENT_IMP_H__

#include "config.h"
#include "aipolicy.h"
#include "worldmanager.h"

class base_patrol_agent : public patrol_agent
{
protected:
	int _index;
	bool _end_flag;
	bool _end_flag_triggered;
	bool _forward;
	int _path_type;	//0 不循环 1 原路返回 2循环
	path_manager::single_path * _path;
public:
	DECLARE_SUBSTANCE(base_patrol_agent);
	base_patrol_agent():_index(0),_end_flag(false),_forward(true),_path_type(2),_path(NULL){}
	virtual bool Init(int path_id, int path_type)
	{
		_path = world_manager::GetPathMan().GetPath(path_id); 
		if(!_path || _path->GetWayPointCount() < 2) return false;
		_index = 0;
		_end_flag = false;
		_end_flag_triggered = false;
		_forward = true;
		_path_type = path_type;
		return true;
	}
	virtual bool Reset()
	{
		_index = 0;
		_end_flag = false;
		_end_flag_triggered = false;
		_forward = true;
		return true;
	}

	virtual int GetPathID()
	{
		return _path->id;
	}

	virtual bool GetFirstWayPoint(A3DVECTOR & pos) 
	{
		_path->GetFirstWayPoint(pos);
		return true;
	}

	/*
	 参数: first_end = true 取出第一次终止状态
	 				 = false 不取出
	 返回值:false 已终止 first_end表示是否第一次终止
	 		true 未终止 pos表示下一个位置
	 */
	virtual bool GetNextWayPoint(A3DVECTOR & pos, bool & first_end)
	{
		if(_end_flag)
		{
			if(first_end)
			{
				first_end = _end_flag_triggered;
				if(_end_flag_triggered) _end_flag_triggered = false;
			}
			return false;
		}
		_path->GetWayPoint(_index,pos);
		
		switch(_path_type)
		{
			case 0:
				if(_index < (int)_path->GetWayPointCount()-1)
					_index ++;
				else
				{
					_end_flag = true;
					_end_flag_triggered = true;
				}
				break;

			case 1:
				if(_forward)
				{
					if(_index < (int)_path->GetWayPointCount()-1)
						_index ++;
					else
					{
						_index --;
						_forward = false;
					}
				}
				else
				{
					if(_index > 0)
						_index --;
					else
					{
						_index ++;
						_forward = true;
					}
				}
				break;

			default:
			case 2:
				if(_index < (int)_path->GetWayPointCount()-1)
					_index ++;
				else
					_index = 0;
				break;
		}
		return true;
	}

	virtual bool GetCurWayPoint(A3DVECTOR & pos)
	{
		_path->GetWayPoint(_index,pos);
		return true;
	}

	virtual bool Save(archive & ar)
	{
		ar << _index << _end_flag << _forward << _path_type << _path->id;
		return true;
	}

	virtual bool Load(archive & ar)
	{
		int path_id;
		ar >> _index >> _end_flag >> _forward >> _path_type >> path_id;
		_path = world_manager::GetPathMan().GetPath(path_id); 
		return _path;
	}
};

#endif
