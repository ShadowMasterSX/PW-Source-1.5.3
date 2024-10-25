#include "object.h"
#include "player.h"


#ifndef __ONLINEGAME_GS_SLICE_H__
#define __ONLINEGAME_GS_SLICE_H__

struct slice
{
	enum { 
		EDGE 		= 1,
		BORDER		= 2,
		OUTSIDE 	= 4,
		INSIDE 		= 8,
		SENSITIVE	= 16,
		};
	int spinlock;
	int flag;
	gobject * player_list;
	gobject * npc_list;
	gobject * matter_list;
	struct rect slice_range;
	int idle_timestamp;	//发出leave idle 消息的时间戳
	bool region_overlap; // 是否跟安全区有交叠
public:
	inline void Lock() { mutex_spinlock(&spinlock);}
	inline void Unlock() { mutex_spinunlock(&spinlock);}
	inline void InsertPlayer(gobject *pPlayer) { insert(player_list,pPlayer); }
	inline void InsertNPC(gobject *pNPC) { insert(npc_list,pNPC); }
	inline void InsertMatter(gmatter *pMatter) { insert(matter_list,pMatter); }
	inline void RemovePlayer(gplayer *pPlayer) { remove(player_list,pPlayer); }
	inline void RemoveNPC(gobject *pNPC) { remove(npc_list,pNPC); }
	inline void RemoveMatter(gobject *pMatter) { remove(matter_list,pMatter); }

	inline int MovePlayer(gplayer *pPlayer,slice * dest)
	{
		spin_doublelock dlock(&this->spinlock, &dest->spinlock);
		if(pPlayer->pPiece != this) 
		{
			return -1;
		}
		RemovePlayer(pPlayer);
		dest->InsertPlayer(pPlayer);
		return 0;
	}
	inline int MoveNPC(gnpc *pNPC, slice *dest)
	{
		spin_doublelock dlock(&this->spinlock, &dest->spinlock);
		if(pNPC->pPiece != this) 
		{
			return -1;
		}
		RemoveNPC(pNPC);
		dest->InsertNPC(pNPC);
		return 0;
	}
	
	inline gplayer* GetPlayer() { return (gplayer*)player_list;}
	inline gmatter* GetMatter() { return (gmatter*)matter_list;}
	inline gnpc* GetNPC() { return (gnpc*)npc_list;}
	inline bool IsEdge() { return flag & EDGE;}
	inline bool IsBorder() { return flag & BORDER;}
	inline bool IsInWorld() { return flag & INSIDE;}
	inline bool IsOutside(float x,float y) {return slice_range.IsOut(x,y);}
	inline bool IsInside(float x,float y) {return slice_range.IsIn(x,y);}
	inline float Distance(slice *pPiece)
	{
		float dis_x= fabs(pPiece->slice_range.left - slice_range.left);
		float dis_y= fabs(pPiece->slice_range.top - slice_range.top);
		return dis_x>dis_y?dis_x:dis_y;
	}
private:
	static inline void lock_twice(slice * src,slice * dest)
	{
		if(src < dest)
		{
			src->Lock();
			dest->Lock();
		}
		else
		{
			dest->Lock();
			src->Lock();
		}
	}
	inline void insert(gobject *& list, gobject * pObj)
	{
		pObj->pPiece = this;
		pObj->pPrev = NULL;
		pObj->pNext = list;
		if(list)
		{
			list->pPrev = pObj;
		}
		list = pObj;
	}
	inline void remove(gobject *& list, gobject * pObj)
	{
		gobject * pNext = pObj->pNext;
		if(pNext) { 
			pNext->pPrev = pObj->pPrev; 
		}
		if(pObj->pPrev){
			pObj->pPrev->pNext = pNext;
		}
		else
		{
			list = pNext;
		}
		pObj->pPiece = NULL;
		pObj->pPrev = NULL;
		pObj->pNext = NULL;
	}
};
#endif
