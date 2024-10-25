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

	GLog::log(GLOG_INFO,"�û�%d�������ְ�ȫ��(%f,%f)",pObj->ID.id,pObj->pos.x,pObj->pos.z);
}

void 
pk_protected_filter::OnRelease()
{
	gobject * pObj = _parent.GetImpl()->_parent;
	gplayer_imp *pImp = (gplayer_imp *)(_parent.GetImpl());

	pImp->EnablePVPFlag(gplayer_imp::PLAYER_PVP_PROTECTED);
	
	GLog::log(GLOG_INFO,"�û�%d�뿪���ְ�ȫ��(%f,%f)",pObj->ID.id,pObj->pos.x,pObj->pos.z);
}

void 
pk_protected_filter::Heartbeat(int tick)
{
	if((_counter += 1) < 7) return;
	//ÿ��7�����Ƿ�������˰�ȫ��
	_counter = 1;
	
	gobject * pObj = _parent.GetImpl()->_parent;
	if (!player_template::IsInPKProtectDomain(pObj->pos))
	{
		_is_deleted = true;
		return;
	}
}

