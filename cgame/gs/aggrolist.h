#ifndef __ONLINEGAME_GS_AGGRO_LIST_H__
#define __ONLINEGAME_GS_AGGRO_LIST_H__

#include <algorithm>
#include <vector.h>

#include <common/types.h>
#include <amemory.h>

class aggro_list
{
	struct aggro_node
	{
		XID id;
		int rage;
		aggro_node():rage(0) {} 
		aggro_node(const XID & __id,int __rage = 0):id(__id),rage(__rage) {}

		bool operator==(const XID &rhs) const 
		{
			return id == rhs;
		}
	};
	typedef abase::vector<aggro_node,abase::fast_alloc<> > AGGROLIST;
	AGGROLIST _alist;
	size_t    _max_size;


	static bool __NodeCompare(const aggro_node &lhs,const aggro_node &rhs)
	{
		return lhs.rage < rhs.rage;
	}
	static bool __RageCompare(const aggro_node & lhs, const int rage)
	{
		return lhs.rage >= rage;
	}


	inline AGGROLIST::iterator __Find(const XID & rhs) const
	{
		return (AGGROLIST::iterator) std::find(_alist.begin(),_alist.end(), rhs);
	}
	inline AGGROLIST::iterator __Find(int rage) const
	{
		return (AGGROLIST::iterator)std::lower_bound(_alist.begin(),_alist.end(),rage,__RageCompare);
	}
public:
	aggro_list(int maxsize):_max_size(maxsize)
	{
		_alist.reserve(maxsize);
	}

	int 	AddRage(const XID & __id, int __rage);
	int	AddRage(const XID & __id, int __rage, int max_rage);
	int 	AddToFrist(const XID & __id,int __addon_rage);
	int 	Remove(const XID & __id);
    int     RemoveRage(const XID& __src, const XID& __dest, float __ratio);
	void 	RegroupAggro();
	void 	SwapAggro(size_t index1,size_t index2);
	void 	AddToLast(const XID & target);
	void 	Fade();
	void 	RemoveFirst()
	{
		if(_alist.empty()) return;
		_alist.erase(_alist.begin());
	}
	
	void 	Clear() {_alist.clear();} 
	size_t 	Size() const { return _alist.size(); }
	int 	GetRage(size_t index) const { return _alist[index].rage;}
	const 	XID & GetID(size_t index) const { return _alist[index].id;}
	bool	GetFirst(XID & target) const
	{
		if(_alist.empty()) return false;
		target = _alist[0].id;
		return true;
	}
	int 	GetEntry(size_t index, XID & target) const
	{
		if(index >= _alist.size()) return 0;
		target = _alist[index].id;
		return _alist[index].rage;
	}

	bool	IsEmpty() { return _alist.empty();}
	bool 	IsFirst(const XID & target) { return _alist.size() && _alist[0].id == target;}
	void 	GetAll(abase::vector<XID> &list)
	{
		size_t i;
		for(i = 0;i < _alist.size(); i ++)
		{
			if(_alist[i].rage <=0) break;
			list.push_back(_alist[i].id);
		}
	}


	void 	OutputList(FILE * file)
	{
		size_t i;
		for(i = 0;i < _alist.size(); i ++)
		{
			if(_alist[i].rage <=0) break;
			fprintf(file,"(%d,%d,%d)\t",_alist[i].id.type,_alist[i].id.id,_alist[i].rage);
		}
		fprintf(file,"\n");
	}
	
};
#endif
