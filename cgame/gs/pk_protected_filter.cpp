#include "string.h"
#include "world.h"
#include "pk_protected_filter.h"
#include "clstab.h"
#include "actobject.h"
#include "playertemplate.h"
#include "petnpc.h"
#include "player_imp.h"
#include <glog.h>

DEFINE_SUBSTANCE(pk_protected_filter,filter,CLS_FILTER_PK_PROTECTED)

void 
pk_protected_filter::OnAttach()
{
	gobject * pObj = _parent.GetImpl()->_parent;
	gplayer_imp *pImp = (gplayer_imp *)(_parent.GetImpl());
	
	pImp->DisablePVPFlag(gplayer_imp::PLAYER_PVP_PROTECTED);

	GLog::log(GLOG_INFO,"用户%d进入新手安全区(%f,%f)",pObj->ID.id,pObj->pos.x,pObj->pos.z);
}

void 
pk_protected_filter::OnRelease()
{
	gobject * pObj = _parent.GetImpl()->_parent;
	gplayer_imp *pImp = (gplayer_imp *)(_parent.GetImpl());

	pImp->EnablePVPFlag(gplayer_imp::PLAYER_PVP_PROTECTED);
	
	GLog::log(GLOG_INFO,"用户%d离开新手安全区(%f,%f)",pObj->ID.id,pObj->pos.x,pObj->pos.z);
}

void 
pk_protected_filter::Heartbeat(int tick)
{
	if((_counter += 1) < 7) return;
	//每隔7秒检查是否该脱离了安全区
	_counter = 1;
	
	gobject * pObj = _parent.GetImpl()->_parent;
	if (!player_template::IsInPKProtectDomain(pObj->pos))
	{
		_is_deleted = true;
		return;
	}
}

