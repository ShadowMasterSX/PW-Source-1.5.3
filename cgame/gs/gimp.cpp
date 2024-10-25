#include <stdlib.h>
#include "gimp.h"
#include "object.h"
#include "world.h"
#include "usermsg.h"
#include <threadpool.h>
#include <glog.h>


DEFINE_SUBSTANCE_ABSTRACT(gobject_imp,substance,-1)
DEFINE_SUBSTANCE_ABSTRACT(dispatcher,substance,-1)
DEFINE_SUBSTANCE_ABSTRACT(controller,substance,-1)

namespace
{
	class runner_enter
	{
		dispatcher * _runner;
		const A3DVECTOR & _pos;
	public:
		runner_enter(dispatcher * runner,const A3DVECTOR &pos):_runner(runner),_pos(pos){}
		void operator()(slice * pPiece)
		{
			_runner->enter_slice(pPiece,_pos);
		}
	};
	class runner_leave
	{
		dispatcher * _runner;
		const A3DVECTOR & _pos;
	public:
		runner_leave(dispatcher * runner,const A3DVECTOR &pos):_runner(runner),_pos(pos){}
		void operator()(slice * pPiece)
		{
			_runner->leave_slice(pPiece,_pos);
		}
	};
}

void
dispatcher::MoveBetweenSlice(slice * pPiece,slice * pNewPiece,const A3DVECTOR &pos)
{
	_imp->_plane->MoveBetweenSlice(pPiece,pNewPiece,
			runner_enter(this,pos),
			runner_leave(this,pos));
}

bool
gobject_imp::StepMove(const A3DVECTOR &offset)
{
	//在调用这个地方之前必须要上锁
	ASSERT(_parent);
	gobject * pObject = _parent;
	slice *pPiece = pObject->pPiece;
	world *pPlane = _plane;
	if(pPiece == NULL)
	{
		//正巧不在格子内，这是有可能的 但是可能性偏低
		//需要做日志
		GLog::log(GLOG_ERR,"用户%d移动错误",pObject->ID.id);
		ASSERT(false);
		return false;
	}
	
	A3DVECTOR newpos(pObject->pos),oldpos(pObject->pos);
	newpos += offset;
	const grid * pGrid = &pPlane->GetGrid();
	dispatcher *pRunner = _runner;
	//检查是否超出了 整个服务器的范围
	if(pGrid->IsOutsideGrid(newpos.x,newpos.z))
	{
		//用于特殊情况的边界检查
		return false;
	}

//	if(pPiece->IsEdge())
	if(!pGrid->IsLocal(newpos.x,newpos.z))
	{
		//暂时的服务器转移计算
		int dest= pPlane->GetSvrNear(newpos);
		if( dest < 0) return false;		//找不到正确的服务器，所以不理睬这个移动命令
		_commander->SwitchSvr(dest,oldpos,newpos,0);	//准备转移(player)或者直接转移(NPC,物品)
		return true;
	}

	ASSERT(pGrid->IsLocal(newpos.x,newpos.z));

	pObject->pos = newpos;
	pRunner->begin_transfer();
	if(pPiece->IsOutside(newpos.x,newpos.z))
	{
		slice *pNewPiece = pGrid->Locate(newpos.x,newpos.z);
		if(pNewPiece == pPiece) {
			pRunner->end_transfer();
			return true;	//这种情况是可能的，由于计算的误差而产生判断的不一致
		}
		if(_commander->MoveBetweenSlice(_parent,pPiece,pNewPiece))
		{
			//移动失败，直接返回
			GLog::log(GLOG_ERR,"用户%d在MoveBetweenSlice时失败",pObject->ID.id);

			pObject->pos = oldpos;
			pRunner->end_transfer();
			return true;
		}
		if(pPiece->IsBorder() || pNewPiece->IsBorder())
		{
			//边界处，将用户移动的命令转发到其他服务器上。
			pRunner->notify_move(oldpos,newpos);
		}

		//处理格间的移动发送的消息
		pRunner->MoveBetweenSlice(pPiece,pNewPiece,newpos);
	}
//	pRunner->notify_pos();	//通知player自己的当前位置(NPC这个函数无实现)，现在不做，因为不需要如此频繁
	pRunner->end_transfer();
	return true;
}

void
dispatcher::update_visible_state(unsigned int newstate, unsigned int newstate2, unsigned int newstate3, unsigned int newstate4, unsigned int newstate5, unsigned int newstate6)
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gobject * pObject = _imp->_parent;
	CMD::Make<CMD::update_visible_state>::From(h1,pObject,newstate,newstate2,newstate3,newstate4,newstate5,newstate6);
	AutoBroadcastCSMsg(_imp->_plane,pObject->pPiece,h1,-1);
}

void 
dispatcher::enchant_result(const XID & caster, int skill, char level,bool invader,int at_state,unsigned char section)
{
	packet_wrapper  h1(64);
	using namespace S2C;
	gobject * pObject = _imp->_parent;
	CMD::Make<CMD::enchant_result>::From(h1,pObject, caster, skill, level, invader,at_state,section);
	AutoBroadcastCSMsg(_imp->_plane,pObject->pPiece,h1,-1);
}

void
dispatcher::toggle_invisible(int invisible_degree)
{
	packet_wrapper  h1(16);
	using namespace S2C;
	gobject * pObject = _imp->_parent;
	CMD::Make<CMD::object_invisible>::From(h1,pObject,invisible_degree);
	AutoBroadcastCSMsg(_imp->_plane,pObject->pPiece,h1);
}
