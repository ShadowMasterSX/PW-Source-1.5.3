
#ifndef __ONLINEGAME_GS_PLAYER_LIMIT_H__
#define __ONLINEGAME_GS_PLAYER_LIMIT_H__

#include <vector.h>

enum
{
	PLAYER_LIMIT_NOFLY,
	PLAYER_LIMIT_NOCHANGESELECT,
	PLAYER_LIMIT_NOMOUNT,
	PLAYER_LIMIT_NOBIND,
	PLAYER_LIMIT_NOAMULET,//禁止护身符自动使用

	PLAYER_LIMIT_MAX,
};

class player_limit
{
	public:
		player_limit():_list(PLAYER_LIMIT_MAX, 0){}
		
		bool SetLimit(int index, bool b)
		{
			int & counter = _list[index];
			if(b)
			{
				++ counter;
				return counter == 1;
			}
			else
			{
				-- counter;
				return counter == 0;
			}
		}
		
		bool GetLimit(int index)
		{
			return _list[index];
		}

		bool Save(archive & ar)
		{
			ar.push_back(_list.begin(), _list.size()*sizeof(int));	
			return true;
		}
		bool Load(archive & ar)
		{
			ar.pop_back(_list.begin(), _list.size()*sizeof(int));
			return true;
		}
		void Swap(player_limit & rhs)
		{
			_list.swap(rhs._list);	
		}
	private:
		abase::vector<int> _list;
};

#endif
